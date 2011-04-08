/*
 *  mymailqueue.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
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
		iDBConnection = new mysqlpp::Connection(false);
	}

	CMyMailQueue::~CMyMailQueue(void)
	{
		delete iDBConnection;
	}

	int FASTCALL CMyMailQueue::Init(CMailerParams* pParams)
	{
		iParams = pParams;

		if(!iDBConnection->connect("mysql", iParams->GetDBHost().c_str(), 
																iParams->GetDBUser().c_str(), iParams->GetDBPass().c_str(), 
																iParams->GetDBPort(), iParams->GetDBCompress()?1:0,
																iParams->GetDBTimeout(), 
																iParams->GetDBSocket().empty()?NULL:iParams->GetDBSocket().c_str()))
		{
			MailerErrors->Push(std::string("Unable to connect db: ") + iDBConnection->error());
			return -1;
		}

		mysqlpp::Query qry = iDBConnection->query();
		std::string sql = "";
		sql = std::string(std::string("set character set ") + iParams->GetDBCharset());
		qry << sql.c_str();
		qry.execute();
		if(!qry.success())
		{
			return -1;
		}

		return 0;
	}

	int FASTCALL CMyMailQueue::Destroy(void)
	{
		iDBConnection->close();
		return 0;
	}

	int FASTCALL CMyMailQueue::SetMailToProcessState(std::string aId, std::string aDB, std::string aPrefix)
	{
		mysqlpp::Query qry = iDBConnection->query();
		std::string sql = "";
		sql = std::string("update `") + aDB + std::string("`.`") + aPrefix +
					std::string("mail_queue`") +
					std::string("set `status` = 'processing'") +
					std::string("where `id` = '") + aId + std::string("'");
		qry << sql.c_str();
		qry.execute();
		if(!qry.success())
		{
			return -1;
		}
		return 0;
	}
	
	EGetMailReturn FASTCALL CMyMailQueue::GetMail(QueueStringMap& rMap)
	{
		mysqlpp::Query qry = iDBConnection->query();
		std::string db = "", prefix = "", sql = "";
		std::string collate_string = "";
		sql = "select `db`, `prefix`, `id`, `type`, `from`, `to`, `cc`, `bcc`, `subject`, \
						`body`, `footer`, `list_name`, `list_address`, `created` \
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
			sql += std::string("`id`, `type`, `from`, `to`, `cc`, `bcc`, \
														`subject`, `body` ") + collate_string +
														std::string(" as body, `footer` ") + collate_string +
														std::string(" as footer, `list_name`, \
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
		qry << sql.c_str();
		mysqlpp::Result res = qry.store();
		if(res)
		{
			mysqlpp::Row row;
			
			if(row = res.at(0))
			{
				rMap["id"] = FieldNullCheck(row["id"]);
				rMap["db"] = FieldNullCheck(row["db"]);
				rMap["prefix"] = FieldNullCheck(row["prefix"]);
				rMap["type"] = FieldNullCheck(row["type"]);
				rMap["from"] = FieldNullCheck(row["from"]);
				rMap["to"] = FieldNullCheck(row["to"]);
				rMap["cc"] = FieldNullCheck(row["cc"]);
				rMap["bcc"] = FieldNullCheck(row["bcc"]);
				rMap["subject"] = FieldNullCheck(row["subject"]);
				rMap["body"] = FieldNullCheck(row["body"]);
				rMap["footer"] = FieldNullCheck(row["footer"]);
				rMap["list_name"] = FieldNullCheck(row["list_name"]);
				rMap["list_address"] = FieldNullCheck(row["list_address"]);
/*				MailerErrors->DebugVerbose(std::string("queue->id: ") + rMap["id"]);
				MailerErrors->DebugVerbose(std::string("queue->db: ") + rMap["db"]);
				MailerErrors->DebugVerbose(std::string("queue->prefix: ") + rMap["prefix"]);
				MailerErrors->DebugVerbose(std::string("queue->type: ") + rMap["type"]);
				MailerErrors->DebugVerbose(std::string("queue->from: ") + rMap["from"]);
				MailerErrors->DebugVerbose(std::string("queue->to: ") + rMap["to"]);
				MailerErrors->DebugVerbose(std::string("queue->cc: ") + rMap["cc"]);
				MailerErrors->DebugVerbose(std::string("queue->bcc: ") + rMap["bcc"]);
				MailerErrors->DebugVerbose(std::string("queue->subject: ") + rMap["subject"]);
				MailerErrors->DebugVerbose(std::string("queue->body: ") + rMap["body"]);
				MailerErrors->DebugVerbose(std::string("queue->footer: ") + rMap["footer"]);
				MailerErrors->DebugVerbose(std::string("queue->list_name: ") + rMap["list_name"]);
				MailerErrors->DebugVerbose(std::string("queue->list_address: ") + rMap["list_address"]);*/
				if(SetMailToProcessState(rMap["id"], rMap["db"], rMap["prefix"]) != 0)
					return eQueueError;
				return eQueueOk;
			}
		}
		else
		{
			MailerErrors->Push(std::string("Failed to get result set: ") + qry.error());
			return eQueueError;
		}
		return eQueueEmpty;
	}
	
	std::string FASTCALL CMyMailQueue::FieldNullCheck(const mysqlpp::ColData aData)
	{
		std::string str = aData.c_str();
		if(str == "NULL")
			str.clear();
		return str;
	} 

	int FASTCALL CMyMailQueue::MailDone(QueueStringMap& rMap)
	{
		if(iParams->GetDisableMailDeleting()) return 0;

		mysqlpp::Query qry = iDBConnection->query();
		std::string sql = "";
		sql = std::string("delete from `") + rMap["db"] + std::string("`.`") + rMap["prefix"] +
					std::string("mail_queue`") +
					std::string("where `id` = '") + rMap["id"] + std::string("'");
		qry << sql.c_str();
		qry.execute();
		if(!qry.success())
		{
			return -1;
		}
		return 0;
	}
}
