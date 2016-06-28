TARGET   = libtdebug
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
TD_ROOT := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
CXX=g++
LINKER=g++
LDFLAGS+= -shared
OPT=-O3

CXXFLAGS+=-fPIC -g -Wall

INCPATH=-I./include -I../tbb-lib/include
ARFLAGS=-rcs

# change these to set the proper directories where each files shoould be
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(INCDIR)/*.H)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm       = rm -rf

all:
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	@make tool

tool: $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LDFLAGS) -o $(TARGET).so
	@ar $(ARFLAGS) libtaskdebug_rt.a $(OBJECTS)
	@mv $(TARGET).so $(OBJDIR)
	@mv libtaskdebug_rt.a $(OBJDIR)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(OPT) $(CXXFLAGS) -c $< $(INCPATH) -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean

clean:
	@$(rm) $(OBJDIR)
	@$(rm) *~
	@echo "Cleanup complete!"
