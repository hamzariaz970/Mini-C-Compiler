# Mini C Compiler Project

This project is a **Mini C Compiler** built for a defined subset of the C
language. It includes:

- a C++ compiler pipeline
- an interactive Next.js frontend
- integration tests
- component/unit tests

The compiler supports:

- lexical analysis
- recursive-descent parsing
- semantic analysis
- symbol-table management
- AST generation
- Three Address Code generation
- constant folding
- dead code elimination
- phase-aware diagnostics with fail-stop behavior

## Quick Start

If you just cloned this project, run:

```bash
npm ci
make
make test
make unit-test
npm run dev
```

Then open:

```text
http://localhost:3000
```

## Prerequisites

Install these before running the project:

- `g++` with C++17 support
- `make`
- `Node.js >= 20.9.0`
- `npm`

Check your environment:

```bash
g++ --version
make --version
node --version
npm --version
```

If your Node version is below `20.9.0`, the frontend will not build correctly.

## Folder Structure

### `app/`

Next.js frontend and API route.

- `app/layout.tsx`
  Root layout for the web app.
- `app/page.tsx`
  Main compiler interface.
  Contains:
  - source editor
  - line-number gutter
  - compile button
  - optimization toggle
  - compiler stage tabs
  - AST viewer
  - diagnostics panel
  - compile-time comparison
- `app/globals.css`
  Global styling for the frontend.
- `app/api/compile/route.ts`
  API route used by the frontend to:
  - save source into a temporary file
  - run `./minic`
  - collect compiler output
  - parse output sections
  - compare optimized and unoptimized timings

### `include/`

Header files for compiler components.

- `ast.hpp`
  AST node definitions and interfaces.
- `diagnostics.hpp`
  Diagnostics model, phase classification, and print interfaces.
- `lexer.hpp`
  Lexer interface.
- `parser.hpp`
  Parser interface, parse results, and parser options.
- `symbol_table.hpp`
  Symbol and symbol-table declarations.
- `tac.hpp`
  TAC program declarations.
- `token.hpp`
  Token types and token-related definitions.
- `types.hpp`
  Type system declarations and compatibility helpers.

### `src/`

Compiler implementation files.

- `main.cpp`
  Compiler entry point. Reads source, runs compiler phases, prints requested
  output sections, and stops at the failing phase.
- `ast.cpp`
  AST print and JSON serialization logic.
- `diagnostics.cpp`
  Diagnostic storage and printing logic.
- `lexer.cpp`
  Lexical analyzer implementation.
- `parser.cpp`
  Recursive-descent parser, semantic checks, AST construction, and TAC emission.
- `symbol_table.cpp`
  Scope handling, declarations, lookup, and duplicate checking.
- `tac.cpp`
  TAC generation utilities, temporaries, labels, and dead-code elimination.
- `token.cpp`
  Token naming and token-related helpers.
- `types.cpp`
  Type compatibility, arithmetic result, condition rules, and assignment rules.

Generated files may also appear here after a build:

- `src/*.o`
  Object files created by `make`

These generated `.o` files are not source files and are rebuilt automatically.

### `tests/`

Integration-style Mini C programs used to validate whole-compiler behavior.

Valid programs:

- `valid_basic.c`
- `valid_functions.c`
- `valid_control_flow.c`
- `valid_arrays.c`
- `valid_dead_code.c`
- `valid_comments_literals.c`
- `valid_scope_shadowing.c`
- `valid_void_function.c`

Invalid programs:

- `invalid_lexical.c`
- `invalid_syntax_missing_semicolon.c`
- `invalid_undeclared.c`
- `invalid_type_mismatch.c`
- `invalid_duplicate.c`
- `invalid_array_misuse.c`
- `invalid_array_size.c`
- `invalid_function_arg_count.c`
- `invalid_function_arg_type.c`
- `invalid_main_missing.c`
- `invalid_main_parameters.c`
- `invalid_missing_return.c`
- `invalid_void_return_value.c`
- `invalid_void_variable.c`

### `unit_tests/`

Assertion-based component tests.

- `unit_tests/run_unit_tests.sh`
  Main unit test harness.
- `unit_tests/README.md`
  Short note on what the unit tests cover.
- `unit_tests/cases/`
  Focused success and failure cases for each compiler component:
  - lexer
  - parser
  - semantic analysis
  - symbol table
  - AST
  - TAC
  - optimization
  - diagnostics

### `docs/`

Project documentation and submission material.

