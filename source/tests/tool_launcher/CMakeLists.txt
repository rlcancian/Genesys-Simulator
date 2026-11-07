find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(genesys_tool_launcher_tests
    ToolLauncherTests.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/gui/genesys/services/ToolLauncher.cpp
)

target_include_directories(genesys_tool_launcher_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/source/applications/gui/genesys/services
)
target_link_libraries(genesys_tool_launcher_tests PRIVATE
    Qt6::Core Qt6::Widgets GTest::gtest_main
)
target_compile_features(genesys_tool_launcher_tests PRIVATE cxx_std_23)

include(GoogleTest)
gtest_discover_tests(genesys_tool_launcher_tests
    TEST_PREFIX "StandaloneTools."
)
