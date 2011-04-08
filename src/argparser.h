/*
 *  argparser.h
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
 *
 */

/*
	TODO:
		- mandatory argument checking
		- usage
		- version
		- testing
*/

#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <string>
#include <vector>
#include <map>
#include "defines.h"

	typedef std::vector<std::string> tValueVector;
	enum EParseState { eParseArg, eParseMod };
	enum EParseToDo { eParseContinue, eParseBreak, eParseError, eParseNone };

	// Short, long, name, pcount, multiple, mandatory, default, modifiers, comment
	struct sArgument
	{
		const char* short_arg;
		const char* long_arg;
		const char* name;
		const int pcount;
		const bool multiple;
		const bool mandatory;
		const char* default_value;
		const char* modifiers;
		const char* comment;
		bool declared;
		tValueVector values;
		std::string value;
		sArgument(const char pShort[], const char pLong[], const char pName[], const int pPCount,
							const bool pMultiple, const bool pMandatory, const char pDefault[],
							const char pModifiers[], const char pComment[]) : 
					short_arg(pShort), long_arg(pLong), name(pName), pcount(pPCount),
					multiple(pMultiple), mandatory(pMandatory), default_value(pDefault),
					modifiers(pModifiers), comment(pComment)
			{
				declared = false;
				value = pDefault;
			}
	};
	typedef std::map<std::string, struct sArgument*> tArgumentMap;
	
	class CArgParser
	{
		private:
			tArgumentMap iArguments;
		protected:
			EParseToDo FASTCALL ParseMod(std::string aLastArg, char const aArg[], EParseState& rState, int aI, bool aSkipFirst, int& rPCount);
			EParseToDo FASTCALL ParseArg(std::string& rLastArg, char const aArg[], EParseState& rState, int& rPCount);
		public:
			static void FASTCALL Allocate(void);
			static void FASTCALL DeAllocate(void);
			CArgParser();
			~CArgParser();
			int FASTCALL Parse(const int argc, char* const argv[], bool aSkipFirst=true);
			void FASTCALL Add(const char pShort[], const char pLong[], const char pName[], const int pPCount,
							const bool pMultiple, const bool pMandatory, const char pDefault[],
							const char pModifiers[], const char pComment[]);
			std::string Value(std::string aName);
			bool Declared(std::string aName);
			void Usage(void);
	};
	extern CArgParser* ArgParser;

#endif
