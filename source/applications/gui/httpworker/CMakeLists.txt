find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)
if(Qt6_FOUND)
    set(GENESYS_QT_PACKAGE Qt6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    set(GENESYS_QT_PACKAGE Qt5)
endif()

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)

add_executable(genesys_httpworker_gui_application
    main.cpp
    WebWorkerDialog.cpp
    WebWorkerDialog.h
)

target_include_directories(genesys_httpworker_gui_application
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_SOURCE_DIR}/source/applications/worker
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(genesys_httpworker_gui_application
    PRIVATE
        ${GENESYS_QT_PACKAGE}::Core
        ${GENESYS_QT_PACKAGE}::Gui
        ${GENESYS_QT_PACKAGE}::Widgets
        genesys_worker_core
)

target_compile_features(genesys_httpworker_gui_application PRIVATE cxx_std_23)

set_target_properties(genesys_httpworker_gui_application
    PROPERTIES
        OUTPUT_NAME genesys-httpworker-gui
)

if(TARGET genesys_worker_core)
    add_dependencies(genesys_httpworker_gui_application genesys_worker_core)
endif()

install(TARGETS genesys_httpworker_gui_application
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_target(genesys_httpworker_gui DEPENDS genesys_httpworker_gui_application)
