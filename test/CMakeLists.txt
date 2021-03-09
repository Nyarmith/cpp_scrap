add_library(catch_main STATIC catch_main.cc)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)

add_executable(test_one test_one.cc)
target_link_libraries(test_one my_cool_lib catch_main)
