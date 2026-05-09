#include "tac.hpp"

namespace {
bool startsWith(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

bool isReachabilityBoundary(const std::string& instruction) {
    return (!instruction.empty() && instruction.back() == ':') ||
           startsWith(instruction, "func ") ||
           startsWith(instruction, "endfunc ");
}

bool endsBasicBlock(const std::string& instruction) {
    return startsWith(instruction, "return") || startsWith(instruction, "goto ");
}
}

std::string TACProgram::newTemp() {
    return "t" + std::to_string(++tempCounter);
}

std::string TACProgram::newLabel() {
    return "L" + std::to_string(++labelCounter);
}

void TACProgram::emit(const std::string& text) {
    code.push_back(text);
}

void TACProgram::emitLabel(const std::string& label) {
    code.push_back(label + ":");
}

void TACProgram::print(std::ostream& out) const {
    for (const std::string& instruction : code) {
        out << instruction << '\n';
    }
}

const std::vector<std::string>& TACProgram::instructions() const {
    return code;
}

std::size_t TACProgram::mark() const {
    return code.size();
}

std::vector<std::string> TACProgram::takeFrom(std::size_t start) {
    if (start > code.size()) return {};
    std::vector<std::string> tail(code.begin() + static_cast<long>(start), code.end());
    code.erase(code.begin() + static_cast<long>(start), code.end());
    return tail;
}

void TACProgram::append(const std::vector<std::string>& instructions) {
    code.insert(code.end(), instructions.begin(), instructions.end());
}

std::size_t TACProgram::eliminateDeadCode() {
    std::vector<std::string> optimized;
    bool unreachable = false;
    std::size_t removedInBlock = 0;
    std::size_t totalRemoved = 0;

    for (const std::string& instruction : code) {
        if (unreachable && isReachabilityBoundary(instruction)) {
            if (removedInBlock > 0) {
                optimized.push_back("# dead code eliminated " +
                                    std::to_string(removedInBlock) +
                                    " unreachable instruction(s)");
                removedInBlock = 0;
            }
            unreachable = false;
        }

        if (unreachable) {
            ++removedInBlock;
            ++totalRemoved;
            continue;
        }

        optimized.push_back(instruction);
        if (endsBasicBlock(instruction)) {
            unreachable = true;
        }
    }

    if (removedInBlock > 0) {
        optimized.push_back("# dead code eliminated " +
                            std::to_string(removedInBlock) +
                            " unreachable instruction(s)");
    }

    code = std::move(optimized);
    return totalRemoved;
}
