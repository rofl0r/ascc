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

TOOLCHAIN := $(shell $(CC) -dumpmachine || echo 'unknown')

ifeq ($(findstring mingw,$(TOOLCHAIN)),mingw)
WIN=1
endif
ifeq ($(findstring cygwin,$(TOOLCHAIN)),cygwin)
WIN=1
endif

ifdef WIN
EXE_EXT=.exe
endif

PROG=ascc$(EXE_EXT)

all: $(PROG)

$(PROG): $(ASCC_OBJS)
	$(CXX) -o $@ $(ASCC_OBJS)

clean:
	rm -f ascc *.o

test:
	sh test.sh

install: $(PROG)
	install -Dm 644 $(PROG) $(DESTDIR)$(bindir)/$(PROG)


.PHONY: all clean test install
