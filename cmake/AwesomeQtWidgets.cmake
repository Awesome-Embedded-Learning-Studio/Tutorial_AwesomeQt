# AwesomeQtWidgets.cmake
# Build infrastructure for AwesomeQt custom widget libraries.
#
# Provides:
#   awesomeqt_add_widget(<name> [MERGE_MODE] [BASE_DIR <path>] ...)
#   awesomeqt_build_merged_widget(<target_name> [OUTPUT_NAME <name>] ...)
#   awesomeqt_install_widget(<target_name> [LIBRARY_DEST <path>] ...)
#
# Prerequisites (set by caller):
#   - CMAKE_AUTOMOC, CMAKE_AUTOUIC, CMAKE_AUTORCC as needed
#   - find_package(Qt6 ...) called by caller before using this module

include_guard(GLOBAL)

# --------------------------------------------------------------------------
# awesomeqt_add_widget
# --------------------------------------------------------------------------
# Collects source files and optionally creates a shared library target.
#
# In INDIVIDUAL mode (default): creates add_library(<name> SHARED ...)
# In MERGE_MODE: appends files to global properties for later aggregation.
# --------------------------------------------------------------------------
function(awesomeqt_add_widget _widget_name)
    set(_options MERGE_MODE)
    set(_one_value_args BASE_DIR INCLUDE_DIR SRC_DIR OUTPUT_NAME)
    set(_multi_value_args HEADERS SOURCES UI_FILES QT_COMPONENTS)
    cmake_parse_arguments(_ARG "${_options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})

    # --- Resolve directories ---
    if(_ARG_BASE_DIR)
        set(_base_dir "${_ARG_BASE_DIR}")
    else()
        set(_base_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    if(_ARG_INCLUDE_DIR)
        set(_include_dir "${_ARG_INCLUDE_DIR}")
    else()
        set(_include_dir "${_base_dir}/include")
    endif()

    if(_ARG_SRC_DIR)
        set(_src_dir "${_ARG_SRC_DIR}")
    else()
        set(_src_dir "${_base_dir}/src")
    endif()

    if(_ARG_OUTPUT_NAME)
        set(_output_name "${_ARG_OUTPUT_NAME}")
    else()
        set(_output_name "${_widget_name}")
    endif()

    # --- Auto-discover or use explicit file lists ---
    set(_public_headers "")
    set(_sources "")
    set(_ui_files "")

    if(_ARG_HEADERS)
        set(_public_headers ${_ARG_HEADERS})
    elseif(EXISTS "${_include_dir}")
        file(GLOB_RECURSE _public_headers
            "${_include_dir}/*.h"
            "${_include_dir}/*.hpp"
        )
    endif()

    if(_ARG_SOURCES)
        set(_sources ${_ARG_SOURCES})
    elseif(EXISTS "${_src_dir}")
        file(GLOB_RECURSE _cpp_files "${_src_dir}/*.cpp" "${_src_dir}/*.cxx")
        file(GLOB_RECURSE _priv_h_files "${_src_dir}/*.h" "${_src_dir}/*.hpp")
        set(_sources ${_cpp_files} ${_priv_h_files})
    endif()

    if(_ARG_UI_FILES)
        set(_ui_files ${_ARG_UI_FILES})
    elseif(EXISTS "${_src_dir}")
        file(GLOB_RECURSE _ui_files "${_src_dir}/*.ui")
    endif()

    set(_all_sources ${_sources} ${_public_headers} ${_ui_files})

    # --- MERGE_MODE: collect into global properties ---
    if(_ARG_MERGE_MODE)
        set_property(GLOBAL APPEND PROPERTY AWESOMEQT_MERGE_SOURCES ${_all_sources})
        set_property(GLOBAL APPEND PROPERTY AWESOMEQT_MERGE_HEADERS ${_public_headers})
        set_property(GLOBAL APPEND PROPERTY AWESOMEQT_MERGE_UI_FILES ${_ui_files})
        if(EXISTS "${_include_dir}")
            set_property(GLOBAL APPEND PROPERTY AWESOMEQT_MERGE_INCLUDE_DIRS "${_include_dir}")
        endif()
        if(_ARG_QT_COMPONENTS)
            set_property(GLOBAL APPEND PROPERTY AWESOMEQT_MERGE_QT_COMPONENTS ${_ARG_QT_COMPONENTS})
        endif()
        message(STATUS "[AwesomeQt] Collected widget: ${_widget_name} (merge mode)")
        return()
    endif()

    # --- INDIVIDUAL mode: create the shared library target ---
    add_library(${_widget_name} SHARED ${_all_sources})

    target_include_directories(${_widget_name}
        PUBLIC
            $<BUILD_INTERFACE:${_include_dir}>
            $<INSTALL_INTERFACE:include/AwesomeQtWidgets/${_widget_name}>
    )

    set_target_properties(${_widget_name} PROPERTIES
        OUTPUT_NAME "${_output_name}"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        AWESOMEQT_WIDGET_BASE_DIR "${_base_dir}"
        AWESOMEQT_WIDGET_INCLUDE_DIR "${_include_dir}"
    )

    # Link Qt components if specified
    if(_ARG_QT_COMPONENTS)
        foreach(_comp IN LISTS _ARG_QT_COMPONENTS)
            set(_qt_target "Qt6::${_comp}")
            if(NOT TARGET "${_qt_target}")
                message(FATAL_ERROR "[AwesomeQt] Qt target '${_qt_target}' not found. "
                    "Call find_package(Qt6 COMPONENTS ${_comp}) before awesomeqt_add_widget().")
            endif()
            list(APPEND _qt_targets "${_qt_target}")
        endforeach()
        target_link_libraries(${_widget_name} PUBLIC ${_qt_targets})
    endif()

    set_property(GLOBAL APPEND PROPERTY AWESOMEQT_WIDGET_TARGETS ${_widget_name})

    message(STATUS "[AwesomeQt] Built widget library: ${_widget_name} -> lib${_output_name}.so")
endfunction()

# --------------------------------------------------------------------------
# awesomeqt_build_merged_widget
# --------------------------------------------------------------------------
# Consumes global properties collected by awesomeqt_add_widget(MERGE_MODE)
# and creates a single merged shared library.
# --------------------------------------------------------------------------
function(awesomeqt_build_merged_widget _target_name)
    set(_one_value_args OUTPUT_NAME INSTALL_INCLUDE_DIR)
    cmake_parse_arguments(_ARG "" "${_one_value_args}" "" ${ARGN})

    if(_ARG_OUTPUT_NAME)
        set(_output_name "${_ARG_OUTPUT_NAME}")
    else()
        set(_output_name "${_target_name}")
    endif()

    if(_ARG_INSTALL_INCLUDE_DIR)
        set(_install_inc "${_ARG_INSTALL_INCLUDE_DIR}")
    else()
        set(_install_inc "include/AwesomeQtWidgets")
    endif()

    get_property(_sources GLOBAL PROPERTY AWESOMEQT_MERGE_SOURCES)
    get_property(_headers GLOBAL PROPERTY AWESOMEQT_MERGE_HEADERS)
    get_property(_ui      GLOBAL PROPERTY AWESOMEQT_MERGE_UI_FILES)
    get_property(_inc_dirs GLOBAL PROPERTY AWESOMEQT_MERGE_INCLUDE_DIRS)

    set(_all_files ${_sources} ${_ui})

    if(NOT _all_files)
        message(WARNING "[AwesomeQt] No sources collected for merged widget '${_target_name}'")
        return()
    endif()

    add_library(${_target_name} SHARED ${_all_files})

    list(REMOVE_DUPLICATES _inc_dirs)

    set(_build_includes "")
    foreach(_d IN LISTS _inc_dirs)
        list(APPEND _build_includes "$<BUILD_INTERFACE:${_d}>")
    endforeach()

    target_include_directories(${_target_name}
        PUBLIC
            ${_build_includes}
            $<INSTALL_INTERFACE:${_install_inc}>
    )

    set_target_properties(${_target_name} PROPERTIES
        OUTPUT_NAME "${_output_name}"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        AWESOMEQT_MERGE_INCLUDE_DIRS "${_inc_dirs}"
    )

    # Link collected Qt components
    get_property(_qt_components GLOBAL PROPERTY AWESOMEQT_MERGE_QT_COMPONENTS)
    if(_qt_components)
        list(REMOVE_DUPLICATES _qt_components)
        set(_qt_targets "")
        foreach(_comp IN LISTS _qt_components)
            list(APPEND _qt_targets "Qt6::${_comp}")
        endforeach()
        target_link_libraries(${_target_name} PUBLIC ${_qt_targets})
    endif()

    set_property(GLOBAL APPEND PROPERTY AWESOMEQT_WIDGET_TARGETS ${_target_name})

    message(STATUS "[AwesomeQt] Built merged widget library: ${_target_name} -> lib${_output_name}.so")
endfunction()

# --------------------------------------------------------------------------
# awesomeqt_install_widget
# --------------------------------------------------------------------------
# Installs a widget library target and its public headers.
# Works for both individual and merged targets.
# --------------------------------------------------------------------------
macro(awesomeqt_install_widget _target_name)
    set(_one_value_args RUNTIME_DEST LIBRARY_DEST ARCHIVE_DEST INCLUDE_DEST)
    cmake_parse_arguments(_INSTALL "" "${_one_value_args}" "" ${ARGN})

    if(NOT _INSTALL_RUNTIME_DEST)
        set(_INSTALL_RUNTIME_DEST "bin")
    endif()
    if(NOT _INSTALL_LIBRARY_DEST)
        set(_INSTALL_LIBRARY_DEST "lib")
    endif()
    if(NOT _INSTALL_ARCHIVE_DEST)
        set(_INSTALL_ARCHIVE_DEST "lib")
    endif()
    if(NOT _INSTALL_INCLUDE_DEST)
        set(_INSTALL_INCLUDE_DEST "include/AwesomeQtWidgets")
    endif()

    install(TARGETS ${_target_name}
        RUNTIME DESTINATION "${_INSTALL_RUNTIME_DEST}"
        LIBRARY DESTINATION "${_INSTALL_LIBRARY_DEST}"
        ARCHIVE DESTINATION "${_INSTALL_ARCHIVE_DEST}"
    )

    get_target_property(_merge_inc_dirs ${_target_name} AWESOMEQT_MERGE_INCLUDE_DIRS)
    if(_merge_inc_dirs)
        foreach(_dir IN LISTS _merge_inc_dirs)
            get_filename_component(_widget_parent "${_dir}" DIRECTORY)
            get_filename_component(_widget_name "${_widget_parent}" NAME)
            install(DIRECTORY "${_dir}/"
                DESTINATION "${_INSTALL_INCLUDE_DEST}/${_widget_name}"
                FILES_MATCHING
                    PATTERN "*.h"
                    PATTERN "*.hpp"
            )
        endforeach()
    else()
        get_target_property(_inc_dir ${_target_name} AWESOMEQT_WIDGET_INCLUDE_DIR)
        if(_inc_dir AND EXISTS "${_inc_dir}")
            install(DIRECTORY "${_inc_dir}/"
                DESTINATION "${_INSTALL_INCLUDE_DEST}/${_target_name}"
                FILES_MATCHING
                    PATTERN "*.h"
                    PATTERN "*.hpp"
            )
        endif()
    endif()

    message(STATUS "[AwesomeQt] Install rules configured for: ${_target_name}")
endmacro()
