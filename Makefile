CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
TARGET   = ll1parser
SRCDIR   = src
OBJDIR   = obj

SRCS = $(SRCDIR)/main.cpp \
       $(SRCDIR)/grammar.cpp \
       $(SRCDIR)/first_follow.cpp \
       $(SRCDIR)/stack.cpp \
       $(SRCDIR)/tree.cpp \
       $(SRCDIR)/error_handler.cpp \
       $(SRCDIR)/parser.cpp

OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Default target
all: dirs $(TARGET)

# Create obj and output directories
dirs:
	mkdir -p $(OBJDIR) output

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build successful: ./$(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run with all default test grammars
run: all
	./$(TARGET)

# Run with a specific grammar and input file:
#   make runfile GRAMMAR=input/grammar1.txt INPUT=input/grammar1_valid.txt
runfile: all
	./$(TARGET) $(GRAMMAR) $(INPUT)

# Clean compiled files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Clean output files too
cleanall: clean
	rm -f output/*.txt

# Check for memory leaks (requires valgrind)
valgrind: all
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

.PHONY: all dirs run runfile clean cleanall valgrind
