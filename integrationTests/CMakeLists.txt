Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.3
)
FetchContent_GetProperties(Catch2)

if(NOT catch2_POPULATED)
    FetchContent_Populate(Catch2)
    add_subdirectory(
        "${catch2_SOURCE_DIR}"
        "${catch2_BINARY_DIR}"
    )
endif()

add_executable(
    IntegrationTests
    integrationTest.cpp
)
target_compile_options(IntegrationTests PUBLIC -fprofile-arcs -ftest-coverage)
target_link_options(IntegrationTests PUBLIC -lgcov --coverage)

target_link_libraries(IntegrationTests Catch2::Catch2WithMain)

file(COPY test1 DESTINATION ${CMAKE_BINARY_DIR})
file(COPY test2 DESTINATION ${CMAKE_BINARY_DIR})
file(COPY test3 DESTINATION ${CMAKE_BINARY_DIR})
