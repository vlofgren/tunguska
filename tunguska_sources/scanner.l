/* Tunguska, ternary virtual machine
 *
 * Copyright (C) 2007-2009 Viktor Lofgren
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

%option noyywrap
%option yylineno
%option c++
%{
#include "parser.h"

/* Non-GNU compatibility */
#ifndef __linux__
char* strndup(const char* string, unsigned int length) {
	char* rv = NULL;

	if(strlen(string) < length) return strdup(string);

	rv = (char*) malloc(length+1);
	memcpy(rv, string, length);
	rv[length] = '\0';

	return rv;
}
#endif

/* Some tool configurations generate broken scanner code
 * without defining EOF. This fixes the problem,
 * while undoubtedly breaking other configurations. */
#ifndef EOF
# define EOF -1
#endif

extern YYSTYPE yylval;
%}
%%
\;.+$				//
\%[0-4A-D]{3}			yylval.sval = strdup(yytext); return NONTRIP;
\%[0-4A-D]{6}			yylval.sval = strdup(yytext); return NONSEXT;
[0-9]+				yylval.ival = atoi((char*)yytext); return DECIMAL;
\-?[0-9.]+f			yylval.sval = strdup(yytext+2); return FLOATVAL;
LOW				return LOW;
HIGH				return HIGH;
\'[^\']*\'			yylval.sval = strndup(yytext+1, strlen(yytext)-2); return STRING;
\#				return '#';
\@EQU				return EQU;
\@INC				return _INC;
[X]				return XREG;
[Y]				return YREG;
[A]				return ACCUMULATOR;
\@ORG				return ORG;
\@DT				return DT;
\@DW				return DW;
\@REST				return REST;
\@ASMDEBUG			return ASMDEBUG;
\$\$				return HERE;
\(				return '(';
\)				return ')';
[A-z\.][A-z\_\.0-9]*		yylval.sval = strdup((char*)yytext); return IDENTIFIER;
\n				return NEWLINE;
[ \t]+				//
\:				return ':';
\;[^\n]*			return COMMENT;
\,				return ',';
\*				return '*';
\/				return '/';
\+				return '+';
\-				return '-';
\{				return '{';
\}				return '}';
%%
