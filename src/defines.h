/*
 *  defines.h
 *  sqlmailer
 *
 *  Created by Jani Hast on 12/9/06.
 *  Copyright 2006 OpenHosting Harri / Antti Harri <iku@openbsd.fi>
 *
 */

#ifndef DEFINES_H
#define DEFINES_H

	#define SQLMAILER_VERSION "1.3"
	#define CONFIG_VERSION 1.4
	#define CONFIG_VERSION_STR "1.4"
	#define ADDRESS_DELIMITER ";"
	#define MAX_DATA_READ_FROM_SOCKET 512
	#ifdef __APPLE_CC__
		#define FASTCALL
	#else
		#define FASTCALL 
		//__fastcall
	#endif

#endif
