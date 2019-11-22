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

# test purpose application file
TARGET_TESTAPP_SRC:=$(APPDIR)testapp.cpp

SRCS_FOR_LIB:=$(wildcard $(SRCDIR)*.cpp)
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.cpp=.o)

# -g: source level debug info
# -fPIC: position independent code
# -I: include path
# -std=c++14: c++ standard 14
# -O3: optimization level 3
CFLAGS+= -fPIC -I $(INC) -std=c++14 -g # -O3

# target file
TARGET=main
TARGET_TEST=test
TARGET_PERF=perf
TARGET_TESTAPP=testapp

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

$(SRCDIR)bptree_iter.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)bptree_iter.o -c $(SRCDIR)bptree_iter.cpp

$(SRCDIR)table_manager.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)table_manager.o -c $(SRCDIR)table_manager.cpp

$(SRCDIR)dbms.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)dbms.o -c $(SRCDIR)dbms.cpp

$(SRCDIR)dbapi.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)dbapi.o -c $(SRCDIR)dbapi.cpp

$(SRCDIR)join.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)join.o -c $(SRCDIR)join.cpp

$(SRCDIR)xaction_manager.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)xaction_manager.o -c $(SRCDIR)xaction_manager.cpp

$(SRCDIR)lock_manager.o:
	$(CXX) $(CFLAGS) -o $(SRCDIR)lock_manager.o -c $(SRCDIR)lock_manager.cpp

$(TARGET_OBJ): $(TARGET_SRC)
	$(CXX) $(CFLAGS) -o $@ -c $^

$(TARGET): $(TARGET_OBJ)
	make static_library
	$(CXX) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt -lpthread

$(TARGET_TEST): $(SRCS_FOR_LIB) $(TARGET_TEST_SRC)
	$(CXX) $(CFLAGS) -DTEST_MODULE -I $(TESTSRC) -o $@ $^ -lpthread

$(TARGET_PERF): $(TARGET_PERF_SRC)
	make static_library
	$(CXX) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt -lpthread

$(TARGET_TESTAPP): $(TARGET_TESTAPP_SRC)
	make static_library
	$(CXX) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt -lpthread

clean:
	rm $(TARGET) $(TARGET_OBJ) $(TARGET_TEST) $(TARGET_PERF) $(TARGET_TESTAPP) $(OBJS_FOR_LIB) $(LIBS)* *.db

library: $(OBJS_FOR_LIB)
	g++ -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library: $(OBJS_FOR_LIB)
	if [ ! -e "$(LIBS)" ]; then mkdir $(LIBS); fi;
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB)
