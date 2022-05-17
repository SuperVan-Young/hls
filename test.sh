#!/bin/bash

# a basic checking
for test_case in $(ls ./HLS-lab1/cases | grep test_case[1-9].txt)
do
    echo ${test_case}
    ./build/display ./HLS-lab1/cases/${test_case} > ./results/input_${test_case}
    ./build/hls ./HLS-lab1/cases/${test_case} > ./results/${test_case}
    echo ""
done

# thorough checking
cd ./HLS-lab1/cases

for i in [1..8]
do
    ./test.sh ${i} ../../build/hls ../build/src/checker ../build/src/score_calc
done