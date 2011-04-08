/*
 *  mailersmtp.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#ifndef MAILERSMTP_H
#define MAILERSMTP_H

#include <map>
#include <string>
#include "stringtools.h"
#include "defines.h"

namespace sqlmailer
{
	class CMailerParams;
	class CMailerTCP;
	
	typedef std::pair<std::string, std::string> SMTPParamsPair;
	typedef std::map<std::string, std::string> SMTPParamsMap;
	class CMailerSMTP
	{
		private:
			CMailerTCP* iMailerTCP;
			CMailerParams* iParams;
			STStringVector iMailLines;
			std::string iAddressDelim;
			bool iConnected;
		protected:
			int FASTCALL SMTPReset(void);
			int FASTCALL CheckLine(std::string aLine, std::string aBeg);
			void FASTCALL CreateData(SMTPParamsMap& rMap);
			int FASTCALL HandleLine(std::string aStr, std::string& rBuf, std::string aExp);
			int FASTCALL Connect();
			int FASTCALL SMTPStart(void);
			int FASTCALL SMTPQuit(void);
			int FASTCALL SMTPEhlo(void);
		public:
			CMailerSMTP();
			~CMailerSMTP();
			int FASTCALL Init(CMailerParams* pParams, std::string aAddressDelim);
			int FASTCALL Destroy(void);
			int FASTCALL Send(SMTPParamsMap& rMap, STStringVector& rErroredMails);
	};
};
#endif
