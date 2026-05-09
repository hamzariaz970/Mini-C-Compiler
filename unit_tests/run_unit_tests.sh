#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COMPILER="$ROOT_DIR/minic"
CASE_DIR="$ROOT_DIR/unit_tests/cases"

if [[ ! -x "$COMPILER" ]]; then
  echo "Compiler binary not found at $COMPILER"
  echo "Run 'make' first."
  exit 1
fi

current_name=""
current_output=""
current_status=0
passed=0
failed=0

run_case() {
  current_name="$1"
  shift
  local source_file="$1"
  shift

  if current_output=$("$COMPILER" "$source_file" "$@" 2>&1); then
    current_status=0
  else
    current_status=$?
  fi
}

fail_assert() {
  local message="$1"
  echo "[FAIL] $current_name"
  echo "Reason: $message"
  echo "--- compiler output ---"
  printf '%s\n' "$current_output"
  echo "-----------------------"
  return 1
}

assert_status() {
  local expected="$1"
  [[ "$current_status" -eq "$expected" ]] || fail_assert "expected exit $expected, got $current_status"
}

assert_contains() {
  local needle="$1"
  [[ "$current_output" == *"$needle"* ]] || fail_assert "missing expected text: $needle"
}

assert_not_contains() {
  local needle="$1"
  [[ "$current_output" != *"$needle"* ]] || fail_assert "unexpected text present: $needle"
}

run_test() {
  local name="$1"
  if "$name"; then
    echo "[PASS] $current_name"
    passed=$((passed + 1))
  else
    failed=$((failed + 1))
  fi
}

test_lexer_success() {
  run_case "lexer_success" "$CASE_DIR/lexer_success.c" --tokens
  assert_status 0
  assert_contains "TOKENS"
  assert_contains "identifier"
  assert_contains "Result: accepted"
  assert_not_contains "DIAGNOSTICS"
}

test_lexer_failure() {
  run_case "lexer_failure" "$CASE_DIR/lexer_failure.c" --tokens --symbols --ast --tac
  assert_status 2
  assert_contains "Stopped at: Lexical Analysis"
  assert_contains "Lexical Analysis - Line"
  assert_contains "unexpected character '@'"
  assert_not_contains "SYMBOL TABLE"
  assert_not_contains "ABSTRACT SYNTAX TREE"
  assert_not_contains "THREE ADDRESS CODE"
}

test_parser_success() {
  run_case "parser_success" "$CASE_DIR/parser_success.c" --tokens --ast
  assert_status 0
  assert_contains "TOKENS"
  assert_contains "ABSTRACT SYNTAX TREE"
  assert_contains "IfStatement"
  assert_contains "Result: accepted"
}

test_parser_failure() {
  run_case "parser_failure" "$CASE_DIR/parser_failure.c" --tokens --symbols --ast --tac
  assert_status 2
  assert_contains "Stopped at: Syntax Analysis"
  assert_contains "Syntax Analysis - Line"
  assert_contains "expected ';' after declaration"
  assert_not_contains "SYMBOL TABLE"
  assert_not_contains "ABSTRACT SYNTAX TREE"
  assert_not_contains "THREE ADDRESS CODE"
}

test_semantic_success() {
  run_case "semantic_success" "$CASE_DIR/semantic_success.c" --symbols --tac
  assert_status 0
  assert_contains "SYMBOL TABLE"
  assert_contains "double_value"
  assert_contains "THREE ADDRESS CODE"
  assert_contains "call double_value, 1"
  assert_contains "Result: accepted"
}

test_semantic_failure() {
  run_case "semantic_failure" "$CASE_DIR/semantic_failure.c" --tokens --symbols --ast --tac
  assert_status 2
  assert_contains "Stopped at: Semantic Analysis"
  assert_contains "Semantic Analysis - Line"
  assert_contains "use of undeclared identifier 'y'"
  assert_contains "SYMBOL TABLE"
  assert_contains "ABSTRACT SYNTAX TREE"
  assert_not_contains "THREE ADDRESS CODE"
}

test_symbol_table_success() {
  run_case "symbol_table_success" "$CASE_DIR/symbol_table_success.c" --symbols
  assert_status 0
  assert_contains "SYMBOL TABLE"
  assert_contains "helper"
  assert_contains "global_count"
  assert_contains "main"
  assert_contains "local"
}

test_symbol_table_failure() {
  run_case "symbol_table_failure" "$CASE_DIR/symbol_table_failure.c" --symbols --ast --tac
  assert_status 2
  assert_contains "Stopped at: Syntax Analysis"
  assert_not_contains "SYMBOL TABLE"
}

test_ast_success() {
  run_case "ast_success" "$CASE_DIR/ast_success.c" --ast --ast-json
  assert_status 0
  assert_contains "ABSTRACT SYNTAX TREE"
  assert_contains "AST JSON"
  assert_contains "\"kind\": \"Program\""
  assert_contains "FunctionDeclaration"
}

test_ast_failure() {
  run_case "ast_failure" "$CASE_DIR/ast_failure.c" --tokens --ast --ast-json --symbols --tac
  assert_status 2
  assert_contains "Stopped at: Syntax Analysis"
  assert_not_contains "ABSTRACT SYNTAX TREE"
  assert_not_contains "AST JSON"
}

test_tac_success() {
  run_case "tac_success" "$CASE_DIR/tac_success.c" --tac
  assert_status 0
  assert_contains "THREE ADDRESS CODE"
  assert_contains "func main"
  assert_contains "return x"
}

test_tac_failure() {
  run_case "tac_failure" "$CASE_DIR/tac_failure.c" --tokens --symbols --ast --tac
  assert_status 2
  assert_contains "Stopped at: Semantic Analysis"
  assert_not_contains "THREE ADDRESS CODE"
}

test_optimization_success() {
  run_case "optimization_success" "$CASE_DIR/optimization_success.c" --tac
  assert_status 0
  assert_contains "# constant folded"
  assert_contains "# dead code eliminated"
}

test_optimization_no_opt_mode() {
  run_case "optimization_no_opt_mode" "$CASE_DIR/optimization_success.c" --tac --no-opt
  assert_status 0
  assert_not_contains "# constant folded"
  assert_not_contains "# dead code eliminated"
  assert_contains "t3 = x + 99"
}

test_diagnostics_success() {
  run_case "diagnostics_success" "$CASE_DIR/diagnostics_success.c" --tokens --symbols --ast --tac
  assert_status 0
  assert_not_contains "DIAGNOSTICS"
  assert_contains "Result: accepted"
}

test_diagnostics_failure() {
  run_case "diagnostics_failure" "$CASE_DIR/diagnostics_failure.c" --tokens --symbols --ast --tac
  assert_status 2
  assert_contains "DIAGNOSTICS"
  assert_contains "Semantic Analysis - Line"
  assert_contains "cannot assign string to int"
  assert_contains "Stopped at: Semantic Analysis"
}

main() {
  run_test test_lexer_success
  run_test test_lexer_failure
  run_test test_parser_success
  run_test test_parser_failure
  run_test test_semantic_success
  run_test test_semantic_failure
  run_test test_symbol_table_success
  run_test test_symbol_table_failure
  run_test test_ast_success
  run_test test_ast_failure
  run_test test_tac_success
  run_test test_tac_failure
  run_test test_optimization_success
  run_test test_optimization_no_opt_mode
  run_test test_diagnostics_success
  run_test test_diagnostics_failure

  echo
  echo "Unit tests passed: $passed"
  echo "Unit tests failed: $failed"

  if [[ "$failed" -ne 0 ]]; then
    exit 1
  fi
}

main "$@"
