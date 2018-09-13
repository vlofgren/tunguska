APPNAME=tunguska_3cc
CFLAGS=-Wall -g
CXXFLAGS=-Wall -g -std=c++98
LDFLAGS=-dH -g
all: $(APPNAME)
parser.tab.cpp: parser.ypp
	bison -d parser.ypp
lex.yy.cc: scanner.l
	flex -+ scanner.l
$(APPNAME): variable.o compiler.o parser.tab.o lex.yy.o expression.o bivalent.o function.o conditional.o memory.o tables.o
	g++ -o $(APPNAME) *.o $(CXXFLAGS)
squeaky:
	rm -rf $(APPNAME) *.o lex.yy.cc parser.tab.* *~
clean: squeaky
test:
	./compile.sh test.c > out.asm
	tg_assembler out.asm
