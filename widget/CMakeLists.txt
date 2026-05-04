cmake_minimum_required(VERSION 3.16)

set(AWESOMEQT_WIDGET_BUILD_MODE "INDIVIDUAL" CACHE STRING
    "Widget build mode: INDIVIDUAL (per-widget .so) or MERGED (single .so)")

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/AwesomeQtWidgets.cmake")

if(AWESOMEQT_WIDGET_BUILD_MODE STREQUAL "MERGED")
    set(AWESOMEQT_USING_MERGE_MODE TRUE)
endif()

add_subdirectory(statusled)
# Future widgets:
# add_subdirectory(other_widget)

if(AWESOMEQT_USING_MERGE_MODE)
    awesomeqt_build_merged_widget(AwesomeQtWidgets)
endif()

# --- Install rules ---
if(AWESOMEQT_USING_MERGE_MODE)
    awesomeqt_install_widget(AwesomeQtWidgets
        LIBRARY_DEST "lib"
        INCLUDE_DEST "include/AwesomeQtWidgets"
    )
else()
    get_property(_all_targets GLOBAL PROPERTY AWESOMEQT_WIDGET_TARGETS)
    foreach(_t IN LISTS _all_targets)
        awesomeqt_install_widget(${_t}
            LIBRARY_DEST "lib"
            INCLUDE_DEST "include/AwesomeQtWidgets"
        )
    endforeach()
endif()
