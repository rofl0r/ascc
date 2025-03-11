prefix=/usr/local
bindir=$(prefix)/bin

ASCC_SRCS = \
	main.cpp \
	cs_parser.cpp \
	cc_error.cpp \
	cc_treemap.cpp \
	cc_internallist.cpp \
	cc_symboltable.cpp \
	fmem.cpp \
	cc_compiledscript.cpp \
	cs_parser_common.cpp \
	cc_options.c \
	cc_error_at_line.cpp \
	cc_script.cpp \
	util_string_compat.c \
	util_string.cpp \
	util_stream.cpp \
	cs_compiler.cpp \
	cc_macrotable.cpp \
	cs_prepro.cpp \
	tokenizer.c \
	eval_if.c

TEMPOBJS = $(ASCC_SRCS:.cpp=.o)
ASCC_OBJS = $(TEMPOBJS:.c=.o)

-include config.mak

CXXFLAGS += -fpermissive

all: ascc

ascc: $(ASCC_OBJS)
	$(CXX) -o ascc $(ASCC_OBJS)

clean:
	rm -f ascc *.o

test:
	sh test.sh

install: ascc
	install -Dm 644 ascc $(DESTDIR)$(bindir)/ascc


.PHONY: all clean test install
