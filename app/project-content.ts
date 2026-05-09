export const supportedCapabilities = [
  "Lexical analysis for keywords, identifiers, literals, punctuation, and operators",
  "Recursive-descent parsing for a practical Mini C subset",
  "Semantic analysis for undeclared identifiers, duplicates, type mismatches, return validity, arrays, and call arguments",
  "Symbol-table management with scoped declarations and shadowing support",
  "AST generation in both readable tree form and JSON form",
  "Three Address Code generation for declarations, assignments, expressions, function calls, conditions, loops, and returns",
  "Control-flow support for if/else, while, and for statements",
  "Function definitions, parameters, returns, and validation of main",
  "Primitive types: int, float, char, bool, and void",
  "Fixed-size single-dimensional arrays",
  "Phase-aware diagnostics with line/column reporting and fail-stop behavior"
] as const;

export const innovationCapabilities = [
  "Constant folding optimization for compile-time expression simplification",
  "Dead code elimination after unconditional return and goto paths",
  "Frontend stage explorer with tokens, parse/AST view, symbols, IR, final code, and diagnostics tabs",
  "Timing comparison between optimized and unoptimized compiler runs"
] as const;

export const unsupportedCapabilities = [
  "Pointers, address-of, dereference, and dynamic memory features",
  "Structs, unions, enums, typedefs, and user-defined aggregate types",
  "switch/case/default, do-while, break, and continue statements",
  "Separate function prototypes without full function bodies",
  "Multi-dimensional or dynamically sized arrays",
  "Full preprocessor and standard library semantics beyond skipped directive lines",
  "Object-oriented C++ features or the complete ANSI C language"
] as const;

export const contributors = [
  {
    name: "Hamza Riaz (414577)",
    email: "hriaz.bscs22seecs@seecs.edu.pk",
    linkedin: "https://www.linkedin.com/in/hamzariaz970/"
  },
  {
    name: "Qurratulain Zafar (412655)",
    email: "qzafar.bscs22seecs@seecs.edu.pk",
    linkedin: "https://www.linkedin.com/in/qurratulain-zafar-549364307/"
  },
  {
    name: "Muniba Noor (407670)",
    email: "mnoor.bscs22seecs@seecs.edu.pk",
    linkedin: "https://www.linkedin.com/in/muniba-noor-6061b830b/"
  }
] as const;
