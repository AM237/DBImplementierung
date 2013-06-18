
CXX = g++ -O3 -Wall -g -Wno-deprecated -std=c++0x
MAIN_BINARIES = $(basename $(wildcard *Main.cpp))
TEST_BINARIES = $(basename $(wildcard *Test.cpp))

SUBDIRS = 
OBJECTS := $(addsuffix .o,$(basename $(filter-out %Main.cpp %Test.cpp,$(wildcard *.cpp))))
SUBDIROBJECTS = $(foreach dir, $(SUBDIRS), $(addsuffix .o,$(basename $(filter-out %Main.cpp %Test.cpp,$(wildcard $(dir)/*.cpp)))))

MAIN_LIBS = -pthread
TEST_LIBS = -lgtest -lpthread
DEBUG = -ggdb
RELEASE = buffermanager

subdirs: 
	for T in $(SUBDIRS); do cd $$T && $(MAKE) compile -C $$T; done

all: clean compile test release

compile: subdirs $(MAIN_BINARIES) $(TEST_BINARIES)

test: subdirs $(TEST_BINARIES)
	 for T in $(TEST_BINARIES); do ./$$T; done

cleanhere: 
	rm -f *\~
	rm -f *.o
	rm -f $(MAIN_BINARIES)
	rm -f $(TEST_BINARIES)
	rm -f *.class
	rm -f core

cleansubdirs:
	for T in $(SUBDIRS); do cd $$T && make clean; done

clean: cleansubdirs cleanhere
	
release: clean compile
	mv $(MAIN_BINARIES) ./bin
	mv $(TEST_BINARIES) ./bin
	mv ./bin/$(MAIN_BINARIES) ./bin/$(RELEASE)
	mv ./bin/$(TEST_BINARIES) ./bin/$(RELEASE)_test
	
	
# Dependent library files, if defined, may be needed AFTER the definition
# of the object files, and not BEFORE ...
%Main: %Main.o $(OBJECTS) $(SUBDIROBJECTS)
	$(CXX) -o $@ $^ $(DEBUG) $(MAIN_LIBS)
	
%Test: %Test.o $(OBJECTS) $(SUBDIROBJECTS)
	$(CXX) -o $@ $^ $(TEST_LIBS)

%.o: %.cpp $(HEADER)
	$(CXX) -c $<
	
# Do not delete intermediate files, .o files in sub directories needed for linking	
.SECONDARY: 
	
# Phonys
.PHONY: subdirs $(SUBDIRS)
