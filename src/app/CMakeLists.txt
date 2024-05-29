add_executable(
    App
    main.cpp
)
set_target_properties(App PROPERTIES OUTPUT_NAME "inter")
target_include_directories(App PUBLIC include)
target_compile_options(App PUBLIC -fprofile-arcs -ftest-coverage)
target_link_options(App PUBLIC -lgcov --coverage)

target_link_libraries(App Reader)
target_link_libraries(App Lexer)
target_link_libraries(App Parser)
target_link_libraries(App Interpreter)
