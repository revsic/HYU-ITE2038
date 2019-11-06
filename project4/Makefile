.SUFFIXES: .cpp .o

# g++ std11
CXX=g++

# directories
SRCDIR=src/
APPDIR=app/
TESTSRC=testsrc/
INC=include/
LIBS=lib/

# main source file
TARGET_SRC:=$(APPDIR)main.cpp
TARGET_OBJ:=$(APPDIR)main.o

# test source file
TARGET_TEST_SRC:=$(TESTSRC)*.cpp

# perf source file
TARGET_PERF_SRC:=$(APPDIR)perf.cpp

SRCS_FOR_LIB:=$(wildcard $(SRCDIR)*.cpp)
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.cpp=.o)

# -g: source level debug info
# -fPIC: position independent code
# -I: include path
# -std=c++11: c++ standard 11
CFLAGS+= -g -fPIC -I $(INC) -std=c++11

# target file
TARGET=main
TARGET_TEST=test
TARGET_PERF=perf

all: $(TARGET)

$(SRCDIR)fileio.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)fileio.o -c $(SRCDIR)fileio.cpp

$(SRCDIR)headers.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)headers.o -c $(SRCDIR)headers.cpp

$(SRCDIR)disk_manager.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)disk_manager.o -c $(SRCDIR)disk_manager.c

# $(SRCDIR)utility.o:
# 	$(CC) $(CFLAGS) -o $(SRCDIR)utility.o -c $(SRCDIR)utility.c

# $(SRCDIR)buffer_manager.o:
# 	$(CC) $(CFLAGS) -o $(SRCDIR)buffer_manager.o -c $(SRCDIR)buffer_manager.c

# $(SRCDIR)table.o:
# 	$(CC) $(CFLAGS) -o $(SRCDIR)table.o -c $(SRCDIR)table.c

# $(SRCDIR)table_manager.o:
# 	$(CC) $(CFLAGS) -o $(SRCDIR)table_manager.o -c $(SRCDIR)table_manager.c

# $(SRCDIR)dbms.o:
# 	$(CC) $(CFLAGS) -o $(SRCDIR)dbms.o -c $(SRCDIR)dbms.c

# $(SRCDIR)dbapi.o:
# 	$(CC) $(CFLAGS) -o $(SRCDIR)dbapi.o -c $(SRCDIR)dbapi.c

# $(SRCDIR)bpt.o:
# 	$(CC) $(CFLAGS) -o $(SRCDIR)bpt.o -c $(SRCDIR)bpt.c

$(TARGET): $(TARGET_OBJ)
	make static_library
	$(CXX) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

$(TARGET_TEST): $(TARGET_TEST_SRC)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

# $(TARGET_PERF): $(TARGET_PERF_SRC)
# 	make static_library
# 	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(TARGET_TEST) $(TARGET_PERF) $(OBJS_FOR_LIB) $(LIBS)* *.db

library: $(OBJS_FOR_LIB)
	g++ -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library: $(OBJS_FOR_LIB)
	if [ ! -e "$(LIBS)" ]; then mkdir $(LIBS); fi;
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB)
