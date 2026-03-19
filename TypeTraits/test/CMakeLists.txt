add_executable(run_tests
        run_tests.cc
        traits_tests.cc
)

target_link_libraries(run_tests
        gtest
        gtest_main
)
target_compile_features(run_tests PRIVATE cxx_std_23)
add_test(NAME run_tests COMMAND run_tests)

