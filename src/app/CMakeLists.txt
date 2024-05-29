add_library(
    AppAssets OBJECT
    include/appExceptions.hpp
    include/argumentParsing.hpp
    argumentParsing.cpp
)
target_include_directories(AppAssets PUBLIC include)
target_compile_options(AppAssets PUBLIC -fprofile-arcs -ftest-coverage)

target_link_libraries(AppAssets Reader)

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
target_link_libraries(App AppAssets)