- `CC_Project_Spring_2026.pdf`
  Project brief.
- `grammar.md`
  Mini C grammar summary.
- `Mini_C_Compiler_Project_Report.docx`
  Editable report.
- `QurratulainZafar_HamzaRiaz_MunibaNoor_Project_Presentation.pptx`
  Presentation slides.

### Root project files

- `Makefile`
  Builds the compiler and runs tests.
- `package.json`
  Frontend dependencies and npm scripts.
- `package-lock.json`
  Locked dependency versions.
- `next.config.mjs`
  Next.js config for local network access and compiler bundling.
- `tsconfig.json`
  TypeScript configuration.
- `next-env.d.ts`
  Next.js TypeScript environment file.
- `minic`
  Compiler executable generated after `make`.

Generated frontend files may also appear:

- `node_modules/`
  Installed npm dependencies
- `.next/`
  Next.js build artifacts

## Build From Scratch

### 1. Install frontend dependencies

```bash
npm ci
```

### 2. Build the compiler

```bash
make
```

This creates the executable:

```text
./minic
```

## How to Run the Compiler

Basic usage:

```bash
./minic <source-file>
```

Example:

```bash
./minic tests/valid_basic.c
```

By default, the compiler prints:

- tokens
- symbol table
- AST
- TAC

You can also request specific sections using flags.

## Compiler Flags

- `--tokens`
  Print lexical tokens.
- `--symbols`
  Print symbol table.
- `--ast`
  Print the readable AST.
- `--ast-json`
  Print AST as JSON.
- `--tac`
  Print Three Address Code.
- `--no-opt`
  Disable optimizations.

Example:

```bash
./minic tests/valid_functions.c --tokens --symbols --ast --ast-json --tac
```

## Compiler Components and How to Test Them

The `.cpp` files in `src/` are not meant to be run directly one by one. They are
all compiled into `./minic`. To test a component, run `./minic` with the right
input file and output flags.

### 1. Lexer

Files:

- `include/lexer.hpp`
- `src/lexer.cpp`
- `include/token.hpp`
- `src/token.cpp`

What it does:

- scans characters
- recognizes keywords, identifiers, literals, punctuation, and operators
- tracks line/column positions
- reports lexical errors

Run lexer output:

```bash
./minic tests/valid_basic.c --tokens
```

Component-level checks:

```bash
./minic unit_tests/cases/lexer_success.c --tokens
./minic unit_tests/cases/lexer_failure.c --tokens --symbols --ast --tac
```

Expected failure behavior:

- prints a lexical diagnostic
- prints `Stopped at: Lexical Analysis`
- does not print downstream sections such as symbol table, AST, or TAC

### 2. Parser / Syntax Analyzer

Files:

- `include/parser.hpp`
- `src/parser.cpp`

What it does:

- performs recursive-descent parsing
- checks grammar structure
- builds AST
- drives semantic analysis and TAC emission

Run parser-focused output:

```bash
./minic unit_tests/cases/parser_success.c --tokens --ast
./minic unit_tests/cases/parser_failure.c --tokens --symbols --ast --tac
```

Expected syntax-failure behavior:

- prints a syntax diagnostic
- prints `Stopped at: Syntax Analysis`
- does not print symbol table, AST, or TAC after the syntax failure

### 3. Semantic Analysis

Files:

- `include/types.hpp`
- `src/types.cpp`
- `include/symbol_table.hpp`
- `src/symbol_table.cpp`
- semantic logic inside `src/parser.cpp`

What it checks:

- undeclared identifiers
- duplicate declarations
- assignment compatibility
- condition expression validity
- return type validity
- function argument count and type
- `main` restrictions
- array misuse
- invalid void declarations

Run semantic-analysis checks:

```bash
./minic unit_tests/cases/semantic_success.c --symbols --tac
./minic unit_tests/cases/semantic_failure.c --tokens --symbols --ast --tac
```

Expected semantic-failure behavior:

- prints a semantic diagnostic
- prints `Stopped at: Semantic Analysis`
- may still print symbol table and AST
- should stop before printing TAC

### 4. Symbol Table

Files:

- `include/symbol_table.hpp`
- `src/symbol_table.cpp`

Run symbol-table output:

```bash
./minic tests/valid_functions.c --symbols
./minic unit_tests/cases/symbol_table_success.c --symbols
```

Check stop-before-symbol-table behavior:

```bash
./minic unit_tests/cases/symbol_table_failure.c --symbols --ast --tac
```

