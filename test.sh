#!/bin/bash

for test_case in $(ls ./cases | grep test_case[1-9].txt)
do
    echo ${test_case}
    ./build/display "./cases/${test_case}" > "./results/input_${test_case}"
    ./build/hls "./cases/${test_case}" > "./results/${test_case}"
    ./build/checker "./cases/${test_case}" "./results/${test_case}"
    echo ""
done