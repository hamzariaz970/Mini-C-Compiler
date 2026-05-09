CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Iinclude
TARGET := minic
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

.PHONY: all clean test unit-test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET) tests/valid_basic.c --tac --symbols
	./$(TARGET) tests/valid_functions.c --tac --symbols
	./$(TARGET) tests/valid_control_flow.c --tac --symbols
	./$(TARGET) tests/valid_arrays.c --tac --symbols
	./$(TARGET) tests/valid_dead_code.c --tac --symbols
	./$(TARGET) tests/valid_comments_literals.c --tac --symbols --ast-json
	./$(TARGET) tests/valid_scope_shadowing.c --tac --symbols
	./$(TARGET) tests/valid_void_function.c --tac --symbols
	! ./$(TARGET) tests/invalid_undeclared.c --tac --symbols
	! ./$(TARGET) tests/invalid_type_mismatch.c --tac --symbols
	! ./$(TARGET) tests/invalid_duplicate.c --tac --symbols
	! ./$(TARGET) tests/invalid_array_misuse.c --tac --symbols
	! ./$(TARGET) tests/invalid_array_size.c --tac --symbols
	! ./$(TARGET) tests/invalid_function_arg_count.c --tac --symbols
	! ./$(TARGET) tests/invalid_function_arg_type.c --tac --symbols
	! ./$(TARGET) tests/invalid_lexical.c --tokens
	! ./$(TARGET) tests/invalid_main_missing.c --tac --symbols
	! ./$(TARGET) tests/invalid_main_parameters.c --tac --symbols
	! ./$(TARGET) tests/invalid_missing_return.c --tac --symbols
	! ./$(TARGET) tests/invalid_syntax_missing_semicolon.c --tac --symbols
	! ./$(TARGET) tests/invalid_void_return_value.c --tac --symbols
	! ./$(TARGET) tests/invalid_void_variable.c --tac --symbols

unit-test: $(TARGET)
	./unit_tests/run_unit_tests.sh

clean:
	rm -f $(TARGET) src/*.o