### 5. AST

Files:

- `include/ast.hpp`
- `src/ast.cpp`

Run AST output:

```bash
./minic tests/valid_functions.c --ast --ast-json
./minic unit_tests/cases/ast_success.c --ast --ast-json
```

Check AST suppression after earlier failure:

```bash
./minic unit_tests/cases/ast_failure.c --tokens --ast --ast-json --symbols --tac
```

### 6. TAC / Intermediate Representation

Files:

- `include/tac.hpp`
- `src/tac.cpp`

Run TAC output:

```bash
./minic tests/valid_basic.c --tac
./minic unit_tests/cases/tac_success.c --tac
```

Check TAC suppression after semantic failure:

```bash
./minic unit_tests/cases/tac_failure.c --tokens --symbols --ast --tac
```

### 7. Optimization

Files involved:

- optimization logic in `src/parser.cpp`
- dead-code elimination in `src/tac.cpp`

What it currently supports:

- constant folding
- dead code elimination

Run optimized TAC:

```bash
./minic unit_tests/cases/optimization_success.c --tac
```

Run unoptimized TAC:

```bash
./minic unit_tests/cases/optimization_success.c --tac --no-opt
```

### 8. Diagnostics

Files:

- `include/diagnostics.hpp`
- `src/diagnostics.cpp`

What it does:

- stores diagnostics by compiler phase
- prints line/column-aware messages
- identifies whether failure happened in:
  - lexical analysis
  - syntax analysis
  - semantic analysis

Run diagnostics-focused tests:

```bash
./minic unit_tests/cases/diagnostics_success.c --tokens --symbols --ast --tac
./minic unit_tests/cases/diagnostics_failure.c --tokens --symbols --ast --tac
```

## How to Run the Integration Test Suite

Run all whole-project tests:

```bash
make test
```

This runs all files in `tests/` and checks that:

- valid files are accepted
- invalid files are rejected

## How to Run the Unit / Component Test Suite

Run:

```bash
make unit-test
```

This executes:

```bash
./unit_tests/run_unit_tests.sh
```

It checks:

- every major compiler component has at least one success case
- every tested component has at least one failure case where applicable
- the compiler stops at the correct phase
- later sections are not printed after earlier failure

## Frontend Instructions

The frontend gives you a browser-based interface for the compiler.

### Start the frontend

```bash
npm run dev
```

Open:

```text
http://localhost:3000
```

The dev script automatically runs:

```bash
make
```

before starting Next.js.

### Production build

Build the frontend:

```bash
npm run build
```

Start it:

```bash
npm start
```

The build step automatically runs:

```bash
make clean && make
```

first.

### Frontend Features

- source code editor with line numbers
- compile button
- optimization toggle
- stage-by-stage output tabs
- AST tree viewer
- phase-aware diagnostics
- fail-stop stage disabling
- timing comparison between optimized and unoptimized runs

## `package.json` Scripts

- `npm run dev`
  Start Next.js development server.
- `npm run build`
  Build production frontend bundle.
- `npm start`
  Start built frontend in production mode.
- `npm run test:compiler`
  Alias for `make test`.

## Common Commands

Build compiler:

```bash
make
```

Clean compiler artifacts:

```bash
make clean
```

Run integration tests:

```bash
make test
```

Run component tests:

```bash
make unit-test
```

Run the frontend:

```bash
npm run dev
```

Run one specific compiler file:

```bash
./minic tests/valid_functions.c --tokens --symbols --ast --ast-json --tac
```

## Troubleshooting

### `node --version` is too old

Use `Node.js >= 20.9.0`, then run:

```bash
npm ci
```

again.

### `make` fails

Make sure:

- `g++` is installed
- `make` is installed
- you are in the `CC_Project_code/` folder

### Frontend does not start

Make sure:

- `npm ci` completed successfully
- `make` succeeds
- `./minic` exists

### Port `3000` is busy

Stop the process using port `3000`, then rerun:

```bash
npm run dev
```

### The compiler binary is missing during frontend use

Run:

```bash
make
```

and then restart the frontend.

## Contributors

This project was made as part of the **Compiler Construction (CS-351)** Course.

Developers:

- **Hamza Riaz (414577)** — `hriaz.bscs22seecs@seecs.edu.pk`
- **Qurratulain Zafar (412655)** — `qzafar.bscs22seecs@seecs.edu.pk`
- **Muniba Noor (407670)** — `mnoor.bscs22seecs@seecs.edu.pk`
