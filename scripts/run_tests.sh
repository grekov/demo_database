#!/bin/bash

cd ../tests

if [[ $? -ne 0 ]]
then
        exit 1
fi

total=0
passed=0

tests=`ls`

echo ""
echo "Running tests..."
echo ""

for one_test in $tests
do
        if [[ $one_test =~ _test$ ]]
        then
                echo "[RUN] : $one_test"
                total=$((total+1))
                ./${one_test} > /dev/null 2>&1

                if [[ $? -eq 0 ]]
                then
                       echo "[PASS]: $one_test passed"
                       passed=$((passed+1))
                else
                       echo "[FAIL]: *** $one_test failed"
                fi
        fi
done

echo ""
echo "Passed / total: $passed / $total"

exit 0
