message("Inside subdirectory test")

enable_testing()

add_executable(input input.cpp)
target_link_libraries(input libio)

macro(add_test_input TEST_NAME TEST_INPUT)
    add_test(NAME input.${TEST_NAME} COMMAND input ${CMAKE_SOURCE_DIR}/cases/${TEST_INPUT})
    message("Add test input." ${TEST_NAME})    
endmacro(add_test_input)

add_test_input(test1 test_case1.txt)
add_test_input(test2 test_case2.txt)
add_test_input(test3 test_case3.txt)