/*
 *  mailererrors.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */
 
// FIXME: Change name of CMailerErrors class to COutput or something.
#ifndef MAILER_ERRORS_H
#define MAILER_ERRORS_H

#include <string> 
#include <queue>
#include "defines.h"

namespace sqlmailer
{
	typedef std::queue<std::string> ErrorQueue;
	
	class CMailerErrors
	{
		private:
			ErrorQueue iErrorQueue;
			ErrorQueue iDebugVerbose;
			ErrorQueue iWarning;
		protected:
		public:
			CMailerErrors(void);
			~CMailerErrors(void);

			void FASTCALL DebugVerbose(std::string aVerboseString);
			std::string FASTCALL PopDebug(void);
			bool FASTCALL EmptyDebug(void);
			
			void FASTCALL Warning(std::string aVerboseString);
			std::string FASTCALL PopWarning(void);
			bool FASTCALL EmptyWarning(void);

			void FASTCALL Push(std::string aErrorStr);
			std::string FASTCALL Pop(void);
			bool FASTCALL Empty();
	};
	extern CMailerErrors* MailerErrors;
}

#endif
