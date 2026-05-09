#ifndef MINIC_TYPES_HPP
#define MINIC_TYPES_HPP

#include <string>

enum class BaseType {
    Int,
    Float,
    Char,
    Bool,
    Void,
    String,
    Error
};

struct Type {
    BaseType base = BaseType::Error;
    bool isArray = false;
    int arraySize = -1;

    static Type scalar(BaseType base);
    static Type array(BaseType base, int size);

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;
    bool isNumeric() const;
    bool isScalar() const;
    std::string str() const;
};

bool canAssign(const Type& from, const Type& to);
bool canUseAsCondition(const Type& type);
Type arithmeticResult(const Type& left, const Type& right);
Type comparisonResult(const Type& left, const Type& right);

#endif
