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
	cc_options.cpp \
	cc_error_at_line.cpp \
	cc_script.cpp \
	util_string_compat.c \
	util_string.cpp \
	util_stream.cpp \
	cs_compiler.cpp \
	cc_macrotable.cpp \
	cs_prepro.cpp \


TEMPOBJS = $(ASCC_SRCS:.cpp=.o)
ASCC_OBJS = $(TEMPOBJS:.c=.o)

CXXFLAGS += -fpermissive -O0 -g3

all: ascc

ascc: $(ASCC_OBJS)
	$(CXX) -o ascc $(ASCC_OBJS)

clean:
	rm -f ascc *.o

test:
	sh test.sh


.PHONY: all clean test
