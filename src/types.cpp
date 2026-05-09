#include "types.hpp"

Type Type::scalar(BaseType base) {
    return Type{base, false, -1};
}

Type Type::array(BaseType base, int size) {
    return Type{base, true, size};
}

bool Type::operator==(const Type& other) const {
    return base == other.base && isArray == other.isArray && arraySize == other.arraySize;
}

bool Type::operator!=(const Type& other) const {
    return !(*this == other);
}

bool Type::isNumeric() const {
    return !isArray && (base == BaseType::Int || base == BaseType::Float || base == BaseType::Char);
}

bool Type::isScalar() const {
    return !isArray && base != BaseType::Error;
}

std::string Type::str() const {
    std::string name;
    switch (base) {
        case BaseType::Int: name = "int"; break;
        case BaseType::Float: name = "float"; break;
        case BaseType::Char: name = "char"; break;
        case BaseType::Bool: name = "bool"; break;
        case BaseType::Void: name = "void"; break;
        case BaseType::String: name = "string"; break;
        case BaseType::Error: name = "error"; break;
    }
    if (isArray) {
        name += "[";
        if (arraySize >= 0) name += std::to_string(arraySize);
        name += "]";
    }
    return name;
}

bool canAssign(const Type& from, const Type& to) {
    if (from.base == BaseType::Error || to.base == BaseType::Error) return true;
    if (to.isArray || from.isArray) return from == to;
    if (from == to) return true;
    if (from.isNumeric() && to.isNumeric()) return true;
    return false;
}

bool canUseAsCondition(const Type& type) {
    if (type.base == BaseType::Error) return true;
    return !type.isArray && (type.base == BaseType::Bool || type.isNumeric());
}

Type arithmeticResult(const Type& left, const Type& right) {
    if (!left.isNumeric() || !right.isNumeric()) return Type::scalar(BaseType::Error);
    if (left.base == BaseType::Float || right.base == BaseType::Float) {
        return Type::scalar(BaseType::Float);
    }
    return Type::scalar(BaseType::Int);
}

Type comparisonResult(const Type& left, const Type& right) {
    if (left.base == BaseType::Error || right.base == BaseType::Error) {
        return Type::scalar(BaseType::Error);
    }
    if ((left.isNumeric() && right.isNumeric()) || left == right) {
        return Type::scalar(BaseType::Bool);
    }
    return Type::scalar(BaseType::Error);
}
