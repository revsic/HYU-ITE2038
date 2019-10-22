.SUFFIXES: .c .o

# gcc 5.4.0
CC=gcc

SRCDIR=src/
APPDIR=app/
TESTSRC=testsrc/
INC=include/
LIBS=lib/

# main source file
TARGET_SRC:=$(APPDIR)main.c
TARGET_OBJ:=$(APPDIR)main.o

# test source file
TARGET_TEST_SRC:=$(TESTSRC)*.c

SRCS_FOR_LIB:=$(wildcard $(SRCDIR)*.c)
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.c=.o)

CFLAGS+= -g -fPIC -I $(INC) -Wno-format

TARGET=main
TARGET_TEST=test

all: $(TARGET)

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

$(SRCDIR)buffer_manager.o:
	$(CC) $(CFLAGS) -o $(SRCDIR)buffer_manager.o -c $(SRCDIR)buffer_manager.c

$(SRCDIR)table_manager.o:
	$(CC) $(CFLAGS) -o $(SRCDIR)table_manager.o -c $(SRCDIR)table_manager.c

$(TARGET): $(TARGET_OBJ)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

$(TARGET_TEST): $(TARGET_TEST_SRC)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(TARGET_TEST) $(OBJS_FOR_LIB) $(LIBS)*

library: $(OBJS_FOR_LIB)
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library: $(OBJS_FOR_LIB)
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB)
