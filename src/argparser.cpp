/*
 *  argparser.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/12/06.
 *  Copyright 2006 Jani Hast <jani.hast@pp.inet.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <iostream>
#include "argparser.h"

CArgParser* ArgParser = NULL;

void FASTCALL CArgParser::Allocate(void) { ArgParser = new CArgParser(); }
void FASTCALL CArgParser::DeAllocate(void) { delete ArgParser; }

CArgParser::CArgParser()
{
}

CArgParser::~CArgParser()
{
	for(tArgumentMap::iterator i = iArguments.begin();
				i != iArguments.end(); i++)
		delete (*i).second;
}

EParseToDo FASTCALL CArgParser::ParseMod(std::string rLastArg, char const aArg[], EParseState& rState, int aI, bool aSkipFirst, int& rPCount)
{
	if(aI == 0 && aSkipFirst)
	{
		rState = eParseArg;
		return eParseContinue;
	}
	if(iArguments[rLastArg]->multiple)
	{
		iArguments[rLastArg]->values.push_back(aArg);
	}
	else
	{
		iArguments[rLastArg]->value = aArg;
	}
	iArguments[rLastArg]->declared = true;
	return eParseNone;
}

EParseToDo FASTCALL CArgParser::ParseArg(std::string& rLastArg, char const aArg[], EParseState& rState, int& rPCount)
{
	std::string arg = aArg;
	bool handled = false;
	for(tArgumentMap::iterator i = iArguments.begin();
				i != iArguments.end(); i++)
	{
		if((*i).second->short_arg != arg && (*i).second->long_arg != arg)
			continue;
		handled = true;
		if((*i).second->pcount > 0)
		{
			rLastArg = (*i).second->name;
			rPCount = (*i).second->pcount;
			rState = eParseMod;
			return eParseContinue;
		}
		else
		{
			rPCount = 0;
			(*i).second->declared = true;
			rState = eParseArg;
			return eParseContinue;
		}
	}
	if(!handled)
	{
		return eParseError;
	}
	return eParseNone;
}

int FASTCALL CArgParser::Parse(const int argc, char* const argv[], bool aSkipFirst)
{
	EParseState state = aSkipFirst?eParseMod:eParseArg;
	EParseToDo todo = eParseNone;
	std::string last_arg;
	int rcount = 0;
	for(int i = 0; i < argc; i++)
	{
		switch(state)
		{
			case eParseMod:
				todo = ParseMod(last_arg, argv[i], state, i, aSkipFirst, rcount);
			break;
			case eParseArg:
				todo = ParseArg(last_arg, argv[i], state, rcount);
			break;
		}
		if(todo == eParseContinue) continue;
		else if(todo == eParseBreak) break;
		else if(todo == eParseError) return -1;
	}
	return 0;
}

void FASTCALL CArgParser::Add(const char pShort[], const char pLong[], const char pName[], const int pPCount,
							const bool pMultiple, const bool pMandatory, const char pDefault[],
							const char pModifiers[], const char pComment[])
{
	iArguments[pName] =
		new struct sArgument(pShort, 
							pLong,
							pName,
							pPCount,
							pMultiple, 
							pMandatory,
							pDefault,
							pModifiers,
							pComment);
}

std::string CArgParser::Value(std::string aName)
{
	return iArguments[aName]->value;
}

bool CArgParser::Declared(std::string aName)
{
	return iArguments[aName]->declared;
}

void CArgParser::Usage(void)
{
	std::cout << "Usage:" << std::endl;
}
