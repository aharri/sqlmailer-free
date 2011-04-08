/*
 *  mymailqueue.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */
 
/* Notes:
 *   Connection will be made primarily 
 *   with tcp-connection, secondary with socket-connection and
 *   if neither of those are declared, it will let to choose by the
 *   database drivers. 
 *
 *   Setting timeout is truncated, because of mysqlpp doesn't have
 *   it anymore.
 */
#include <iostream>
#include "mymailqueue.h"
#include "mailerparams.h"
#include "mailererrors.h"

namespace sqlmailer
{
	CMyMailQueue::CMyMailQueue(void)
	{
		iParams = NULL;
		iName.clear();
		iDBConnection = new mysqlpp::Connection(true);
	}

	CMyMailQueue::~CMyMailQueue(void)
	{
		delete iDBConnection;
	}

	int FASTCALL CMyMailQueue::Init(CMailerParams* pParams, const int aServerNumber)
	{
		iParams = pParams;
		iParams->SetCurrentServer(aServerNumber);
		std::string server = "";
		std::string user = iParams->GetDBUser().c_str();
		std::string pass = iParams->GetDBPass().c_str();
		if(!iParams->GetDBHost().empty())
		{
			server = std::string(iParams->GetDBHost()) + std::string(":") + StringFromInt(iParams->GetDBPort());
		}
		else if(!iParams->GetDBSocket().empty())
		{
			server = iParams->GetDBSocket();
		}
		else
		{
			server = "0";
		}

		MailerErrors->DebugVerbose(std::string("Server = ") + server);
		MailerErrors->DebugVerbose(std::string("User = ") + user);
		iName = server;
		if(iParams->GetDBCompress())
		{
			try
			{
				iDBConnection->set_option(new mysqlpp::CompressOption()); // mysqlpp manual claimed that this will be delete by library.. I wonder if that's true :P
			}
			catch(mysqlpp::BadOption e)
			{
				MailerErrors->Warning(std::string("Unable to set compress option: ") + e.what());
			}
		}
		try
		{
			iDBConnection->set_option(new mysqlpp::ConnectTimeoutOption(iParams->GetDBTimeout())); // mysqlpp manual claimed that this will be delete by library.. I wonder if that's true :P
		}
		catch(mysqlpp::BadOption e)
		{
			MailerErrors->Warning(std::string("Unable to set timeout option: ") + e.what());
		}

		try
		{
			if(!iDBConnection->connect("mysql", server.c_str(), user.c_str(), pass.c_str()))
			{
				MailerErrors->Push(std::string("Unable to connect db: ") + iDBConnection->error());
				return -1;
			}
			MailerErrors->DebugVerbose("Connection succeeded");
		}
		catch(mysqlpp::Exception e)
		{
				MailerErrors->Push(std::string("Exception was thrown when connecting db: ") + e.what());
				return -1;
		}
		std::string sql = "";
		sql = std::string(std::string("set character set ") + iParams->GetDBCharset());
		mysqlpp::Query qry = iDBConnection->query();
		qry << sql.c_str();
		if(qry.execute() == false)
		{
			return -1;
		}
		MailerErrors->DebugVerbose("All done in init");
		return 0;
	}

	int FASTCALL CMyMailQueue::Destroy(void)
	{
		iDBConnection->disconnect();
		return 0;
	}

	int FASTCALL CMyMailQueue::SetMailToProcessState(std::string aId, std::string aDB, std::string aPrefix)
	{
		std::string sql = "";
		sql = std::string("update `") + aDB + std::string("`.`") + aPrefix +
					std::string("mail_queue`") +
					std::string("set `status` = 'processing'") +
					std::string("where `id` = '") + aId + std::string("'");
		try
		{
			mysqlpp::Query qry = iDBConnection->query(sql.c_str());
			if(qry.execute() == false)
			{
				return -1;
			}
		}
		catch(mysqlpp::Exception e)
		{
			HandleCatch(e, __LINE__, __FILE__);
		}
	
		return 0;
	}
	
