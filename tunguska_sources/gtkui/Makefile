DEBUG=-ggdb
PKGLIBS=gtk+-2.0 gtksourceview-2.0 gio-2.0
INCLUDES="../"
LDFLAGS=`pkg-config ${PKGLIBS} --libs` -lfl -lz ${DEBUG}
CFLAGS=`pkg-config ${PKGLIBS} --cflags` -I${INCLUDES} -O3 -funroll-loops -Wall -DUSE_GTKSOURCEVIEW ${DEBUG}
CXXFLAGS=${CFLAGS}
LEX=flex

LOCAL_OBJS=tunguska_gtk.o processor_monitor.o variable_tracker.o code_editor.o inspector.o  file_manager.o tunguska_wrapper.o
TUNGUSKA_OBJS=../machine.o ../trit.o ../tryte.o ../disk.o ../memory.o ../interrupt.o ../agdp.o
all: tunguska_gtk
tunguska_gtk: ${LOCAL_OBJS}
	g++ ${LOCAL_OBJS} ${TUNGUSKA_OBJS} -o tunguska_gtk ${LDFLAGS}
stats: clean
	wc -l *{cc,h}
clean:
	rm -rf *.o tunguska_gtk
