# LL(1) Parser — CS4031 Compiler Construction Assignment 02

## Team Members
| Name | Roll Number |
|------|-------------|
| Student 1 | 23i-0563 |
| Student 2 | *(partner roll number)* |

**Section:** *(your section)*  
**Language:** C++17  
**Semester:** Spring 2026

---

## Project Structure

```
23i_0563-RollNumber2-Section/
├── src/
│   ├── main.cpp            # Entry point, orchestrates all grammars
│   ├── grammar.h / .cpp    # CFG loading, left factoring, left recursion removal
│   ├── first_follow.h/.cpp # FIRST and FOLLOW set computation
│   ├── parser.h / .cpp     # LL(1) table construction, stack-based parsing, error recovery
│   ├── stack.h  / .cpp     # Stack ADT (push/pop/top/isEmpty)
│   ├── tree.h   / .cpp     # Parse tree node structure + ASCII printer
│   └── error_handler.h/.cpp# Error types, reporting, and formatting
├── input/
│   ├── grammar1.txt        # Simple grammar (epsilon productions)
│   ├── grammar1_valid.txt  # Valid strings for grammar 1
│   ├── grammar1_errors.txt # Invalid strings for grammar 1
│   ├── grammar2.txt        # Expression grammar (left recursion)
│   ├── grammar2_valid.txt
│   ├── grammar2_errors.txt
│   ├── grammar3.txt        # Statement grammar (left factoring + dangling else)
│   ├── grammar3_valid.txt
│   ├── grammar3_errors.txt
│   ├── grammar4.txt        # Program grammar (left recursion in Expr)
│   ├── grammar4_valid.txt
│   └── grammar4_errors.txt
├── output/                 # Auto-generated when you run the parser
│   ├── grammarN_valid_grammar_transformed.txt
│   ├── grammarN_valid_first_follow_sets.txt
│   ├── grammarN_valid_parsing_table.txt
│   ├── grammarN_valid_parsing_trace.txt
│   ├── grammarN_valid_parse_trees.txt
│   └── ... (same for _errors)
├── docs/
│   └── report.pdf
├── Makefile
└── README.md
```

---

## Compilation

**Requirements:** `g++` with C++17 support (GCC 7+ or Clang 5+)

```bash
# Build
make

# Clean build artifacts
make clean

# Clean everything including outputs
make cleanall
```

---

## Running

### Run all 4 test grammars (default)
```bash
make run
# or
./ll1parser
```

### Run with a specific grammar and input file
```bash
make runfile GRAMMAR=input/grammar2.txt INPUT=input/grammar2_valid.txt
# or
./ll1parser input/grammar2.txt input/grammar2_valid.txt
```

### Memory leak check (requires valgrind)
```bash
make valgrind
```

---

## Grammar File Format

- One production per line
- Arrow symbol: `->`
- Alternatives separated by `|`
- **Non-terminals:** Multi-character names starting with uppercase (e.g., `Expr`, `Term`)
- **Terminals:** lowercase keywords, operators, single characters
- **Epsilon:** use `epsilon`

### Example (`grammar.txt`)
```
Expr -> Expr + Term | Term
Term -> Term * Factor | Factor
Factor -> ( Expr ) | id
```

---

## Input String File Format

- One string per line
- Tokens separated by spaces
- Only terminals from the grammar

### Example (`input.txt`)
```
id + id * id
( id + id ) * id
id + * id
```

---

## Grammar Descriptions

| Grammar | File | Description | Transformations Applied |
|---------|------|-------------|------------------------|
| 1 | grammar1.txt | Simple grammar with epsilon | None needed |
| 2 | grammar2.txt | Classic expression grammar | Left recursion removal |
| 3 | grammar3.txt | If-then-else (dangling else) | Left factoring (NOT LL(1) — conflict demonstrated) |
| 4 | grammar4.txt | Program with expressions | Left recursion removal (Expr) |

---

## Features Implemented

| Feature | Status |
|---------|--------|
| CFG file loading | ✅ |
| Left factoring | ✅ |
| Direct left recursion removal | ✅ |
| Indirect left recursion removal | ✅ |
| FIRST set computation (with epsilon) | ✅ (fixed: all-NT-epsilon case) |
| FOLLOW set computation | ✅ |
| LL(1) parsing table construction | ✅ |
| LL(1) conflict detection & reporting | ✅ |
| Stack-based LL(1) parsing | ✅ |
| Parse tree generation (ASCII) | ✅ |
| Error detection (4 types) | ✅ |
| Panic-mode error recovery | ✅ |
| Multiple error reporting (continues after error) | ✅ |
| Line/column number in error messages | ✅ |
| Output files (transformed grammar, sets, table, trace, trees) | ✅ |

---

## Error Recovery Strategy

**Panic Mode Recovery** is implemented:
1. On an empty table entry `M[X, a]`, an error is reported with line/column.
2. The synchronising set is computed as `FOLLOW(X) ∪ {$}`.
3. Stack symbols are popped until the top's FOLLOW set contains the current token.
4. Input tokens are skipped until a synchronising symbol is found.
5. If no progress is made (to avoid infinite loops), the top symbol is force-popped.
6. Parsing continues to find additional errors.

---

## Known Limitations

- Grammar 3 (if-then-else) is inherently ambiguous (dangling else), so it is **not LL(1)**. The parser correctly detects and reports this conflict. Parsing still proceeds using the first-production rule.
- Very deeply nested grammars may generate large output files.
- The `'` (prime) character is used for new non-terminals introduced during left recursion removal.

---

## Sample Output

```
╔══════════════════════════════════════════╗
║  AFTER LEFT RECURSION REMOVAL             ║
╚══════════════════════════════════════════╝
  Expr -> Term Expr'
  Expr' -> + Term Expr' | epsilon
  Term -> Factor Term'
  Term' -> * Factor Term' | epsilon
  Factor -> ( Expr ) | id
```

Parse trace excerpt:
```
Step Stack (bottom->top)           Input Remaining        Action
---- ------------------------------ ---------------------- -------------------------
1    $ Expr                         id + id * id $         Predict: Expr -> Term Expr'
2    $ Expr' Term                   id + id * id $         Predict: Term -> Factor Term'
...
17   $                              $                      Accept
```
