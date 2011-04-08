/*
 *  mailersmtp.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#include "mailersmtp.h"
#include "mailererrors.h"
#include "mailerparams.h"
#include "mailertcp.h"
#include "stringtools.h"
#include "base64.h"
#include <iostream>
#include <regex.h>

namespace sqlmailer
{
	CMailerSMTP::CMailerSMTP()
	{
		iMailerTCP = new CMailerTCP();
		iConnected = false;
	}

	CMailerSMTP::~CMailerSMTP()
	{
		delete iMailerTCP;
	}

	int FASTCALL CMailerSMTP::Init(CMailerParams* pParams, std::string aAddressDelim)
	{
		iAddressDelim = aAddressDelim;
		iParams = pParams;
		iConnected = false;
		
		if(iParams->GetDisableMailSending())
			return 0;
		if(iMailerTCP->Init(pParams) != 0)
		{
			MailerErrors->Push("Error initializing tcp");
			return -1;
		}

		return 0;
	}

	int FASTCALL CMailerSMTP::Connect()
	{
		if(iParams->GetDisableMailSending())
			return 0;

		if(iConnected) return 0;

		if(iMailerTCP->Connect() != 0)
		{
			MailerErrors->Push("Unable to connect tcp");
			return -1;
		}
		
		if(SMTPStart() != 0)
		{
			MailerErrors->Push("SMTP start returned error");
			return -1;
		}
		
		if(SMTPEhlo() != 0)
		{
			MailerErrors->Push("Reply of ehlo was unknown");
			return -1;
		}
		iConnected = true;

		return 0;
	}

	int FASTCALL CMailerSMTP::Destroy(void)
	{
		if(iParams->GetDisableMailSending())
			return 0;

		if(SMTPQuit() != 0)
		{
			MailerErrors->Push("Unknown id from smtp quit");
			// Lets just disconnect, so no need for return -1;
		}
		if(iMailerTCP->Disconnect() != 0)
		{
			MailerErrors->Push("Unable to disconnect tcp");
			return -1;
		}
		if(iMailerTCP->Destroy() != 0)
			return -1;
		return 0;
	}

	int FASTCALL CMailerSMTP::Send(SMTPParamsMap& rMap, STStringVector& rErroredMails)
	{
		int ret = 0;
		std::string buf = "", str = "";

		if(Connect() != 0) 
			return -1;

		// MAIL FROM:
		// mail from: crusa@openbsd.fi
		// 250 2.1.0 Ok
		str = std::string("mail from: <") + rMap["from"] + std::string(">");
		if(HandleLine(str, buf, "250") != 0)
		{
			MailerErrors->Push(std::string("Error mail from: <") + buf + std::string(">"));
			SMTPReset();
			return -1;
		}
		MailerErrors->DebugVerbose(str.c_str());

		// RCPT TO:
		// rcpt to: crusa@openbsd.fi
		// rcpt to: jani.hast@pp.inet.fi
		// 250 2.1.5 Ok
		STStringVector::size_type error_counter = 0;
		STStringVector vec;
		StringSplit(vec, rMap["to"], iAddressDelim);
		StringSplit(vec, rMap["cc"], iAddressDelim);
		StringSplit(vec, rMap["bcc"], iAddressDelim);
		for(STStringVector::iterator i = vec.begin(); i != vec.end(); i++)
		{
			str = std::string("rcpt to: <") + StringTrim((*i)) + std::string(">");
			if(HandleLine(str, buf, "250") != 0)
			{
				MailerErrors->Warning(std::string("Warning in rcpt to(address dropped): ") + buf);
				error_counter ++;
				rErroredMails.push_back(*i);
			}
			MailerErrors->DebugVerbose(str.c_str());
		}

		if(error_counter >= vec.size())
		{
			MailerErrors->Push(std::string("There weren't any valid addresses"));
			SMTPReset();
			return -1;
		}
		// DATA
		// data
		// 354 End data with <CR><LF>.<CR><LF>
		str = std::string("data");
		if(HandleLine(str, buf, "354") != 0)
		{
			MailerErrors->Push(std::string("Error data start: ") +buf);
			SMTPReset();
			return -1;
		}
		MailerErrors->DebugVerbose(str.c_str());

		// To: crusa1@openbsd.fi
		// From: crusa2@openbsd.fi
		// CC: crusa3@openbsd.fi
		// BCC: crusa4@openbsd.fi
		// Subject: testi...
		// buuhaa
		iMailLines.clear();
		CreateData(rMap);
		for(STStringVector::iterator i = iMailLines.begin();
					i != iMailLines.end(); i++)
		{
			str = (*i);
			if(!iParams->GetDisableMailSending())
			{
				if(iMailerTCP->Write(str + std::string("\r\n")) != 0)
				{
					MailerErrors->Push("Couldn't write to smtp server");
					SMTPReset();
					return -1;
				}
			}
			MailerErrors->DebugVerbose(str.c_str());
		}
		iMailLines.clear();

		// DATA END as .
		str = std::string(".");
		if(HandleLine(str, buf, "250") != 0)
		{
			MailerErrors->Push(std::string("Error mail end: ") +buf);
			SMTPReset();
			return -1;
		}
		MailerErrors->DebugVerbose(str.c_str());

		return ret;
	}

	int FASTCALL CMailerSMTP::SMTPReset(void)
	{
		std::string buf = "";
		return HandleLine("rset\r\n", buf, "250");
	}
	
	int FASTCALL CMailerSMTP::HandleLine(std::string aStr, std::string& rBuf, std::string aExp)
	{
		if(iParams->GetDisableMailSending())
			return 0;

		if(iMailerTCP->Write(aStr + std::string("\r\n")) != 0)
		{
			MailerErrors->Push("Error writing to tcp");
			return -1;
		}
		
		if(iMailerTCP->Read(rBuf) != 0)
		{
			MailerErrors->Push("Error reading from tcp");
			return -1;
		}
		
		if(CheckLine(rBuf, aExp) != 0)
			return -1;
		return 0;
	}
	
	int FASTCALL CMailerSMTP::CheckLine(std::string aLine, std::string aBeg)
	{
		if(aBeg.empty()) return 0;
		// FIXME: Maybe regex matching here?
		if(aBeg.length() > aLine.length())
			return 1;
		if(aBeg != aLine.substr(0, aBeg.length()))
			return -1;
		return 0;
	}
	
	void FASTCALL CMailerSMTP::CreateData(SMTPParamsMap& rMap)
	{
		if(rMap["type"] == "mlist")
		{
			iMailLines.push_back(std::string("To: ") + rMap["list_address"]);
			iMailLines.push_back(std::string("From: ") + rMap["from"]);
			// XXX: is "<mailto:" needed ?
			iMailLines.push_back(std::string("List-Post: ") + 
				std::string("<mailto:") + rMap["list_address"] + std::string(">"));
			iMailLines.push_back(std::string("List-Id: ") + rMap["list_name"]);
			iMailLines.push_back(std::string("Reply-To: ") + 
				std::string("<") + rMap["list_address"] + std::string(">"));
			iMailLines.push_back(std::string("Precedence: bulk"));
		}
		else
		{
			iMailLines.push_back(std::string("To: ") + rMap["to"]);
			iMailLines.push_back(std::string("From: ") + rMap["from"]);
			iMailLines.push_back(std::string("Cc: ") + rMap["cc"]);
		}
		iMailLines.push_back(std::string("Subject: =?") + iParams->GetMailCharset() +
			std::string("?B?") + base64_encode(reinterpret_cast<const unsigned char*>(rMap["subject"].c_str()), rMap["subject"].length()) +
			std::string("?="));
		iMailLines.push_back(std::string("User-Agent: SQLMailer"));
		iMailLines.push_back(std::string("MIME-Version: 1.0"));
		iMailLines.push_back(std::string("Content-Type: text/plain; charset=\"") + iParams->GetMailCharset() + "\"");
		iMailLines.push_back(std::string("Content-Transfer-Encoding: ") + iParams->GetBodyEncoding());
		iMailLines.push_back(std::string("Content-Disposition: inline"));
		std::string body_temp = "";
		if(!rMap["body"].empty())
			body_temp += StringLineWrap(rMap["body"], iParams->GetLineWrap(), "\r\n");
		if(!rMap["footer"].empty())
			body_temp += "\r\n" + rMap["footer"];

		if(iParams->GetBodyEncoding() == "base64")
		{
			body_temp = base64_encode(reinterpret_cast<const unsigned char*>(body_temp.c_str()), body_temp.length());
			body_temp = StringLineWrap(body_temp, 76, "\n");
			iMailLines.push_back(body_temp);
		}
		else
		{
			iMailLines.push_back(body_temp);
		}
	}
	
	int FASTCALL CMailerSMTP::SMTPStart(void)
	{
		std::string buf = "";
		iMailerTCP->Read(buf);
		// 220 openbsd.fi ESMTP
		if(CheckLine(buf, "220") != 0)
		{
			MailerErrors->Push(std::string("Error conn: ") +buf);
			return -1;
		}
		return 0;
	}
	
	int FASTCALL CMailerSMTP::SMTPQuit(void)
	{
		if(!iMailerTCP->Connected()) return 0;
		std::string buf = "";
		iMailerTCP->Write("QUIT\r\n");
		iMailerTCP->Read(buf);
		// quit
		// 221 2.0.0 Bye
		if(CheckLine(buf, "221") != 0)
		{
			MailerErrors->Push(std::string("Error quit: ") +buf);
			return -1;
		}
		return 0;
	}
	
	int FASTCALL CMailerSMTP::SMTPEhlo(void)
	{
		std::string buf = "";
		std::string ehlo = std::string("ehlo ") + iParams->GetSMTPHost() + std::string("\r\n");
		iMailerTCP->Write(ehlo);
		iMailerTCP->Read(buf);
		// ehlo openbsd.fi
		// 250-openbsd.fi
		// 250-PIPELINING
		// 250-SIZE 10240000
		// 250-VRFY
		// 250-ETRN
		// 250-ENHANCEDSTATUSCODES
		// 250-8BITMIME
		// 250 DSN
		if(CheckLine(buf, "250") != 0)
		{
			MailerErrors->Push(std::string("Error ehlo: ") +buf);
			return -1;
		}
		return 0;
	}
}
