#ifndef MINIC_SYMBOL_TABLE_HPP
#define MINIC_SYMBOL_TABLE_HPP

#include "diagnostics.hpp"
#include "types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

enum class SymbolKind {
    Variable,
    Parameter,
    Function
};

struct Symbol {
    std::string name;
    SymbolKind kind = SymbolKind::Variable;
    Type type;
    Type returnType;
    std::vector<Type> parameters;
    int scopeLevel = 0;
    bool defined = false;
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    int currentLevel() const;

    bool declare(const Symbol& symbol, int line, int column, Diagnostics& diagnostics);
    Symbol* resolve(const std::string& name);
    const Symbol* resolve(const std::string& name) const;
    std::vector<Symbol> snapshot() const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    std::vector<Symbol> declarations;
};

#endif
