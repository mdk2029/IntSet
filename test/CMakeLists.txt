
# Download and unpack googletest at configure time

if(NOT GTEST_FOUND)
  find_package(gtest REQUIRED HINTS ${CMAKE_CURRENT_LIST_DIR}/../cmake)
endif()

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
add_executable(intset_test "")
target_sources(intset_test PUBLIC
    "${TEST_SOURCES}"
    )
target_link_libraries(intset_test ${PROJECT_NAME} gmock_main )
target_include_directories(intset_test
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )

