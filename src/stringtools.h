/*
 *  stringtools.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <string>
#include <vector>

typedef std::vector<std::string> STStringVector;
void StringSplit(STStringVector& rVec, std::string aString, std::string aDelim);
void StringJoin(STStringVector aVec, std::string &rString, std::string aDelim);
std::string StringLeftTrim(std::string aString);
std::string StringRightTrim(std::string aString);
std::string StringTrim(std::string aString);
std::string StringLineWrap(std::string aLine, std::string::size_type aLineLen, std::string aBreak);
std::string StringFromInt(const int aNumber);

#endif
