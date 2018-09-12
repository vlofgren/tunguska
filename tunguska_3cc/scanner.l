/*  3cc - Ternary C Compiler for Tunguska
 *  Copyright (C) 2008  Viktor Lofgren
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
%option noyywrap
%option	yylineno
%{

#include "type.h"
#include "expression.h"
#include "parser.tab.hpp"

extern void newline();
int tritodec(const char* c) {
	int value = 0;
	while(*c) {
		value*=3;
		switch(*c) {
			case 'N':
			case '-': value -= 1; break;
			case '0':
			case 'O': break;
			case 'P':
			case '+': value +=1; break;
			default: printf("'%c' in tritodec\n", *c); break; // Lexer malfunction
		}
		c++;
	}
	return value;
}

int nontodec(const char* c) {
	int value = 0;
	while(*c) {
		value*=9;
		switch(*c) { // Support both ABCD01234 and QWER01234
			case 'R':
			case 'D': value-=4; break;
			case 'E':
			case 'C': value-=3; break;
			case 'W':
			case 'B': value-=2; break;
			case 'Q':
			case 'A': value-=1; break;
			case '0': break;
			case '1': value+=1; break;
			case '2': value+=2; break;
			case '3': value+=3; break;
			case '4': value+=4; break;
			default: printf("'%c' in nontodec\n", *c); break; // Lexer malfunction
		}
		c++;
	}
	return value;
}

int atoinontri(const char* s) {
	if(s[0] == '0') {
		if(s[1] == 'n') return nontodec(s+2);
		else if(s[1] == 't') return tritodec(s+2);
	}

	return atoi(s);
}

int char_constant(char c) {
	char translation[] = {  0, ' ','\n','\t','\b',   0,  0,   0,   0,   0,
			    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
			    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			    'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3',
			    '4', '5', '6', '7', '8', '9', '.', ',', '!', '?',
			    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
			    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
			    'u', 'v', 'w', 'x', 'y', 'z',  0 ,   0,   0,   0,
			      0,   0,   0,   0,   0,   0, '=', '-', '*', '/',
			    '%', '<', '>',   0,   0, '(', ')', '$', '+', '#' };

	for(int i  = 0; i < 100; i++) {
		if(translation[i] == c) return i;

	}

	throw new std::runtime_error("Unsupported character");

}

int escape_char(char c) {
	switch(c) {
		case '\\': return char_constant(c);
		case '\'': return char_constant(c);
		case 'n': return 2;
		case '0': return 0;
		default: throw new std::runtime_error("Unimplemented/unsupported escape sequence");
	}
}

#include <string.h>
extern YYSTYPE yylval;
%}
%s METASTATE
%%
\/\/.+$				//
\/\*(^\*\/)+			//
\"[^\"]*\"			yylval.strval = strndup(yytext+1, strlen(yytext)-2); return STRING; 
char				yylval.tval = new t_i6(); return TYPE;
int				yylval.tval = new t_i12(); return TYPE;
void				yylval.tval = new t_void(); return TYPE;
return				return RETURN;
static				return STATIC;
if				return IF;
else				return ELSE;
while				return WHILE;
for				return FOR;
asm				return ASM;
continue			return CONTINUE;
extern				return EXTERN;
volatile			return VOLATILE;
reentrant			return REENTRANT;
break				return BREAK;
sizeof				return SIZEOF;
struct				return STRUCT;
typedebug			return TYPEDEBUG;
sptest				return SPTEST;
\-\>				return PTRFIELD;
\#				BEGIN(METASTATE); return '#';
<METASTATE>\n			BEGIN(INITIAL); newline(); return '\n'; 
pragma				return PRAGMA;
0n[ABCDQWER01234]+		yylval.ival = atoinontri(yytext); return DECIMAL;
0t[N\-0OP\+]+			yylval.ival = atoinontri(yytext); return DECIMAL;
[0-9]+				yylval.ival = (int) atoi(yytext); return DECIMAL;
\'\\[^\']\'			yylval.ival = escape_char(*(yytext+2)); return DECIMAL;
\'[^\'\\]\'			yylval.ival = char_constant(*(yytext+1)); return DECIMAL;
[a-zA-Z_][a-zA-Z0-9_]*		yylval.strval = (char*) strdup((char*)yytext); return IDENTIFIER;
\(				return '(';
\)				return ')';
\{				return '{';
\}				return '}';
[\[]				return '[';
[\]]				return ']';

\|\|				return LOG_OR;
\&\&				return LOG_AND;

\|				return '|';
\^				return '^';
\&				return '&';
\~				return '~';
\?				return '?';
\:				return ':';

\>				return '>';
\<				return '<';
\>\=				return GE;
\<\=				return LE;
\=\=				return EQ;
\!\=				return NE;

\+\=				return ADDASSIGN;
\-\=				return SUBASSIGN;
\*\=				return MULASSIGN;
\&\=				return ANDASSIGN;
\^\=				return XORASSIGN;
\|\=				return ORASSIGN;

\<P\>				return PRM;
\<S\>				return TSH;

\+\+				return INC;
\-\-				return DEC;

\=				return '=';
\+				return '+';
\-				return '-';
\%				return '%';
\/				return '/';
\*				return '*';
\,				return ',';
\;				return ';';
\.				return '.';

\n				newline();
<*>[\t\ ]+			//


%%
