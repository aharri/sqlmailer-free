/*
 *  mailerparams.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#ifndef MAILERPARAMS_H
#define MAILERPARAMS_H
 
#include <string>
#include <map>
#include <vector>
#include "defines.h"
#include "tinyxml/tinyxml.h"
#include "stringtools.h"

namespace sqlmailer
{
	typedef std::pair <std::string, std::string> DBStringPair;
	typedef std::map <std::string, std::string> DBStringMap;
	typedef std::vector<DBStringMap> DBStringVec;

	struct CDBServer
	{
		std::string host;
		int port;
		std::string socket;
		bool compress;
		int timeout;
		DBStringMap options;
		DBStringVec queues;
		CDBServer() {}
	};

	typedef std::vector<CDBServer> DBServersVec;
	class CMailerParams
	{
		private:
			TiXmlDocument* iConfig;
			TiXmlElement* iRoot;
			int iSMTPPort;
			bool iDebugOut;
			std::string iSMTPHost;
			int iRCPTPerRun;
			DBServersVec iServers;
			int iLineWrap;
			int iMailOverMinOld;
			std::string iBodyEncoding;
			std::string iMailCharset;
			std::string iDBCharset;
			int iCurrentServer;
		protected:
		public:
			CMailerParams(void);
			~CMailerParams(void);
			int FASTCALL Init(std::string aConfigFile);
			int FASTCALL Destroy(void);
			std::string FASTCALL GetValue(std::string aCategory, std::string aKey);
			int FASTCALL GetRcpts(void);

			std::string FASTCALL GetDBHost(void);
			std::string FASTCALL GetDBUser(void);
			std::string FASTCALL GetDBPass(void);
			std::string FASTCALL GetDBSocket(void);
			int FASTCALL GetDBPort(void);
			int FASTCALL GetDBTimeout(void);
			bool FASTCALL GetDBCompress(void);
			int FASTCALL GetQueueDB(int aId, std::string& rDB, std::string& rPrefix, std::string& rCollate);
			int FASTCALL GetQueueCount(void);
			int FASTCALL GetServerCount(void);
			void FASTCALL SetCurrentServer(const int aCurrentServer);

			std::string FASTCALL GetSMTPHost(void);
			int FASTCALL GetSMTPPort(void);
			bool FASTCALL GetDisableMailSending(void);
			bool FASTCALL GetDisableMailDeleting(void);
			bool FASTCALL GetDebugOut(void);
			int FASTCALL GetLineWrap(void);
			int FASTCALL GetMailOverMinOld(void);
			std::string FASTCALL GetBodyEncoding(void);
			std::string FASTCALL GetMailCharset(void);
			std::string FASTCALL GetDBCharset(void);
	};
}

#endif
