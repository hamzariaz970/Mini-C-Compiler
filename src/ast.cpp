#include "ast.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace {
std::string escapeJson(const std::string& value) {
    std::ostringstream out;
    for (char ch : value) {
        switch (ch) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(ch));
                } else {
                    out << ch;
                }
                break;
        }
    }
    return out.str();
}

void indentJson(std::ostream& out, int indent) {
    for (int i = 0; i < indent; ++i) out << "  ";
}
}

ASTNode::ASTNode(std::string nodeKind, std::string nodeValue)
    : kind(std::move(nodeKind)), value(std::move(nodeValue)) {}

void ASTNode::addChild(const ASTNodePtr& child) {
    if (child) children.push_back(child);
}

void ASTNode::print(std::ostream& out, int indent) const {
    for (int i = 0; i < indent; ++i) out << "  ";
    out << kind;
    if (!value.empty()) out << ": " << value;
    out << '\n';
    for (const ASTNodePtr& child : children) {
        child->print(out, indent + 1);
    }
}

void ASTNode::printJson(std::ostream& out, int indent) const {
    indentJson(out, indent);
    out << "{\n";
    indentJson(out, indent + 1);
    out << "\"kind\": \"" << escapeJson(kind) << "\",\n";
    indentJson(out, indent + 1);
    out << "\"value\": \"" << escapeJson(value) << "\",\n";
    indentJson(out, indent + 1);
    out << "\"children\": [";
    if (!children.empty()) out << '\n';
    for (std::size_t i = 0; i < children.size(); ++i) {
        children[i]->printJson(out, indent + 2);
        if (i + 1 < children.size()) out << ',';
        out << '\n';
    }
    if (!children.empty()) indentJson(out, indent + 1);
    out << "]\n";
    indentJson(out, indent);
    out << "}";
}

ASTNodePtr makeNode(const std::string& kind, const std::string& value) {
    return std::make_shared<ASTNode>(kind, value);
}
