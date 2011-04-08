#include <iostream>
#include "mailerparams.h"
#include "mymailqueue.h"
#include "sqlmailer.h"
#include "mailersmtp.h"
#include "mailererrors.h"
#include "argparser.h"

using namespace sqlmailer;

static sqlmailer::CMailerParams* mp = NULL;
static sqlmailer::CSQLMailer* mm = NULL;
static sqlmailer::CMailerSMTP* ms = NULL;

int AllocateAll(void);
int InitAll(void);
void DestroyAll(void);
void DeallocateAll(void);
void FlushErrors(void);
void FlushDebugs(void);
void FlushWarnings(void);

int ParamInit(int argc, char* argv[])
{
	/* void FASTCALL Add(const char pShort[],
	 *                   const char pLong[],
	 *                   const char pName[],
	 *                   const int pPCount,
	 *                   const bool pMultiple,
	 *                   const bool pMandatory,
	 *                   const char pDefault[],
	 *                   const char pModifiers[],
	 *                   const char pComment[]);
	 */
	ArgParser->Add("-c", "--config", "config", 1, false, false, "/etc/sqlmailer.conf", "<file>", "Config file");
	ArgParser->Add("-v", "--version", "version", 0, false, false, SQLMAILER_VERSION, "", "Version");
	ArgParser->Add("-h", "--help", "usage", 0, false, false, "", "", "Usage help");
	return ArgParser->Parse(argc, argv);
}

int AllocateAll(void)
{
	ArgParser = new CArgParser;
	MailerErrors = new CMailerErrors();
	mp = new sqlmailer::CMailerParams();
	mm = new sqlmailer::CSQLMailer();
	ms = new sqlmailer::CMailerSMTP();
	return 0;
}

int InitAll(void)
{
	if(mp->Init(ArgParser->Value("config")) != 0)
	{
		MailerErrors->Push("Unable to init params");
		return -1;
	}
	if(ms->Init(mp, ADDRESS_DELIMITER) != 0)
	{
		MailerErrors->Push("Unable to init smtp");
		return -1;
	}
	if(mm->Init(mp, ms) != 0)
	{
		MailerErrors->Push("Unable to init mailer");
		return -1;
	}
	return 0;
}

void DestroyAll(void)
{
	if(ms->Destroy() != 0)
	{
		MailerErrors->Push("Unable to destroy smtp");
	}
	if(mm->Destroy() != 0)
	{
		MailerErrors->Push("Unable to destroy mailer");
	}
	if(mp->Destroy() != 0)
	{
		MailerErrors->Push("Unable destroy params");
	}
}

void DeallocateAll(void)
{
	delete ms;
	delete mm;
	delete mp;
	delete MailerErrors;
	delete ArgParser;
}

void FlushErrors(void)
{
	while(!MailerErrors->Empty())
		std::cerr << "Error: " << MailerErrors->Pop().c_str() << "\n";
}

void FlushDebugs(void)
{
	if(!mp->GetDebugOut()) return;
	while(!MailerErrors->EmptyDebug())
		std::cerr << "Debug: " << MailerErrors->PopDebug().c_str() << "\n";
}

void FlushWarnings(void)
{
	while(!MailerErrors->EmptyWarning())
		std::cerr << "Warning: " << MailerErrors->PopWarning().c_str() << "\n";
}

void FlushAll(void)
{
	FlushErrors();
	FlushWarnings();
	FlushDebugs();
}

int main (int argc, char* argv[])
{
	if(AllocateAll() != 0)
	{
		FlushAll();
		DeallocateAll();
		return -1;
	}
	if(ParamInit(argc, argv) != 0)
	{
		return -1;
	}
	if(ArgParser->Declared("version"))
	{
		std::cout << "SQLMailer version " << SQLMAILER_VERSION << std::endl;
		DeallocateAll();
		return 0;
	}
	if(ArgParser->Declared("usage"))
	{
		std::cout << "SQLMailer version " << SQLMAILER_VERSION << std::endl;
		ArgParser->Usage();
		DeallocateAll();
		return 0;
	}
	
	if(InitAll() != 0)
	{
		FlushAll();
		DestroyAll();
		DeallocateAll();
		return -1;
	}
	
	if(mm->Execute() != 0)
	{
		MailerErrors->Push("Error executing mailer");
	}
	

	DestroyAll();
	FlushAll();
	DeallocateAll();

	return 0;
}
