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
 
 /*  A note on memory leaks. 3CC leaks a ridiculous amount of memory. But
  *  it doesn't matter that terribly much. While it allocates an
  *  absoltely humongous amount of objects, they are mostly REALLY tiny,
  *  so (and valgrind confirms this) over the course of a session, the
  *  total amount of allocated memory is a matter of hundreds of kB, whereas
  *  about a quarter is lost. Adding to this, it only builds up during the
  *  short run time of the program, so when things come around it really
  *  is a non-issue.
  *
  */

#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "type.h"
#include "variable.h"
#include <stdarg.h>
#include <fstream>
#include <unistd.h>

extern void yyparse();


extern int atoinontri(const char* c);

compiler::compiler() {
	lexer = new yyFlexLexer();
	output = stdout;
}

compiler::~compiler() {
}
void compiler::pushA() {
	if(PXY) { PXY=false; this->printf("\t\tPHX\n\t\tPHY\n"); }
	else if(PA) { this->printf("\t\tPSH\tA\n"); }
	PA = true;
}

void compiler::pushXY()	{
	if(PA) { PA = false;this->printf("\t\tPSH\tA\n"); }
	else if(PXY) { this->printf("\t\tPHX\n\t\tPHY\n"); }
	PXY = true;
}

void compiler::pullA() {
	if(PXY) { PXY = false; this->printf("\t\tPHX\n\t\tPHY\n");  }
	if(PA) { PA = false; return; }
	else this->printf("\t\tPLL\tA\n");
}
void compiler::pullXY() {
	if(PA) { PA = false; this->printf("\t\tPSH\tA\n"); }
	if(PXY) { PXY = false; return; }
	else this->printf("\t\tPLY\n\t\tPLX\n");
}

int compiler::printf(const char* arg, ...) {
	if(PA) { PA = false; this->printf("\t\tPSH\tA\n"); }
	if(PXY) { PXY = false; this->printf("\t\tPHX\n\t\tPHY\n");  }

	va_list t;
	va_start(t, arg);
	int ret = vfprintf(output, arg, t);
	va_end(t);

	return ret;
}

void compiler::decl_struct(const char* name, t_struct* typ) {
	if(structs.find(name) != structs.end()) {
		if(*structs[name] != *typ)
			throw new runtime_error("Redeclaration of struct");
		return;
	}

	structs[name] = typ;
}

t_struct* compiler::struct_ref(const char* name) const {
	if(structs.find(name) != structs.end())
		 return (*structs.find(name)).second;

	::printf("Unable to find struct '%s'\n", name);
	exit(EXIT_FAILURE);

}
void compiler::decl_fun(const char* name, const function_prototype& fp) {
	if(functions.find(name) != functions.end()) {
		const function_prototype& fp2 = (*functions.find(name)).second;
		if(fp2 != fp) {
			printf("Redeclaration of function %s\n", name);
			exit(EXIT_FAILURE);
			return;
		}
	} else functions[name] = fp;
}

const function_prototype& compiler::fun_ref(const char* name) const {
	if(functions.find(name) != functions.end())
		 return (*functions.find(name)).second;

	::printf("Unable to find function '%s'\n", name);
	exit(EXIT_FAILURE);

}
void cio::op(const char* op, const char* arg) {
	if(arg) printf("\t%s\t%s\n", op, arg);
	else printf("\t%s\n", op);
}

void cio::label(const char* name) {
	printf("%s:\n", name);
}

void compiler::pragma(const char* command, int data) {
	if(strcasecmp(command, "ORIGIN") == 0) {
		compiler::printf("@ORG\t%d\n", data);
	} else {
		compiler::printf("; Unrecognized pragma %s = %d\n", command, data);
	}
}

void compiler::pragma(const char* command) {
	if(strcasecmp(command, "SCL") == 0) {
		compiler::printf("@EQU\t__CURRENT_LOCATION\t$$\n");
	} else if(strcasecmp(command, "RCL") == 0) {
		compiler::printf("@ORG\t__CURRENT_LOCATION+1\n");
	} else {
		compiler::printf("; Unrecognized pragma %s\n", command);
	}
}

void compiler::begin() {
	compiler::printf("@EQU\ttmp\t%%DDD443\n");
}

void compiler::compile_file(const char* filename) {
	this->filename = filename;
	this->effective_filename = filename;
	this->line = 1;

	ifstream* stream = new ifstream(filename);
	lexer->yyrestart(stream);
	yyparse();
	delete stream;

}
void compiler::set_output(const char* outfile) {
	output = fopen(outfile, "w");
	if(output == NULL) {
		perror(outfile);
		exit(EXIT_FAILURE);
	}
}

int yylex() { return compiler::instance()->get_lexer()->yylex(); }

compiler* compiler::compiler_instance = 0;

void help(const char* command) {
	printf("Usage: %s [-h] [-o filename] [-O origin] file1 [file2 ... fileN]\n", command);
	printf("\t -o\t Set output file\n");
	printf("\t -O\t Set origin\n");
	printf("\t -h\t Show this screen\n\n");
}

int main(int argc, char* argv[]) {
	using namespace cio;

	int optret;
	const char* outfile = NULL;
	int origin = 0;
	compiler* c  = compiler::instance();

	while((optret = getopt(argc, argv, "hco:O:")) != -1) {
		switch(optret) {
			case 'h': help(argv[0]); exit(EXIT_SUCCESS);
			case 'o': outfile = strdup(optarg); break;
			case 'O': origin = atoinontri(optarg); break;
			case ':':
			case '?': help(argv[0]); exit(EXIT_FAILURE);
		}
	}
	int filearg = optind;

	if(origin < atoinontri("0nDDDDDD") || origin > atoinontri("0n444444")) {
		printf("Origin (%d) is outside of addressible memory [%d ... %d].\n",
			origin, atoinontri("0nDDDDDD"), atoinontri("0n444444"));
		return EXIT_FAILURE;
	}

	if(origin < atoinontri("0nCA1000")) {
		printf("Warning! You have specified a dangerously low \n"
		       "origin. This will almost certainly result in \n"
		       "you overwriting some essential part of system \n"
		       "registers, the call stack, the variable stack \n"
		       "or some other sensitive area!\n\n");
	}
	if(outfile) c->set_output(outfile);

	c->begin();
	c->printf("@ORG\t%d\n", origin);

	try {
		while(filearg != argc) {
			c->compile_file(argv[filearg]);

			filearg++;
		}
	} catch (exception* e) {
		fprintf(stderr, "%s[%d]: %s\n", c->get_effective_file(), c->get_line(), e->what());
		exit(EXIT_FAILURE);
	}
	compiler::instance()->printf("; Begin string constants\n");
	compiler::instance()->get_mmgr()->define();


	compiler::instance()->printf("; Define variable stack pointer\n");
	compiler::instance()->printf("__VSS:\t\t@DT %%444\n");

	// TODO: Make this location changeable through pragma or 
	// 	compiler arguments
	compiler::instance()->printf("@ORG\t\t%%CAADDD\n");
	compiler::instance()->printf("; Define variable stack\n");
	compiler::instance()->printf("@REST\t\t%%444\n");
	compiler::instance()->printf("__VS @EQU %%CAA000\n");
}
