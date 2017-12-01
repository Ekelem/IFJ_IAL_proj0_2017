#!/bin/bash
if [ ! -d "tests" ]; then
  echo "Source folder missing"
  exit 42
fi

PASSED=0
FAILED=0

cd tests/with_error
TOTAL_TESTS_WITH_ERROR=`ls -l | wc -l`
((TOTAL_TESTS_WITH_ERROR= (TOTAL_TESTS_WITH_ERROR-1)/2))
cd ../without_error
TOTAL_TESTS_WITHOUT_ERROR=`ls -l | wc -l`
((TOTAL_TESTS_WITHOUT_ERROR= (TOTAL_TESTS_WITHOUT_ERROR-1)/2))
cd ../..

echo "******START OF PART_1******"

# Return code should be equal to 0. Compare the user output and expected output
for ((i=1; i <= $TOTAL_TESTS_WITHOUT_ERROR; i++))
do
    FILESIZE=$(stat -c%s tests/without_error/$i.input)
    if [  $FILESIZE -le 1 ]; then
        break
    fi
    #Copy i.input to compiler.bas
    output=`cat tests/without_error/$i.input | ./IFJ17 > COMPILER/ifj17.bas && COMPILER/ic17int COMPILER/ifj17.bas`
    err_code=`echo $?`
    if [ $err_code -eq 0 ]; then
        expected_output=`cat tests/without_error/$i.expected`
        if [ $output != $expected_output ]; then
            echo -e "Test $i: Expected:$expected_output     Output:$output \e[31mFAILED\e[39m"
            ((FAILED++))
        else
            echo -e "Test $i: \e[32mOK\e[39m"
            ((PASSED++))
        fi
    else
        echo -e "Test $i: Expected error code:0  User error code:$err_code \e[31mFAILED\e[39m"
        ((FAILED++))
    fi
done
echo "*******END OF PART_1********"
echo ""

echo "***TESTING ERROR CODES***"
# We expect a non-zero return code
for ((i=1; i <= $TOTAL_TESTS_WITH_ERROR; i++))
do
    FILESIZE=$(stat -c%s tests/with_error/$i.input)
    if [  $FILESIZE -le 1 ]; then
        break
    fi
    #Copy i.input to compiler.bas
    output=`cat tests/with_error/$i.input | ./IFJ17 > COMPILER/ifj17.bas && COMPILER/ic17int COMPILER/ifj17.bas`
    err_code=`echo $?`
    if [ $err_code -ne 0 ]; then
        expected_output=`cat tests/with_error/$i.expected`
        if [ $err_code != $expected_output ]; then
            echo -e "Test $i: Expected:$expected_output     Output:$err_code  \e[31mFAILED\e[39m"
            ((FAILED++))
        else
            ((PASSED++))
            echo -e  "\e[32mOK\e[39m"
        fi
    else
        echo "Test $i: Expected error code:$expected_output  User error code:0 \e[31mFAILED\e[39m"
        ((FAILED++))
    fi
    echo ""
done

echo ""
echo "***************"
echo "PASSED: $PASSED"
echo "FAILED: $FAILED"
