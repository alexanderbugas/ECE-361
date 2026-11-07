#!/bin/sh
# regress.sh: everything make test runs, ending with the one line the sheet
# asks for: the number of tests that passed and failed.
#   1. the unit tests (tests/test_all), with their summary line taken off
#   2. every scenario in scenarios/: ./vent's trace compared with the file of
#      the same name in expected/, as HW2's and HW6's regress.sh do
# Usage, in project/ after make and make tests/test_all:   sh regress.sh
# Exits with 1 if anything failed, so that make test fails.

unit=$(./tests/test_all)                    # the unit tests' output
echo "$unit" | sed '$d'                     # all of it but the last line, the summary
summary=$(echo "$unit" | tail -n 1)         # "N passed, M failed"
passed=$(echo "$summary" | cut -d' ' -f1)
failed=$(echo "$summary" | cut -d' ' -f3)

for input in scenarios/*.txt; do
    [ -e "$input" ] || continue             # no scenarios: the pattern stays unmatched
    name=$(basename "$input")
    if ./vent "$input" | diff - "expected/$name" > /dev/null; then
        echo "PASS  scenario $name"
        passed=$((passed + 1))
    else
        echo "FAIL  scenario $name"
        failed=$((failed + 1))
    fi
done

echo
echo "$passed passed, $failed failed"
[ "$failed" -eq 0 ]                         # the exit status: 0 only if nothing failed
