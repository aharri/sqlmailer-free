/*
 *  sqlmailer.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#ifndef SQLMAILER_H
#define SQLMAILER_H

#include "defines.h"
#include "mymailqueue.h"

namespace sqlmailer
{	
	class CMailerParams;
	class CMailerSMTP;

	class CSQLMailer
	{
		private:
			int iRecipientsHandled;
			std::string iAddressDelimiter;
			CMailerParams* iParams;
			CMailerSMTP* iSMTP;
		protected:
			int FASTCALL HandleMail(CMyMailQueue* pQueue, QueueStringMap& rMap, int& rCount);
/*			int FASTCALL CSQLMailer::CheckRecipients(STStringVector &rVec);
			int FASTCALL CheckRecipients(int& rCount, std::string &rTo, std::string &rCC, std::string &rBCC);
			int FASTCALL ValidateRecipient(std::string aRecipient);*/
		public:
			CSQLMailer(void);
			~CSQLMailer(void);
			int FASTCALL Init(CMailerParams* pParams, CMailerSMTP* pSMTP);
			int FASTCALL Destroy(void);
			int FASTCALL Execute(void);
	};
};
#endif
