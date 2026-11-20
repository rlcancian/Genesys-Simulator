include(GoogleTest)

add_executable(genesys_launcher_tests
    test_launcher.cpp
)

target_link_libraries(genesys_launcher_tests
    PRIVATE
        GTest::gtest_main
        genesys_launcher_runtime
        genesys_launcher_update
        Qt6::Core
)
target_compile_features(genesys_launcher_tests PRIVATE cxx_std_23)

set_property(TARGET genesys_launcher_tests PROPERTY FOLDER tests/launcher)

gtest_discover_tests(genesys_launcher_tests
    PROPERTIES
        LABELS "launcher"
)

add_executable(genesys_launcher_gui_smoke
    gui_smoke.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../UpdateDialog.cpp
)
target_link_libraries(genesys_launcher_gui_smoke
    PRIVATE
        genesys_launcher_runtime
        Qt6::Core
        Qt6::Widgets
)
target_compile_features(genesys_launcher_gui_smoke PRIVATE cxx_std_23)
set_property(TARGET genesys_launcher_gui_smoke PROPERTY FOLDER tests/launcher)
