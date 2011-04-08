/*
 *  mymailqueue.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#ifndef MYMAILQUEUE_H
#define MYMAILQUEUE_H

#include <mysql++/mysql++.h>
#include <string>
#include <map>
#include "defines.h"

namespace sqlmailer
{
	class CMailerParams;
	
	typedef std::pair<std::string, std::string> QueueStringPair;
	typedef std::map<std::string, std::string> QueueStringMap;
	
	enum EGetMailReturn { eQueueOk, eQueueEmpty, eQueueError };
	
	class CMyMailQueue
	{
		private:
			CMailerParams* iParams;
			mysqlpp::Connection* iDBConnection;
		protected:
			std::string FASTCALL FieldNullCheck(mysqlpp::ColData aData);
			int FASTCALL SetMailToProcessState(std::string aId, std::string aDB, std::string aPrefix);
		public:
			CMyMailQueue(void);
			~CMyMailQueue(void);
			int FASTCALL Init(CMailerParams* pParams);
			int FASTCALL Destroy(void);
			EGetMailReturn FASTCALL GetMail(QueueStringMap& rMap);
			int FASTCALL MailDone(QueueStringMap& rMap);
	};
};

#endif
