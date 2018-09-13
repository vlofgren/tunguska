ASSEMBLER=../tunguska_sources/tg_assembler
TRICC=../tunguska_3cc/tunguska_3cc
SOURCES=string.c stdio.c math.c main.c system.c graphics.c demos.c

PP_SOURCES=$(SOURCES:.c=.ppc)

all: image clean_mini

# Compile image
image: $(PP_SOURCES)
	$(TRICC) -O 0n400000 -oout.asm $(PP_SOURCES)
	$(ASSEMBLER) out.asm
vectors: examples/vectors.ppc
	$(TRICC) -ovectors.asm examples/vectors.ppc
	$(ASSEMBLER) vectors.asm
backtrace: $(PP_SOURCES)
	echo -e "run $(PP_SOURCES)\nbacktrace\nlist\nstop\nquit" > backtrace
	gdb $(TRICC) -batch -x backtrace
	rm backtrace

# Remove preprocessed sources
clean_mini: 
	rm -rf *.ppc examples/*ppc *ppcE
clean: clean_mini
	rm -rf out.asm out.ternobj
# Tell make to use suffix rules for targets ending in .ppc
.SUFFIXES: .ppc

# Tell make how to preprocess sources
.c.ppc:
	cpp -P $< -o $@
