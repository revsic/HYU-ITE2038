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
# -std=c++14: c++ standard 14
CFLAGS+= -g -fPIC -I $(INC) -std=c++14

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
	$(CXX) $(CFLAGS) -o $(SRCDIR)disk_manager.o -c $(SRCDIR)disk_manager.cpp

$(SRCDIR)buffer_manager.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)buffer_manager.o -c $(SRCDIR)buffer_manager.cpp

$(SRCDIR)bptree.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)bptree.o -c $(SRCDIR)bptree.cpp

$(SRCDIR)table_manager.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)table_manager.o -c $(SRCDIR)table_manager.cpp

$(SRCDIR)dbms.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)dbms.o -c $(SRCDIR)dbms.cpp

$(SRCDIR)dbapi.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)dbapi.o -c $(SRCDIR)dbapi.cpp

$(TARGET_OBJ): $(TARGET_SRC)
	$(CXX) $(CFLAGS) -o $@ -c $^

$(TARGET): $(TARGET_OBJ)
	make static_library
	$(CXX) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

$(TARGET_TEST): $(SRCS_FOR_LIB) $(TARGET_TEST_SRC)
	$(CXX) $(CFLAGS) -DTEST_MODULE -I $(TESTSRC) -o $@ $^

$(TARGET_PERF): $(TARGET_PERF_SRC)
	make static_library
	$(CXX) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(TARGET_TEST) $(TARGET_PERF) $(OBJS_FOR_LIB) $(LIBS)* *.db

library: $(OBJS_FOR_LIB)
	g++ -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library: $(OBJS_FOR_LIB)
	if [ ! -e "$(LIBS)" ]; then mkdir $(LIBS); fi;
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB)
