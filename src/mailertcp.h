/*
 *  mailertcp.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#ifndef MAILERTCP_H
#define MAILERTCP_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <stdlib.h>
#include <errno.h>
#include "defines.h"

namespace sqlmailer
{
	class CMailerParams;
	
	class CMailerTCP
	{
		private:
			bool iConnected;
			int iSocket;
			char* iBuffer;
			struct sockaddr_in iServerAddress;
			struct hostent* iHostEnt;
			CMailerParams* iParams;
		protected:
		public:
			CMailerTCP();
			~CMailerTCP();
			int FASTCALL Init(CMailerParams* pParams);
			int FASTCALL Destroy(void);
			int FASTCALL Connect(void);
			int FASTCALL Disconnect(void);
			int FASTCALL Read(std::string& rStr);
			int FASTCALL Write(std::string aStr);
			bool FASTCALL Connected(void);
	};
}

#endif
