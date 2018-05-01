find_package(Threads REQUIRED)

add_executable(intset-bench "")
target_sources(intset-bench PUBLIC
    "bench.cpp")
target_include_directories(intset-bench PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )
target_link_libraries(intset-bench ${PROJECT_NAME} benchmark Threads::Threads)
