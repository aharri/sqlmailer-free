/*
 *  stringtools.cpp
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#include <sstream>
#include "stringtools.h"

void StringSplit(STStringVector& rVec, std::string aString, std::string aDelim)
{
	STStringVector::size_type i = 0;
	STStringVector::size_type j = 0;
	while((i = aString.find_first_of(aDelim, j)) != std::string::npos)
	{
		rVec.push_back(aString.substr(j, i-j));
		j = i+1;
	}
	if(j < aString.length())
	{
		rVec.push_back(aString.substr(j, std::string::npos));
	}
}

void StringJoin(STStringVector aVec, std::string &rString, std::string aDelim)
{
	for(STStringVector::size_type i = 0; i < aVec.size(); i++)
	{
		rString += aVec.at(i);
		if(i < aVec.size()-1)
			rString += aDelim;
	}
}

std::string StringLeftTrim(std::string aString)
{
	std::string::iterator ite = aString.begin();
	for(std::string::size_type i = 0; i < aString.length(); i++)
	{
		if((*ite) >= 0x21 && (*ite) <= 126)
		{
			return aString.substr(i, aString.length()-i);
		}
		ite ++;
	}
	return aString;
}

std::string StringRightTrim(std::string aString)
{
	std::string::iterator ite = aString.end();
	for(std::string::size_type i = aString.length()-1; i >= 0 ; i--)
	{
		if((*ite) >= 0x21 && (*ite) <= 126)
		{
			return aString.substr(i, aString.length()-i);
		}
		ite --;
	}
	return aString;
}

std::string StringTrim(std::string aString)
{
	aString = StringLeftTrim(aString);
	return StringLeftTrim(aString);
}

std::string StringLineWrap(std::string aLine, std::string::size_type aLineLen, std::string aBreak)
{
	if(aLineLen == 0 || aLineLen == 0)
		return aLine;
	std::string::size_type pos = 0;
	std::string ret = "", temp = "";
	while(aLine.length() > 0)
	{
		if(aLine.length() <= aLineLen)
		{
			ret += aLine + aBreak;
			aLine = "";
			break;
		}
		temp = aLine.substr(0, aLineLen);
		pos = temp.rfind(aBreak);
		if(pos > 0 && pos != std::string::npos)
		{
			ret += temp = temp.substr(0, pos);
			aLine = aLine.replace(0, temp.length()-1, "");
			continue;
		}
		pos = temp.find_last_of(" ");
		if(pos > 0)
		{
			ret += temp = temp.substr(0, pos) + aBreak;
			aLine = aLine.replace(0, temp.length()-1, "");
			continue;
		}
		ret += temp + aBreak;
		aLine = aLine.replace(0, temp.length()-1, "");
	}
	return ret;
}

std::string StringFromInt(const int aNumber)
{
  std::ostringstream ret;
  ret << aNumber;
  return ret.str();
}

