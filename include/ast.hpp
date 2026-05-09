#ifndef MINIC_AST_HPP
#define MINIC_AST_HPP

#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;

struct ASTNode {
    std::string kind;
    std::string value;
    std::vector<ASTNodePtr> children;

    ASTNode(std::string nodeKind, std::string nodeValue = "");

    void addChild(const ASTNodePtr& child);
    void print(std::ostream& out, int indent = 0) const;
    void printJson(std::ostream& out, int indent = 0) const;
};

ASTNodePtr makeNode(const std::string& kind, const std::string& value = "");

#endif
