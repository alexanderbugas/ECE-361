#!/bin/sh
# regress.sh: the HW6 regression test. Runs ./replay on every scenario in
# scenarios/ and compares its trace with the file of the same name in
# expected/.
# Usage, in hw06/ after make:   sh regress.sh
# Exits with 1 if any trace differs, so that make test fails.
fail=0
for input in scenarios/*.txt; do
    [ -e "$input" ] || continue     # no scenarios yet: the pattern stays unmatched
    name=$(basename "$input")
    if ./replay "$input" | diff - "expected/$name" > /dev/null; then
        echo "PASS  regression $name"
    else
        echo "FAIL  regression $name"
        fail=1
    fi
done
exit $fail
