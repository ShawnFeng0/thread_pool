project(thread_pool_test)

find_package(GTest)

if (GTest_FOUND)
    link_libraries(GTest::GTest GTest::Main)
else ()
    # Download and unpack googletest at configure time
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
    if (result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif ()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
    if (result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif ()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL)

    link_libraries(gtest gtest_main)
endif ()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCES)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} thread_pool)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