	EGetMailReturn FASTCALL CMyMailQueue::GetMail(QueueStringMap& rMap)
	{
		std::string db = "", prefix = "", sql = "";
		std::string collate_string = "";
		sql = "select `db`, `prefix`, `id`, `type`, `from`, `subject`, \
						`body`, `footer`, `list_id`, `list_name`, `list_address`, `created` \
						from (";
		for(int i = 0; i < iParams->GetQueueCount(); i++)
		{
			collate_string = "";
			iParams->GetQueueDB(i, db, prefix, collate_string);
			if(!collate_string.empty())
				collate_string = std::string("collate ") + collate_string;
			if(i != 0) sql += " union ";
			sql += std::string("(select ");
			sql += std::string("'") + db + std::string("' as `db`, ");
			sql += std::string("'") + prefix + std::string("' as `prefix`, ");
			sql += std::string("`id`, `type`, `from`, \
														`subject`, `body` ") + collate_string +
														std::string(" as body, `footer` ") + collate_string +
														std::string(" as footer, `list_id`, `list_name`, \
														`list_address`, `created` from `") + 
														db + std::string("`.`") + prefix + 
														std::string("mail_queue` \
															where `status` = 'waiting' \
															and TIMESTAMPADD(minute,") +
															StringFromInt(iParams->GetMailOverMinOld()) + 
															std::string(", `created`) < now() \
															limit 1)");
		}
		sql += ") as t order by `created` asc limit 1";

		std::vector<mysqlpp::Row> v;
		try
		{
			mysqlpp::Query qry = iDBConnection->query();
			qry << sql.c_str();
			qry.storein(v);
		}
		catch(mysqlpp::Exception e)
		{
			HandleCatch(e, __LINE__, __FILE__);
			return eQueueError;
		}

		for(std::vector<mysqlpp::Row>::iterator ite = v.begin(); ite != v.end(); ite++) // NOTE: There will be only one row(sql is limited to 1)
		{
			mysqlpp::Row &row = (*ite);
			
			rMap["id"] = FieldNullCheck(row["id"]);
			rMap["db"] = FieldNullCheck(row["db"]);
			rMap["prefix"] = FieldNullCheck(row["prefix"]);
			rMap["type"] = FieldNullCheck(row["type"]);
			rMap["from"] = FieldNullCheck(row["from"]);
			rMap["subject"] = FieldNullCheck(row["subject"]);
			rMap["body"] = FieldNullCheck(row["body"]);
			rMap["footer"] = FieldNullCheck(row["footer"]);
			rMap["list_id"] = FieldNullCheck(row["list_id"]);
			rMap["list_name"] = FieldNullCheck(row["list_name"]);
			rMap["list_address"] = FieldNullCheck(row["list_address"]);
			rMap["to"] = "";
			rMap["cc"] = "";
			rMap["bcc"] = "";

			sql = std::string("select `header_type`, `email`");
			sql += std::string(" from `") + db + std::string("`.`") + prefix + std::string("mail_queue_data`");
			sql += std::string(" where `queue_id` = ") + rMap["id"];
			std::vector<mysqlpp::Row> addresses;
			try
			{
				mysqlpp::Query mails_qry = iDBConnection->query();
				mails_qry << sql.c_str();
				mails_qry.storein(addresses);

				for(std::vector<mysqlpp::Row>::iterator mite = addresses.begin(); mite != addresses.end(); mite++)
				{
					mysqlpp::Row &mrow = (*mite);
					std::string atype = FieldNullCheck(mrow["header_type"]);
					std::string amail = FieldNullCheck(mrow["email"]);
					if(rMap[atype].length() > 0) rMap[atype] += ";";
					rMap[atype] += amail;
				}
			}
			catch(mysqlpp::Exception e)
			{
				HandleCatch(e, __LINE__, __FILE__);
				return eQueueError;
			}

			if(SetMailToProcessState(rMap["id"], rMap["db"], rMap["prefix"]) != 0)
				return eQueueError;

			if(rMap["type"] == "mlist")
			{
				std::string sql2 = std::string("select `admin` from `") + 
					rMap["db"] + std::string("`.`") + rMap["prefix"] +
					std::string("mailinglists` where `id` = ") +
					rMap["list_id"] + std::string(" limit 1");

					try
					{
						mysqlpp::Query qry2 = iDBConnection->query();
						qry2 << sql2.c_str();
						std::vector<mysqlpp::Row> v2;
						qry2.storein(v2);

						if(v2.begin() == v2.end())
						{
							MailerErrors->DebugVerbose(std::string("SQL query that returned 0 rows: ") + sql2);
							MailerErrors->Push(std::string("Unable to obtain admin from the list ") + rMap["list_name"]);
							rMap["admin"] = rMap["from"];
						}
						else
						{
							std::vector<mysqlpp::Row>::iterator ite2 = v2.begin();
							mysqlpp::Row &row2 = (*ite2);
							rMap["admin"] = FieldNullCheck(row2["admin"]);
						}
					}
					catch(mysqlpp::Exception e)
					{
						HandleCatch(e, __LINE__, __FILE__);
						rMap["admin"] = rMap["from"];
					}
					
			}
			else
			{
				rMap["admin"] = rMap["from"];
			}

			return eQueueOk;
		}
		return eQueueEmpty;
	}
	
