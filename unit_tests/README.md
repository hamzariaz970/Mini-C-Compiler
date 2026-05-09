# Unit Tests

This folder contains assertion-based component tests for the Mini C compiler.
The goal is to verify both successful behavior and proper fail-stop behavior
for the main compiler components:

- lexer
- parser
- semantic analysis
- symbol table output
- AST output
- TAC output
- optimization
- diagnostics

Run the unit test harness with:

```bash
make unit-test
```

The harness checks that each component has:

- at least one success case with the expected output
- at least one failure case
- a correct `Stopped at: ...` phase message when the pipeline rejects input
- downstream sections suppressed when an earlier phase fails
