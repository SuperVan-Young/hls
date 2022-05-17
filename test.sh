#!/bin/bash

hls=./build/hls
display=./build/display
checker=../build/src/checker
score_calc=../build/src/score_calc
case_dir=./HLS-lab1/cases/

# a basic checking
echo validity checking
for i in {1..10}
do
    ${display} ${case_dir}/test_case${i}.txt > ./results/input_test_case${i}.txt
    ${hls} ${case_dir}/test_case${i}.txt > ./results/test_case${i}.txt
    # ${case_dir}${checker} ${case_dir}/test_case${i}.txt ./results/test_case${i}.txt
done

# thorough checking
cd ./HLS-lab1/cases

for i in {1..10}
do
    ./test.sh ${i} ../../build/hls ${checker} ${score_calc}
done