	std::string FASTCALL CMyMailQueue::FieldNullCheck(const mysqlpp::String aData)
	{
		std::string str = aData.c_str();
		if(str == "NULL")
			str.clear();
		return str;
	} 

	int FASTCALL CMyMailQueue::MailDone(QueueStringMap& rMap, STStringVector& rErroredMails)
	{
		if(iParams->GetDisableMailDeleting()) return 0;

		std::string sql = "";
		
		if(rErroredMails.size() > 0 && rMap["type"] == "mlist")
		{
			std::string delete_list = "";
			StringJoin(rErroredMails, delete_list, "','");
			MailerErrors->DebugVerbose(std::string("Deleting mail address': ") + delete_list);
			delete_list = std::string("'") + delete_list + std::string("'");

			sql = std::string("delete from `") + rMap["db"] + std::string("`.`") + rMap["prefix"] +
						std::string("mailinglists_subscribers`") +
						std::string("where `list_id` = ") + rMap["list_id"] +
						std::string(" and address in (") + delete_list + std::string(")");
			try
			{
				mysqlpp::Query qry = iDBConnection->query(sql.c_str());
				if(qry.execute() == false)
				{
					MailerErrors->Push(std::string("Unable to delete mail addresses('") + delete_list + std::string("') from list ") + rMap["list_id"]);
				}
			}
			catch(mysqlpp::Exception e)
			{
				MailerErrors->Push(std::string("Unable to delete mail addresses('") + delete_list + std::string("') from list ") + rMap["list_id"]);
				HandleCatch(e, __LINE__, __FILE__);
			}
		}
		
		sql = std::string("delete from `") + rMap["db"] + std::string("`.`") + rMap["prefix"] +
					std::string("mail_queue`") +
					std::string("where `id` = '") + rMap["id"] + std::string("'");
		try
		{
			mysqlpp::Query qry2 = iDBConnection->query(sql.c_str());
			if(qry2.execute() == false)
			{
				MailerErrors->Push(std::string("Unable to delete mail(") + rMap["id"] + std::string(") from queue"));
			}
		}
		catch(mysqlpp::Exception e)
		{
			MailerErrors->Push(std::string("Unable to delete mail(") + rMap["id"] + std::string(") from queue"));
			HandleCatch(e, __LINE__, __FILE__);
		}
		return 0;
	}
		
	std::string FASTCALL CMyMailQueue::GetName(void)
	{
		return iName;
	}
	
	void FASTCALL CMyMailQueue::HandleCatch(mysqlpp::Exception& rException, int aLine, std::string aFile)
	{
		char line[10] = "";
		snprintf(line, 10, "%i", aLine);
		MailerErrors->Push(std::string("Exception(") + std::string(aFile) +
			std::string(" ") + std::string(line) + std::string("):") +
			std::string(rException.what()));
	}

}
