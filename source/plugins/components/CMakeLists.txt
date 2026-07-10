file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_plugins_components STATIC
    ${GENESYS_PLUGINS_COMPONENTS_SOURCES}
)

target_include_directories(genesys_plugins_components
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_plugins_components
    PUBLIC
        genesys_kernel_util
        genesys_plugins_data
)

target_compile_features(genesys_plugins_components PUBLIC cxx_std_23)

# Optional GLPK dependency for MetabolicFluxBalance and WholeCellModeling FBA.
# Detection order: pkg-config first, then raw find_library fallback.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GLPK QUIET glpk)
endif()
if(NOT GLPK_FOUND)
    find_library(GLPK_LIBRARIES_RAW NAMES glpk)
    find_path(GLPK_INCLUDE_DIRS_RAW NAMES glpk.h)
    if(GLPK_LIBRARIES_RAW AND GLPK_INCLUDE_DIRS_RAW)
        set(GLPK_FOUND TRUE)
        set(GLPK_LIBRARIES "${GLPK_LIBRARIES_RAW}")
        set(GLPK_INCLUDE_DIRS "${GLPK_INCLUDE_DIRS_RAW}")
    endif()
endif()

if(GLPK_FOUND)
    message(STATUS "GenESyS: GLPK found — MetabolicFluxBalance and WholeCellModeling FBA will use GLPK LP solver (system dependency of BiochemicalSimulation and WholeCellModeling plugins)")
    target_compile_definitions(genesys_plugins_components PRIVATE GENESYS_HAVE_GLPK)
    target_include_directories(genesys_plugins_components PRIVATE ${GLPK_INCLUDE_DIRS})
    target_link_libraries(genesys_plugins_components PRIVATE ${GLPK_LIBRARIES})
else()
    message(STATUS "GenESyS: GLPK not found — MetabolicFluxBalance will use built-in basis-enumeration solver (suitable for small models only; install libglpk-dev for WCM-scale networks)")
endif()
