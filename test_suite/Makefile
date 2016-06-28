ifneq (,$(shell which clang++ 2>/dev/null))
CXX=clang++
CXXFLAGS = -I$(TD_ROOT)/include -I$(TBBROOT)/include -ftaskdebug -g
else
CXX=g++
CXXFLAGS = -I$(TD_ROOT)/include -I$(TBBROOT)/include -O3
endif # clang++

#LIBS = -L$(TBBROOT)/obj -ltbb -L$(TD_ROOT)/obj -ltdebug
LIBS = -ltbb -L$(TD_ROOT)/obj -ltdebug

SRCS = $(wildcard *.cpp)

PROGS = $(patsubst %.cpp,%,$(SRCS))

all: $(PROGS)

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -rf $(PROGS) *~ *.out util.pyc traces
