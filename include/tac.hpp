#ifndef MINIC_TAC_HPP
#define MINIC_TAC_HPP

#include <iostream>
#include <string>
#include <vector>

class TACProgram {
public:
    std::string newTemp();
    std::string newLabel();
    void emit(const std::string& text);
    void emitLabel(const std::string& label);
    void print(std::ostream& out) const;
    const std::vector<std::string>& instructions() const;
    std::size_t mark() const;
    std::vector<std::string> takeFrom(std::size_t start);
    void append(const std::vector<std::string>& instructions);
    std::size_t eliminateDeadCode();

private:
    int tempCounter = 0;
    int labelCounter = 0;
    std::vector<std::string> code;
};

#endif
