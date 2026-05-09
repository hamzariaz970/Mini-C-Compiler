#include "diagnostics.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::string readFile(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("could not open input file: " + path);
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void printUsage(const char* program) {
    std::cout << "Usage: " << program
              << " <source.c> [--tokens] [--symbols] [--ast] [--ast-json] [--tac] [--no-opt]\n";
}

void printTokens(const std::vector<Token>& tokens) {
    std::cout << "\nTOKENS\n";
    std::cout << std::left << std::setw(8) << "Line" << std::setw(8) << "Col"
              << std::setw(18) << "Type" << "Lexeme\n";
    for (const Token& token : tokens) {
        if (token.type == TokenType::End) continue;
        std::cout << std::left << std::setw(8) << token.line << std::setw(8) << token.column
                  << std::setw(18) << tokenTypeName(token.type) << token.lexeme << '\n';
    }
}

std::string kindName(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::Variable: return "variable";
        case SymbolKind::Parameter: return "parameter";
        case SymbolKind::Function: return "function";
    }
    return "unknown";
}

void printSymbols(const SymbolTable& symbols) {
    std::cout << "\nSYMBOL TABLE\n";
    std::cout << std::left << std::setw(14) << "Name" << std::setw(12) << "Kind"
              << std::setw(14) << "Type" << std::setw(8) << "Scope" << "Details\n";
    for (const Symbol& symbol : symbols.snapshot()) {
        std::cout << std::left << std::setw(14) << symbol.name << std::setw(12)
                  << kindName(symbol.kind);
        if (symbol.kind == SymbolKind::Function) {
            std::cout << std::setw(14) << symbol.returnType.str() << std::setw(8)
                      << symbol.scopeLevel << "params=" << symbol.parameters.size();
        } else {
            std::cout << std::setw(14) << symbol.type.str() << std::setw(8)
                      << symbol.scopeLevel;
        }
        std::cout << '\n';
    }
}

void printDiagnosticsAndStop(const Diagnostics& diagnostics, DiagnosticPhase phase) {
    std::cout << "\nDIAGNOSTICS\n";
    diagnostics.print(std::cout, phase);
    std::cout << "\nStopped at: " << diagnosticPhaseName(phase) << '\n';
    std::cout << "\nResult: rejected\n";
}
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    bool showTokens = false;
    bool showSymbols = false;
    bool showAst = false;
    bool showAstJson = false;
    bool showTac = false;
    ParserOptions parserOptions;
    for (int i = 2; i < argc; ++i) {
        std::string option = argv[i];
        if (option == "--tokens") showTokens = true;
        else if (option == "--symbols") showSymbols = true;
        else if (option == "--ast") showAst = true;
        else if (option == "--ast-json") showAstJson = true;
        else if (option == "--tac") showTac = true;
        else if (option == "--no-opt") parserOptions.optimize = false;
        else {
            std::cerr << "Unknown option: " << option << '\n';
            printUsage(argv[0]);
            return 1;
        }
    }
    if (!showTokens && !showSymbols && !showAst && !showAstJson && !showTac) {
        showTokens = true;
        showSymbols = true;
        showAst = true;
        showTac = true;
    }

    try {
        Diagnostics diagnostics;
        std::string source = readFile(argv[1]);
        Lexer lexer(source, diagnostics);
        std::vector<Token> tokens = lexer.scanTokens();

        if (showTokens) printTokens(tokens);
        if (diagnostics.hasErrors(DiagnosticPhase::Lexical)) {
            printDiagnosticsAndStop(diagnostics, DiagnosticPhase::Lexical);
            return 2;
        }

        Parser parser(tokens, diagnostics, parserOptions);
        bool ok = parser.parseProgram();

        if (diagnostics.hasErrors(DiagnosticPhase::Syntax)) {
            printDiagnosticsAndStop(diagnostics, DiagnosticPhase::Syntax);
            return 2;
        }

        if (showSymbols) printSymbols(parser.symbols());
        if (showAst) {
            std::cout << "\nABSTRACT SYNTAX TREE\n";
            parser.ast()->print(std::cout);
        }
        if (showAstJson) {
            std::cout << "\nAST JSON\n";
            parser.ast()->printJson(std::cout);
            std::cout << '\n';
        }

        if (diagnostics.hasErrors(DiagnosticPhase::Semantic)) {
            printDiagnosticsAndStop(diagnostics, DiagnosticPhase::Semantic);
            return 2;
        }

        if (showTac) {
            std::cout << "\nTHREE ADDRESS CODE\n";
            parser.tac().print(std::cout);
        }

        if (diagnostics.hasErrors()) {
            std::cout << "\nDIAGNOSTICS\n";
            diagnostics.print(std::cout);
        }

        std::cout << "\nResult: " << (ok ? "accepted" : "rejected") << '\n';
        return ok ? 0 : 2;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}
