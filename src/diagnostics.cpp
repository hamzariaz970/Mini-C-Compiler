#include "diagnostics.hpp"

std::string diagnosticPhaseName(DiagnosticPhase phase) {
    switch (phase) {
        case DiagnosticPhase::Lexical: return "Lexical Analysis";
        case DiagnosticPhase::Syntax: return "Syntax Analysis";
        case DiagnosticPhase::Semantic: return "Semantic Analysis";
    }
    return "Compiler";
}

void Diagnostics::error(DiagnosticPhase phase, int line, int column, const std::string& message) {
    errors.push_back({phase, line, column, message});
}

void Diagnostics::error(int line, int column, const std::string& message) {
    error(DiagnosticPhase::Semantic, line, column, message);
}

bool Diagnostics::hasErrors() const {
    return !errors.empty();
}

bool Diagnostics::hasErrors(DiagnosticPhase phase) const {
    for (const Diagnostic& diagnostic : errors) {
        if (diagnostic.phase == phase) return true;
    }
    return false;
}

int Diagnostics::count() const {
    return static_cast<int>(errors.size());
}

void Diagnostics::print(std::ostream& out) const {
    for (const Diagnostic& diagnostic : errors) {
        out << diagnosticPhaseName(diagnostic.phase) << " - Line "
            << diagnostic.line << ", column " << diagnostic.column
            << ": " << diagnostic.message << '\n';
    }
}

void Diagnostics::print(std::ostream& out, DiagnosticPhase phase) const {
    for (const Diagnostic& diagnostic : errors) {
        if (diagnostic.phase != phase) continue;
        out << diagnosticPhaseName(diagnostic.phase) << " - Line "
            << diagnostic.line << ", column " << diagnostic.column
            << ": " << diagnostic.message << '\n';
    }
}
