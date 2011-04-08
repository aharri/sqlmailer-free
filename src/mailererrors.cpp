/*
 *  mailererrors.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#include "mailererrors.h"

#include <iostream>

namespace sqlmailer
{
	CMailerErrors* MailerErrors = NULL;

	CMailerErrors::CMailerErrors(void)
	{
		while(!iErrorQueue.empty()) iErrorQueue.pop();
	}

	CMailerErrors::~CMailerErrors(void)
	{
		while(!iErrorQueue.empty()) iErrorQueue.pop();
	}
	
	void FASTCALL CMailerErrors::DebugVerbose(std::string aVerboseString)
	{
		//std::cout << "- " << aVerboseString << std::endl;
		iDebugVerbose.push(aVerboseString);
	}

	std::string FASTCALL CMailerErrors::PopDebug(void)
	{
		std::string ret = iDebugVerbose.front();
		iDebugVerbose.pop();
		return ret;
	}

	bool FASTCALL CMailerErrors::EmptyDebug(void)
	{
		return iDebugVerbose.empty();
	}

	void FASTCALL CMailerErrors::Warning(std::string aVerboseString)
	{
		//std::cout << "- " << aVerboseString.c_str() << std::endl;
		iWarning.push(aVerboseString);
	}

	std::string FASTCALL CMailerErrors::PopWarning(void)
	{
		std::string ret = iWarning.front();
		iWarning.pop();
		return ret;
	}

	bool FASTCALL CMailerErrors::EmptyWarning(void)
	{
		return iWarning.empty();
	}

	void FASTCALL CMailerErrors::Push(std::string aErrorStr)
	{
		//std::cout << "- " << aErrorStr.c_str() << std::endl;
		iErrorQueue.push(aErrorStr);
	}

	std::string FASTCALL CMailerErrors::Pop(void)
	{
		std::string ret = iErrorQueue.front();
		iErrorQueue.pop();
		return ret;
	}

	bool FASTCALL CMailerErrors::Empty(void)
	{
		return iErrorQueue.empty();
	}
} 
