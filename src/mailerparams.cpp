/*
 *  mailerparams.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#include "mailerparams.h"
#include "mailererrors.h"

namespace sqlmailer
{

	CMailerParams::CMailerParams(void)
	{
		iRoot = NULL;
		iConfig = NULL;
		iDebugOut = false;
		iLineWrap = 0;
		iCurrentServer = 0;
	}

	CMailerParams::~CMailerParams(void)
	{
	}

	int FASTCALL CMailerParams::Init(std::string aConfigFile)
	{
		const char* temp = NULL;
		iConfig = new TiXmlDocument(aConfigFile.c_str());
		if(!iConfig->LoadFile())
		{
			MailerErrors->Push(std::string("Unable to load ") + aConfigFile + std::string("<") + iConfig->ErrorDesc() + std::string(">"));
			return -1;
		}
		iRoot = iConfig->FirstChildElement("sqlmailer");
		if(iRoot == NULL)
		{
			MailerErrors->Push(std::string("Unable to find root element<sqlmailer>"));
			return -1;
		}
		double version = 0.0;
		if(iRoot->QueryDoubleAttribute("version", &version) != TIXML_SUCCESS || version != CONFIG_VERSION)
		{
			MailerErrors->Push(std::string("Wrong version of config file, expecting: ") + CONFIG_VERSION_STR);
			return -1;
		}
		TiXmlElement* element = iRoot->FirstChildElement("smtp");
		TiXmlElement* element2 = NULL;
		if(element == NULL)
		{
			MailerErrors->Push(std::string("No smtp element declared"));
			return -1;
		}

		// SMTP Port
		if(element->QueryIntAttribute("port", &iSMTPPort) != TIXML_SUCCESS)
		{
			MailerErrors->Push("No smtp port declared");
			return -1;
		}

		// SMTP host
		const char* smtp_host = element->Attribute("host");
		if(smtp_host == NULL)
		{
			MailerErrors->Push("No smtp host declared");
			return -1;
		}
		iSMTPHost = std::string(smtp_host);

		bool flag = false;
		element = iRoot->FirstChildElement("mailer");
		if(element == NULL)
		{
			MailerErrors->Push("No mailer element defined");
			return -1;
		}

		element2 = element->FirstChildElement("rcpts");
		if(element2 != NULL)
		{
			if(element2->QueryIntAttribute("per_run", &iRCPTPerRun) == TIXML_SUCCESS)
			{
				flag = true;
			}
		}
		if(!flag)
		{
			MailerErrors->Warning("rcpts per run in config file is missing");
		}
		
		element2 = element->FirstChildElement("format");
		if(element2 != NULL)
		{
			if(element2->QueryIntAttribute("line_wrap", &iLineWrap) == TIXML_SUCCESS)
			{
				flag = true;
			}
		}
		if(!flag)
		{
			MailerErrors->Warning("format/line_wrap in config file is missing");
		}

		element2 = element->FirstChildElement("body_encoding");
		if(element2 != NULL)
		{
			temp = element2->Attribute("type");
			if(temp != NULL)
			{
				iBodyEncoding = temp;
				flag = true;
			}
		}
		if(!flag)
		{
			MailerErrors->Warning("body_encoding/type in config file is missing");
		}

		element2 = element->FirstChildElement("mail_charset");
		if(element2 != NULL)
		{
			temp = element2->Attribute("value");
			if(temp != NULL)
			{
				iMailCharset = temp;
				flag = true;
			}
		}
		if(!flag)
		{
			MailerErrors->Warning("mail_charset/value in config file is missing");
		}

		element2 = element->FirstChildElement("db_charset");
		if(element2 != NULL)
		{
			temp = element2->Attribute("value");
			if(temp != NULL)
			{
				iDBCharset = temp;
				flag = true;
			}
		}
		if(!flag)
		{
			MailerErrors->Warning("db_charset/value in config file is missing");
		}

		element2 = element->FirstChildElement("queue");
		if(element2 != NULL)
		{
			if(element2->QueryIntAttribute("over_min_old", &iMailOverMinOld) == TIXML_SUCCESS)
			{
				flag = true;
			}
		}
		if(!flag)
		{
			MailerErrors->Warning("queue/over_min_old in config file is missing");
		}

		element2 = element->FirstChildElement("debug");
		if(element2 != NULL)
		{
			temp = element2->Attribute("out");
			if(temp != NULL)
			{
				std::string flag = temp;
				if(std::string(temp) == "true") iDebugOut = true;
				flag = true;
			}
		}
		if(!flag)
		{
			MailerErrors->Warning("debug options in config file is missing");
		}
		
		TiXmlNode* node = iRoot->FirstChild("servers");
		if(node != NULL)
		{
			// Let's forget other servers for now.
			for(TiXmlNode* i = node->FirstChild("server"); i!=NULL; i = node->IterateChildren("server", i))
			{
				iServers.push_back(CDBServer());
				element = i->ToElement();
				temp = element->Attribute("host");
				if(temp == NULL)
				{
					MailerErrors->Warning("No host for server");
					iServers.pop_back();
					continue;
				}
				iServers.back().host = temp;

				temp = element->Attribute("compress");
				if(temp == NULL)
				{
					iServers.back().compress = false;
				}
				if(std::string(temp) == "true")
				{
					iServers.back().compress = true;
				}
				else if(std::string(temp) == "false")
				{
					iServers.back().compress = false;
				}
				else
				{
					MailerErrors->Warning("Wrong value in element server and attribute compress. Should be true/false");
					iServers.back().compress = false;
				}

				temp = element->Attribute("socket");
				if(temp == NULL)
				{
				}
				iServers.back().socket = temp;

				if(element->QueryIntAttribute("port", &iServers.back().port) != TIXML_SUCCESS)
				{
					iServers.back().port = 0;
				}

				if(element->QueryIntAttribute("timeout", &iServers.back().timeout) != TIXML_SUCCESS)
				{
					iServers.back().timeout = 60;
				}
				
				TiXmlNode* node2 = i->FirstChild("options");
				if(node2 == NULL)
				{
					MailerErrors->Push("No options for server");
					iServers.pop_back();
					continue;
				}	
				else
				{
					for(TiXmlNode* j = node2->FirstChild("option"); 
								j != NULL; 
								j = node2->IterateChildren("option", j))
					{
						element = j->ToElement();
						temp = element->Attribute("name");
						if(temp == NULL)
						{
							MailerErrors->Warning("Invalid option founded");
							continue;
						}
						std::string name = temp;
						temp = element->Attribute("value");
						if(temp == NULL)
						{
							MailerErrors->Warning("Invalid option founded");
							continue;
						}
						std::string value = temp;
						if(name.empty())
						{
							MailerErrors->Warning("Invalid option founded");
							continue;
						}
						iServers.back().options[name] = value;
					}
				}

				node2 = i->FirstChild("queues");		
				if(node2 == NULL)
				{
					MailerErrors->Push("No queues for server");
					iServers.pop_back();
					continue;
				}	
				else
				{
					for(TiXmlNode* j = node2->FirstChild("queue"); 
								j != NULL; 
								j = node2->IterateChildren("queue", j))
					{
						element = j->ToElement();
						temp = element->Attribute("db");
						if(temp == NULL)
						{
							MailerErrors->Warning("Invalid queue founded");
							continue;
						}
						std::string name = temp;
						temp = element->Attribute("prefix");
						if(temp == NULL)
						{
							MailerErrors->Warning("Invalid queue founded");
							continue;
						}
						std::string value = temp;
						if(name.empty())
						{
							MailerErrors->Warning("Invalid queue founded");
							continue;
						}
						temp = element->Attribute("collate");
						if(temp == NULL)
						{
							MailerErrors->Warning("Invalid queue founded");
							continue;
						}
						std::string collat = temp;
						if(name.empty())
						{
							MailerErrors->Warning("Invalid queue founded");
							continue;
						}
						iServers.back().queues.push_back(DBStringMap());
						iServers.back().queues.back().insert(DBStringPair("db", name));
						iServers.back().queues.back().insert(DBStringPair("prefix", value));
						iServers.back().queues.back().insert(DBStringPair("collate", collat));
					}
				}
			} // if i != NULL

		}

		return 0;
	}

	int FASTCALL CMailerParams::Destroy(void)
	{
		delete iConfig;
		return 0;
	}
	
	std::string FASTCALL CMailerParams::GetValue(std::string aCategory, std::string aKey)
	{
		return "";
	}

	int FASTCALL CMailerParams::GetRcpts(void)
	{
		return iRCPTPerRun;
	}

	std::string FASTCALL CMailerParams::GetDBHost(void)
	{
		return iServers[iCurrentServer].host;
	}

	std::string FASTCALL CMailerParams::GetDBUser(void)
	{
		return iServers[iCurrentServer].options["user"];
	}
	
	std::string FASTCALL CMailerParams::GetDBPass(void)
	{
		return iServers[iCurrentServer].options["pass"];
	}

	std::string FASTCALL CMailerParams::GetDBSocket(void)
	{
		return iServers[iCurrentServer].socket;
	}
	
	int FASTCALL CMailerParams::GetDBPort(void)
	{
		return iServers[iCurrentServer].port;
	}
	
	int FASTCALL CMailerParams::GetDBTimeout(void)
	{
		return iServers[iCurrentServer].timeout;
	}
	
	bool FASTCALL CMailerParams::GetDBCompress(void)
	{
		return iServers[iCurrentServer].compress;
	}
	
	int FASTCALL CMailerParams::GetQueueDB(int aId, std::string& rDB, std::string& rPrefix, std::string& rCollate)
	{
		rDB = iServers[iCurrentServer].queues[aId]["db"];
		rPrefix = iServers[iCurrentServer].queues[aId]["prefix"];
		rCollate = iServers[iCurrentServer].queues[aId]["collate"];
		return 0;
	}
	
	std::string FASTCALL CMailerParams::GetSMTPHost(void)
	{
		return iSMTPHost;
	}
	
	int FASTCALL CMailerParams::GetSMTPPort(void)
	{
		return iSMTPPort;
	}

	int FASTCALL CMailerParams::GetQueueCount(void)
	{
		return iServers[iCurrentServer].queues.size();
	}
	
	bool FASTCALL CMailerParams::GetDisableMailSending(void)
	{
		return false;
	}
	
	bool FASTCALL CMailerParams::GetDisableMailDeleting(void)
	{
		return false;
	}
	
	bool FASTCALL CMailerParams::GetDebugOut(void)
	{
		return iDebugOut;
	}
	
	int FASTCALL CMailerParams::GetLineWrap(void)
	{
		return iLineWrap;
	}
	
	int FASTCALL CMailerParams::GetMailOverMinOld(void)
	{
		return iMailOverMinOld;
	}
	
	std::string FASTCALL CMailerParams::GetBodyEncoding(void)
	{
		return iBodyEncoding;
	}
	std::string FASTCALL CMailerParams::GetMailCharset(void)
	{
		return iMailCharset;
	}
	std::string FASTCALL CMailerParams::GetDBCharset(void)
	{
		return iDBCharset;
	}

	int FASTCALL CMailerParams::GetServerCount(void)
	{
		return iServers.size();
	}

	void FASTCALL CMailerParams::SetCurrentServer(const int aCurrentServer)
	{
		iCurrentServer = aCurrentServer;
	}
}
