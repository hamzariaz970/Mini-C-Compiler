#include "symbol_table.hpp"

SymbolTable::SymbolTable() {
    enterScope();
}

void SymbolTable::enterScope() {
    scopes.emplace_back();
}

void SymbolTable::exitScope() {
    if (scopes.size() > 1) scopes.pop_back();
}

int SymbolTable::currentLevel() const {
    return static_cast<int>(scopes.size()) - 1;
}

bool SymbolTable::declare(const Symbol& symbol, int line, int column, Diagnostics& diagnostics) {
    auto& current = scopes.back();
    if (current.count(symbol.name)) {
        diagnostics.error(DiagnosticPhase::Semantic, line, column, "duplicate declaration of '" + symbol.name + "'");
        return false;
    }

    Symbol stored = symbol;
    stored.scopeLevel = currentLevel();
    current[stored.name] = stored;
    declarations.push_back(stored);
    return true;
}

Symbol* SymbolTable::resolve(const std::string& name) {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) return &found->second;
    }
    return nullptr;
}

const Symbol* SymbolTable::resolve(const std::string& name) const {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) return &found->second;
    }
    return nullptr;
}

std::vector<Symbol> SymbolTable::snapshot() const {
    return declarations;
}
