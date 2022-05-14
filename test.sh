#!/bin/bash

for test_case in $(ls ./cases | grep .txt)
do
    echo ${test_case}
    ./build/hls "./cases/${test_case}" > "./results/${test_case}"
    ./build/checker "./cases/${test_case}" "./results/${test_case}"
    echo ""
done