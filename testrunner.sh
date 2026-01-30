#!/bin/bash

# console args
COMPILER=./k0
LEX_ARG=-lexer
SYN_ARG=-tree
SEM_ARG=-symtab


valgrind_check() {
    local file=$1
    local args=$2
    local testname
    testname=$(basename "$file")

    valgrind --leak-check=full --error-exitcode=99 --quiet "$COMPILER" $args "$file" > /dev/null 2>&1
    if [[ $? -eq 0 ]]; then
        echo "    [O] Testing for memory leaks... passed"
        return 0
    else
        echo "    [X] Testing for memory leaks... failed"
        return 1
    fi
}


# LEXICAL

# counters
pass=0
fail=0

echo "==== Running invalid lexical tests ===="

# for file in tests/errors/lex/literals/lex*.kt; do
for file in $(find tests/errors/lex/literals/lex*.kt -type f -name 'lex*.kt'); do
    [[ -f "$file" ]] || continue
    testname=$(basename "$file")
    # read the token from the file
    input=$(cat "$file")

    echo "$input" | $COMPILER $LEX_ARG > /dev/null 2>&1
    # $COMPILER "$file" > /dev/null 2>&1
    result=$?

    if [[ "$result" -eq 1 ]]; then
        echo "[O] file: $testname... passed (expected 1, got $result)"
        ((pass++))
    else
        echo "[X] file: $testname... failed (expected 1, got $result)"
        ((fail++))
    fi
done

for file in $(find tests/kotlin/ -type f -name 'lex*.kt'); do
    [[ -f "$file" ]] || continue
    testname=$(basename "$file")
    # read the token from the file
    input=$(cat "$file")

    echo "$input" | $COMPILER $LEX_ARG > /dev/null 2>&1
    # $COMPILER "$file" > /dev/null 2>&1
    result=$?

    if [[ "$result" -eq 1 ]]; then
        echo "[O] file: $testname... passed (expected 1, got $result)"
        ((pass++))
    else
        echo "[X] file: $testname... failed (expected 1, got $result)"
        ((fail++))
    fi
done

echo ""
echo "==== Running valid lexical tests ===="

# for file in tests/k0/lex/literals/lex*.kt; do
for file in $(find tests/k0/ -type f -name 'lex*.kt'); do
    [[ -f "$file" ]] || continue
    testname=$(basename "$file")
    # read the token from the file
    input=$(cat "$file")

    echo "$input" | $COMPILER $LEX_ARG > /dev/null 2>&1
    # $COMPILER "$file" > /dev/null 2>&1
    result=$?

    if [[ "$result" -eq 0 ]]; then
        echo "[O] file: $testname... passed (expected 0, got $result)"
        ((pass++))
    else
        echo "[X] file: $testname... failed (expected 0, got $result)"
        ((fail++))
    fi
done

echo ""
echo "==== Lexical Test Summary ===="
echo "Passed: $pass"
echo "Failed: $fail"
echo "Total: $((pass + fail))"


# SYNTAX

# counters
pass=0
fail=0

echo ""
echo "==== Running invalid syntax tests ===="

for file in tests/errors/syn*.kt; do
    [[ -f "$file" ]] || continue
    testname=$(basename "$file")

    $COMPILER $SYN_ARG "$file" > /dev/null 2>&1
    result=$?

    if [[ "$result" -eq 2 ]]; then
        echo "[O] file: $testname... passed (expected 2, got $result)"
        ((pass++))
    else
        echo "[X] file: $testname... failed (expected 2, got $result)"
        ((fail++))
    fi
done

echo ""
echo "==== Running valid syntax tests ===="

for file in tests/k0/syn*.kt; do
    [[ -f "$file" ]] || continue
    testname=$(basename "$file")

    $COMPILER $SYN_ARG "$file" > /dev/null 2>&1
    result=$?

    if [[ "$result" -eq 0 ]]; then
        echo "[O] file: $testname... passed (expected 0, got $result)"
        valgrind_check "$file" $SYN_ARG || ((fail++))  # run valgrind for memory leaks
        ((pass++))
    else
        echo "[X] file: $testname... failed (expected 0, got $result)"
        ((fail++))
    fi
done

echo ""
echo "==== Syntax Test Summary ===="
echo "Passed: $pass"
echo "Failed: $fail"
echo "Total: $((pass + fail))"

# SEMANTICS

# counters
pass=0
fail=0

echo ""
echo "==== Running invalid semantics tests ===="

for file in tests/errors/sem*.kt; do
    [[ -f "$file" ]] || continue
    testname=$(basename "$file")

    $COMPILER $SEM_ARG "$file" > /dev/null 2>&1
    result=$?

    if [[ "$result" -eq 3 ]]; then
        echo "[O] file: $testname... passed (expected 3, got $result)"
        ((pass++))
    else
        echo "[X] file: $testname... failed (expected 3, got $result)"
        ((fail++))
    fi
done

echo ""
echo "==== Running valid semantics tests ===="

for file in tests/k0/sem*.kt; do
    [[ -f "$file" ]] || continue
    testname=$(basename "$file")

    $COMPILER $SEM_ARG "$file" > /dev/null 2>&1
    result=$?

    if [[ "$result" -eq 0 ]]; then
        echo "[O] file: $testname... passed (expected 0, got $result)"
        ((pass++))
    else
        echo "[X] file: $testname... failed (expected 0, got $result)"
        ((fail++))
    fi
done

echo ""
echo "==== Semantics Test Summary ===="
echo "Passed: $pass"
echo "Failed: $fail"
echo "Total: $((pass + fail))"
