CXX=g++
CXXFLAGS=-std=c++17 -O4
SRCDIR=./src
OBJDIR=./obj
BINDIR=./bin
MAINFILE=$(SRCDIR)/main.cpp
TESTMAINFILE=$(SRCDIR)/unittest.cpp
SRCFILES=$(SRCDIR)/OPERM5.cpp $(SRCDIR)/RNG.cpp $(SRCDIR)/Strategy.cpp $(SRCDIR)/Super_rng.cpp
OBJFILES=$(OBJDIR)/OPERM5.o $(OBJDIR)/RNG.o $(OBJDIR)/Strategy.o $(OBJDIR)/Super_rng.o
TARGET=$(BINDIR)/program
TEST=$(BINDIR)/test_program

.PHONY: all clean test

all: $(TARGET)

test: $(TEST)

$(TARGET): $(OBJFILES) $(MAINFILE)
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJFILES) $(MAINFILE) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	find . -name "*.gch" -type f -delete
	find . -name "*.o" -type f -delete
	find . -name "*.out" -type f -delete
	rm -rf $(OBJDIR) 
	rm -rf $(BINDIR)

$(TEST): $(OBJFILES) $(TESTMAINFILE)
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJFILES) $(TESTMAINFILE) -o $@
