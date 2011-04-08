/*
 *  sqlmailer.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#include <iostream>
#include "defines.h"
#include "sqlmailer.h"
#include "mailerparams.h"
#include "mailererrors.h"
#include "mailersmtp.h"
#include "stringtools.h"
#include "mailersmtp.h"

namespace sqlmailer
{

	CSQLMailer::CSQLMailer(void)
	{
		iParams = NULL;
		iAddressDelimiter = ADDRESS_DELIMITER;
	}

	CSQLMailer::~CSQLMailer(void)
	{
	}

	int FASTCALL CSQLMailer::Init(CMailerParams* pParams, CMailerSMTP* pSMTP)
	{
		iRecipientsHandled = 0;
		iParams = pParams;
		iSMTP = pSMTP;
		
		return 0;
	}

	int FASTCALL CSQLMailer::Destroy(void)
	{
		return 0;
	}

	int FASTCALL CSQLMailer::Execute(void)
	{
		int ret = 0;
		int recipient_count = 0;
		for(int i = 0; i < iParams->GetServerCount(); i++)
		{
			MailerErrors->DebugVerbose(std::string("Server ") + StringFromInt(i) + std::string(" executed"));

			CMyMailQueue* qu = new CMyMailQueue();
			if(qu->Init(iParams, i) != 0)
			{
				MailerErrors->Push(std::string("Unable to init server(")+qu->GetName()+std::string(")"));
				delete qu;
				continue;
			}
			else
				MailerErrors->DebugVerbose("Init succeeded");

			bool done = false;
			while(!done)
			{
				QueueStringMap mail;
				MailerErrors->DebugVerbose("Querying mails");
				switch(qu->GetMail(mail))
				{
					case eQueueOk:
						MailerErrors->DebugVerbose("Will be handling mail");
						if(HandleMail(qu, mail, recipient_count) != 0)
						{
							MailerErrors->Push("Failed to handle mail.");
							done = true;
						}
						MailerErrors->DebugVerbose("Mail is handled");
					break;
					case eQueueEmpty:
						MailerErrors->DebugVerbose(std::string("Empty queue"));
						done = true;
					break;
					case eQueueError:
						MailerErrors->Push(std::string("Getmail failed"));
						done = true;
					break;
				}
				if(recipient_count > iParams->GetRcpts() && iParams->GetRcpts() != 0)
				{
					done = true;
				}
			}
			
			MailerErrors->DebugVerbose("Calling destroy");
			if(qu->Destroy() != 0)
			{
				MailerErrors->Push("Unable to destroy queue");
				delete qu;
			}
			else
				MailerErrors->DebugVerbose("Destroy succeeded");

			delete qu;

			if(recipient_count > iParams->GetRcpts() && iParams->GetRcpts() != 0)
			{
				MailerErrors->DebugVerbose(std::string("Recipients exceeded"));
				break;
			}
		}

		return ret;
	}
	
	int FASTCALL CSQLMailer::HandleMail(CMyMailQueue* pQueue, QueueStringMap& rMap, int& rCount)
	{
		STStringVector erroredMails;

		if(iSMTP->Send(rMap, erroredMails) != 0)
		{
			MailerErrors->Push("Failed to send mail");
			if(erroredMails.size() <= 0)
				return -1;
		}
		
		if(erroredMails.size() > 0)
		{
			MailerErrors->DebugVerbose("Sending error mail to admin");
			SMTPParamsMap error_mail;
			error_mail["from"] = rMap["admin"];
			error_mail["to"] = rMap["admin"];
			error_mail["cc"] = "";
			error_mail["bcc"] = "";
			error_mail["subject"] = "Automatic notification from SQLMailer";
			error_mail["body"] = std::string("Automatic notification for mailing list ");
			error_mail["body"] += rMap["list_name"];
			error_mail["body"] += std::string(",\nthese addresses were automatically removed:\n\n");
			for(STStringVector::iterator i = erroredMails.begin(); i != erroredMails.end(); i++)
			{
				error_mail["body"] += (*i) + std::string("\n");
			}
			error_mail["body"] += std::string("\n");
			error_mail["footer"] = "Automatic notification from SQLMailer";
			error_mail["type"] = "normal";
			error_mail["list_name"] = "";
			error_mail["list_address"] = "";

			if(iSMTP->Send(error_mail, erroredMails) != 0)
			{
				MailerErrors->Push("Failed to send error mail for admin");
			}
		}
		
		if(pQueue->MailDone(rMap, erroredMails) != 0)
		{
			MailerErrors->Push("Failed to set mail as done");
			return -1;
		}
		return 0;
	}

}

