project(thread_pool_examples)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

link_libraries(pthread)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} thread_pool)
