DEBUG=#-ggdb
PKGLIBS=sdl
LDFLAGS=`pkg-config ${PKGLIBS} --libs` -lz ${DEBUG} -ggdb 
CXXFLAGS=`pkg-config ${PKGLIBS} --cflags` -O0 -funroll-loops -Wall ${DEBUG} -ggdb  -std=c++98
CFLAGS=${CXXFLAGS}
LEX=flex
#
TGTK=
#set TGTK=tunguska-gtk to compile partially implemented gtk front end

all: tunguska tg_assembler $(TGTK)
tunguska-gtk:
	make -C gtkui all
tunguska: trit.o tryte.o machine.o tunguska.o display.o keyboard.o interrupt.o disk.o agdp.o memory.o 
	g++ trit.o tryte.o machine.o tunguska.o interrupt.o display.o keyboard.o disk.o agdp.o memory.o -o tunguska ${LDFLAGS}
parser.tab.cpp: parser.ypp
	bison -d parser.ypp
	mv parser.tab.hpp parser.h
	sed -i 's/parser.tab.hpp/parser.h/g' parser.tab.cpp
lex.yy.cc: scanner.l
	flex -+ scanner.l
tg_assembler: trit.o tryte.o machine.o interrupt.o parser.tab.o lex.yy.o assembler.o memory.o agdp.o
	g++ trit.o tryte.o machine.o interrupt.o assembler.o parser.tab.o lex.yy.o memory.o agdp.o -o tg_assembler ${LDFLAGS}

stats:	clean
	wc -l *{cc,h,ypp,l}
	make -C gtkui stats
clean:
	rm -rf *.o *~ tg_assembler tunguska *.tab.* *.yy.* parser.h
	make -C gtkui clean
