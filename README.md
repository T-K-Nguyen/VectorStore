VectorStore
============

## Overview
Custom data structure implementations (`ArrayList`, `SinglyLinkedList`, `VectorStore`) for the assignment. This repository now includes an automated test harness for `ArrayList` to enable rapid, extensible testing without manually writing hundreds of `assert` statements in `main.cpp`.

## Automated ArrayList Unit Tests (Single-Line TEST Mode)

Primary harness: `tests/test_runner.cpp`
Unit test suite: `tests/arraylist_unit_tests.txt`

### Build
```
g++ -std=c++17 -O2 -I . tests/test_runner.cpp VectorStore.cpp -o test_runner.exe
```

### Run (Windows PowerShell / cmd)
```
test_runner.exe tests\arraylist_unit_tests.txt unit_results.txt
```
Add `--verbose` to display per-test PASS lines.

### TEST Line Syntax
Each line is an isolated scenario:
```
TEST <id> <op1> ; <op2> ; ... ; <assertN>
```
Internal state is reset every line. Assertions count individually toward totals.

#### Core Operations
```
CAP <cap>              # set initial capacity of primary list
ADD <v>
ADD_AT <index> <v>
SET <index> <v>
GET <index> = <expected>
REMOVE_AT <index> = <expectedValue>
SIZE = <expectedSize>
EMPTY = true|false
INDEX_OF <v> = <expectedIndex>
CONTAINS <v> = true|false
TO_STRING = [literal, list, form]
CLEAR
SELF_ASSIGN           # arr = arr
```

#### Dual-List (Copy / Assignment) Operations
```
CAP2 <cap>             # create/replace secondary list (arr2)
COPY_NEW               # arr2 = copy-ctor(arr)
ASSIGN_TO2             # *arr2 = arr
ASSIGN_FROM2           # arr = *arr2
SIZE2 = <expected>
GET2 <index> = <expected>
SET2 <index> <value>
REMOVE_AT2 <index> = <expectedValue>
TO_STRING2 = [literal]
CLEAR2
```

#### Exception Expectation Ops
Each must trigger an out_of_range to pass:
```
THROW_GET <index>
THROW_SET <index> <value>
THROW_REMOVE <index>
THROW_ADD_AT <index> <value>
```

### Current Results (Latest Run)
From `unit_results.txt` after full suite execution:
```
TEST_SUMMARY:    passed=70 failed=0 total=70
ASSERT_SUMMARY:  passed=233 failed=0 total=233
```

### Coverage Highlights
- Construction (default & custom capacity, including zero / tiny) & growth (multiple expansions)
- All mutators: add (append & indexed), removeAt (front/middle/end sequences), set
- Query functions: get, size, empty, indexOf (duplicates, absent), contains
- String representation stability
- Deep copy correctness (copy constructor) and independence after divergence
- Assignment operator (including self-assign, multi-step ping-pong, post-clear)
- Clear semantics (capacity reset + reuse) without cross-impact on copies
- Exception safety for every guarded index path (negative, >=size, add out of bounds)
- Dual-structure divergence + synchronization scenarios (original vs copy vs assignment)

### Exit Code Policy
Currently non-zero exit code reflects only batch-mode failures (legacy). TEST mode failures are still visibly reported and can be wired into exit code easily if needed.

### Adding More Tests
Append new `TEST <id>` lines. Keep IDs unique and ordered for readability. Use dual-list ops to probe copy isolation and mutation ordering edge cases.

### Possible Future Enhancements
- Iterator explicit traversal & failure expectation commands (advance past end)
- Deterministic fuzz generator (FUZZ <ops> <seed>) with final checksum assertion
- Capacity introspection (debug-only) to assert growth ratios
- Integration of TEST assertion failures into process exit code

## Harness Development Notes
Design favors minimal parsing overhead and deterministic scenarios. Assertions within a test do not abort the test unless there is a parse/semantic error; all failures are aggregated for that line.

## Manual Test Driver
`main.cpp` retained for ad-hoc experimentation (not part of automated suite).

## Notes
- Only allowed standard headers used per assignment constraints.
- Growth policy uses 1.5x expansion with overflow guard.

## Next Possible Enrichments
- Add coverage for `SinglyLinkedList` and `VectorStore` via analogous TEST lines.
- Implement missing `SinglyLinkedList` & `VectorStore` methods and extend harness.

---
Happy testing!

## Manual Test Driver
`main.cpp` still contains a narrative, human-readable test flow—useful for debugging.

## Notes
- Only standard allowed headers are used per assignment constraint.
- Growth policy in `ArrayList` uses 1.5x strategy with overflow protection.

## Next Possible Enhancements
- Implement remaining `SinglyLinkedList` interface methods & add DSL coverage.
- Add similarity metrics & vector search tests for `VectorStore`.
- Add randomized fuzz generator producing temporary DSL files.

---
Happy testing!
