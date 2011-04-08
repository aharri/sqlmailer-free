/*
 *  mailertcp.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#include <iostream>
#include "mailertcp.h"
#include "mailerparams.h"
#include "mailererrors.h"

namespace sqlmailer
{	
	CMailerTCP::CMailerTCP()
	{
		iBuffer = new char[MAX_DATA_READ_FROM_SOCKET];
		iConnected = false;
	}

	CMailerTCP::~CMailerTCP()
	{
		iConnected = false;
		delete[] iBuffer;
	}

	int FASTCALL CMailerTCP::Init(CMailerParams* pParams)
	{
		iConnected = false;
		iParams = pParams;
		iHostEnt = gethostbyname(pParams->GetSMTPHost().c_str());
		if(iHostEnt == NULL)
		{
			MailerErrors->Push("Unable to get host by name");
			return -1;
		}
		
		iSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(iSocket <= 0)
		{
			MailerErrors->Push("Unable to create socket");
			return -1;
		}
		iServerAddress.sin_family = AF_INET;
		iServerAddress.sin_addr = *reinterpret_cast<struct in_addr*>(iHostEnt->h_addr);
		iServerAddress.sin_port = htons(pParams->GetSMTPPort());
		memset(&(iServerAddress.sin_zero), '\0', 8);
		return 0;
	}

	int FASTCALL CMailerTCP::Destroy(void)
	{
		iConnected = false;
		return 0;
	}

	int FASTCALL CMailerTCP::Connect(void)
	{
		if(connect(iSocket, (struct sockaddr*)&iServerAddress, sizeof(iServerAddress)) < 0)
		{
			switch(errno)
			{
				case ENOTSOCK:
					MailerErrors->Push(std::string("Unable to connect smtp host>NOTSOCK"));
				break;
				case EADDRNOTAVAIL:
					MailerErrors->Push(std::string("Unable to connect smtp host>EADDRNOTAVAIL"));
				break;
				case EAFNOSUPPORT:
					MailerErrors->Push(std::string("Unable to connect smtp host>EAFNOSUPPORT"));
				break;
				case EISCONN:
					MailerErrors->Push(std::string("Unable to connect smtp host>EISCONN"));
				break;
				case ETIMEDOUT:
					MailerErrors->Push(std::string("Unable to connect smtp host>ETIMEDOUT"));
				break;
				case ECONNREFUSED:
					MailerErrors->Push(std::string("Unable to connect smtp host>ECONNREFUSED"));
				break;
				case ENETUNREACH:
					MailerErrors->Push(std::string("Unable to connect smtp host>ENETUNREACH"));
				break;
				case EADDRINUSE:
					MailerErrors->Push(std::string("Unable to connect smtp host>EADDRINUSE"));
				break;
				case EFAULT:
					MailerErrors->Push(std::string("Unable to connect smtp host>EFAULT"));
				break;
				case EINPROGRESS:
					MailerErrors->Push(std::string("Unable to connect smtp host>EINPROGRESS"));
				break;
				case EALREADY:
					MailerErrors->Push(std::string("Unable to connect smtp host>EALREADY"));
				break;
				case ENOTDIR:
					MailerErrors->Push(std::string("Unable to connect smtp host>ENOTDIR"));
				break;
				case ENAMETOOLONG:
					MailerErrors->Push(std::string("Unable to connect smtp host>ENAMETOOLONG"));
				break;
				case ENOENT:
					MailerErrors->Push(std::string("Unable to connect smtp host>ENOENT"));
				break;
				case EACCES:
					MailerErrors->Push(std::string("Unable to connect smtp host>EACCES"));
				break;
				case ELOOP:
					MailerErrors->Push(std::string("Unable to connect smtp host>ELOOP"));
				break;
				default:
					char temp[50];
					sprintf(temp, "%i", errno);
					MailerErrors->Push(std::string("Unable to connect smtp host>") + std::string(temp));
				break;
				
			};
			return -1;
		}
		iConnected = true;
					
		return 0;
	}
	
	int FASTCALL CMailerTCP::Disconnect(void)
	{
		iConnected = false;
		// FIXME: Does this return something?
		close(iSocket);
		return 0;
	}
	
	int FASTCALL CMailerTCP::Read(std::string& pStr)
	{
		if(!iConnected)
			return -1;
		int nread = 0;
		pStr.clear();
		while((nread = read(iSocket, iBuffer, MAX_DATA_READ_FROM_SOCKET-1)) > 0)
		{
			iBuffer[nread] = '\0';
			pStr += iBuffer;
			// std::cout << pStr.c_str();
			if(nread < MAX_DATA_READ_FROM_SOCKET-1) 
				break;
		}
		return 0;
	}
	
	int FASTCALL CMailerTCP::Write(std::string aStr)
	{
		if(!iConnected)
			return -1;
		// std::cout << aStr.c_str() << "\n";
		write(iSocket, aStr.c_str(), aStr.length());
		return 0;
	}

	bool FASTCALL CMailerTCP::Connected(void)
	{
		return iConnected;
	}

}
