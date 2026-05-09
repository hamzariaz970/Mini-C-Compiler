#ifndef MINIC_DIAGNOSTICS_HPP
#define MINIC_DIAGNOSTICS_HPP

#include <iostream>
#include <string>
#include <vector>

enum class DiagnosticPhase {
    Lexical,
    Syntax,
    Semantic
};

struct Diagnostic {
    DiagnosticPhase phase = DiagnosticPhase::Semantic;
    int line = 1;
    int column = 1;
    std::string message;
};

class Diagnostics {
public:
    void error(DiagnosticPhase phase, int line, int column, const std::string& message);
    void error(int line, int column, const std::string& message);
    bool hasErrors() const;
    bool hasErrors(DiagnosticPhase phase) const;
    int count() const;
    void print(std::ostream& out) const;
    void print(std::ostream& out, DiagnosticPhase phase) const;

private:
    std::vector<Diagnostic> errors;
};

std::string diagnosticPhaseName(DiagnosticPhase phase);

#endif
