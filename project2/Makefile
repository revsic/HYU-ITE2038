.SUFFIXES: .c .o

# gcc 5.4.0
CC=gcc

SRCDIR=src/
TESTSRC=testsrc/
INC=include/
LIBS=lib/

# SRCS:=$(wildcard src/*.c)
# OBJS:=$(SRCS:.c=.o)

# main source file
TARGET_SRC:=$(SRCDIR)main.c
TARGET_OBJ:=$(SRCDIR)main.o

# test source file
TARGET_TEST_SRC:=$(TESTSRC)*.c

# Include more files if you write another source file.
SRCS_FOR_LIB:=$(SRCDIR)bpt.c ${SRCDIR}dbapi.c ${SRCDIR}disk_manager.c ${SRCDIR}fileio.c ${SRCDIR}utility.c
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.c=.o)

CFLAGS+= -g -fPIC -I $(INC)

TARGET=main
TARGET_TEST=test

all: $(TARGET) $(TARGET_TEST)

$(SRCDIR)bpt.o:
	$(CC) $(CFLAGS) -o $(SRCDIR)bpt.o -c $(SRCDIR)bpt.c

$(SRCDIR)dbapi.o:
	$(CC) $(CFLAGS) -o $(SRCDIR)dbapi.o -c $(SRCDIR)dbapi.c

$(SRCDIR)disk_manager.o:
	$(CC) $(CFLAGS) -o $(SRCDIR)disk_manager.o -c $(SRCDIR)disk_manager.c

$(SRCDIR)fileio.o:
	$(CC) $(CFLAGS) -o $(SRCDIR)fileio.o -c $(SRCDIR)fileio.c

$(SRCDIR)utility.o:
	$(CC) $(CFLAGS) -o $(SRCDIR)utility.o -c $(SRCDIR)utility.c

$(TARGET): $(TARGET_OBJ)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

$(TARGET_TEST): $(TARGET_TEST_SRC)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(TARGET_TEST) $(OBJS_FOR_LIB) $(LIBS)* testfile

library: $(OBJS_FOR_LIB)
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library: $(OBJS_FOR_LIB)
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB)
