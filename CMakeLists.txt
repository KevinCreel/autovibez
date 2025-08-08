cmake_minimum_required(VERSION 3.16)
project(autovibez VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cross-platform installation paths
# Platform-specific path detection and fallbacks
if(WIN32)
    # Windows: Use APPDATA environment variable
    if(DEFINED ENV{APPDATA})
        set(USER_DATA_HOME "$ENV{APPDATA}")
    else()
        set(USER_DATA_HOME "$ENV{USERPROFILE}/AppData/Roaming")
    endif()
    
    if(DEFINED ENV{LOCALAPPDATA})
        set(USER_CACHE_HOME "$ENV{LOCALAPPDATA}")
    else()
        set(USER_CACHE_HOME "$ENV{USERPROFILE}/AppData/Local")
    endif()
    
    set(USER_CONFIG_HOME "${USER_DATA_HOME}")
    set(USER_STATE_HOME "${USER_DATA_HOME}")
    
elseif(APPLE)
    # macOS: Use standard macOS directories
    set(USER_DATA_HOME "$ENV{HOME}/Library/Application Support")
    set(USER_CONFIG_HOME "$ENV{HOME}/Library/Application Support")
    set(USER_CACHE_HOME "$ENV{HOME}/Library/Caches")
    set(USER_STATE_HOME "$ENV{HOME}/Library/Application Support")
    
else()
    # Linux/Unix: Use XDG Base Directory Specification
    if(DEFINED ENV{XDG_DATA_HOME})
        set(USER_DATA_HOME "$ENV{XDG_DATA_HOME}")
    else()
        set(USER_DATA_HOME "$ENV{HOME}/.local/share")
    endif()
    
    if(DEFINED ENV{XDG_CONFIG_HOME})
        set(USER_CONFIG_HOME "$ENV{XDG_CONFIG_HOME}")
    else()
        set(USER_CONFIG_HOME "$ENV{HOME}/.config")
    endif()
    
    if(DEFINED ENV{XDG_CACHE_HOME})
        set(USER_CACHE_HOME "$ENV{XDG_CACHE_HOME}")
    else()
        set(USER_CACHE_HOME "$ENV{HOME}/.cache")
    endif()
    
    if(DEFINED ENV{XDG_STATE_HOME})
        set(USER_STATE_HOME "$ENV{XDG_STATE_HOME}")
    else()
        set(USER_STATE_HOME "$ENV{HOME}/.local/state")
    endif()
endif()

# Set installation prefix based on platform and environment
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(WIN32)
        # Windows: Default to user's AppData
        set(CMAKE_INSTALL_PREFIX "${USER_DATA_HOME}" CACHE PATH "Installation prefix" FORCE)
    elseif(APPLE)
        # macOS: Default to user's Application Support
        set(CMAKE_INSTALL_PREFIX "${USER_DATA_HOME}" CACHE PATH "Installation prefix" FORCE)
    else()
        # Linux/Unix: Default to user's .local
        set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Installation prefix" FORCE)
    endif()
    message(STATUS "Setting default installation prefix to: ${CMAKE_INSTALL_PREFIX}")
endif()

# Set data directory path using platform-specific paths
set(DATADIR_PATH "${USER_DATA_HOME}/autovibez")

# Enable testing
enable_testing()

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(CURL REQUIRED)

# Find TagLib using pkg-config
pkg_check_modules(TAGLIB REQUIRED taglib)

# Find ProjectM and GLM using pkg-config
pkg_check_modules(PROJECTM REQUIRED libprojectM-4)
pkg_check_modules(GLM REQUIRED glm)

# Find ImGui
find_package(PkgConfig REQUIRED)
pkg_check_modules(IMGUI REQUIRED imgui)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/audio)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/data)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/utils)
include_directories(${IMGUI_INCLUDE_DIRS})

# Add executable
add_executable(autovibez
    # Core application files
    src/core/autovibez_app.cpp
    src/core/autovibez_app.hpp
    src/core/main.cpp
    src/core/setup.cpp
    src/core/setup.hpp
    
    # Audio components
    src/audio/audio_capture.cpp
    src/audio/audio_capture.hpp
    src/audio/loopback.cpp
    src/audio/loopback.hpp
    src/audio/mix_player.cpp
    src/audio/mix_player.hpp
    src/audio/mp3_analyzer.cpp
    src/audio/mp3_analyzer.hpp
    
    # Data management
    src/data/config_manager.cpp
    src/data/config_manager.hpp
    src/data/database_interfaces.hpp
    src/data/mix_database.cpp
    src/data/mix_database.hpp
    src/data/mix_downloader.cpp
    src/data/mix_downloader.hpp
    src/data/mix_manager.cpp
    src/data/mix_manager.hpp
    src/data/mix_metadata.cpp
    src/data/mix_metadata.hpp
    src/data/mix_query_builder.cpp
    src/data/mix_query_builder.hpp
    src/data/mix_validator.cpp
    src/data/mix_validator.hpp
    src/data/smart_mix_selector.cpp
    src/data/smart_mix_selector.hpp
    src/data/sqlite_connection.cpp
    src/data/sqlite_connection.hpp
    
    # User interface
    src/core/preset_manager.cpp
    src/core/preset_manager.hpp
    src/ui/help_overlay.cpp
    src/ui/help_overlay.hpp
    src/ui/message_overlay.cpp
    src/ui/message_overlay.hpp
    src/ui/message_overlay_wrapper.cpp
    src/ui/message_overlay_wrapper.hpp
    
    # Platform-specific utilities
    src/platform/opengl.h
    src/platform/path_manager.cpp
    src/platform/path_manager.hpp
    
    # General utilities
    src/utils/constants.hpp
    src/utils/uuid_utils.cpp
    src/utils/uuid_utils.hpp
    src/utils/error_handler.hpp
    src/utils/audio_utils.cpp
    src/utils/audio_utils.hpp
    src/utils/datetime_utils.cpp
    src/utils/datetime_utils.hpp
    src/utils/url_utils.cpp
    src/utils/url_utils.hpp
    src/utils/path_utils.cpp
    src/utils/path_utils.hpp
    src/utils/string_utils.cpp
    src/utils/string_utils.hpp
    src/utils/json_utils.cpp
    src/utils/json_utils.hpp
    src/data/base_metadata.hpp
)

# Link libraries
target_link_libraries(autovibez
    PRIVATE
    ${PROJECTM_LIBRARIES}
    ${GLM_LIBRARIES}
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_mixer::SDL2_mixer
    OpenGL::GL
    projectM-4-playlist
    SQLite::SQLite3
    yaml-cpp
    CURL::libcurl
    ${TAGLIB_LIBRARIES}
    ${IMGUI_LIBRARIES}
)

# Platform-specific installation destinations
if(WIN32)
    # Windows: Install to user's AppData
    install(TARGETS autovibez
        RUNTIME DESTINATION bin
    )
    
    # Install assets to user's AppData
    install(DIRECTORY assets/
        DESTINATION share/autovibez
        USE_SOURCE_PERMISSIONS
    )
    
    # Install desktop file (Windows doesn't use .desktop files)
    # But we can install a shortcut or registry entry if needed
    
    # Install data files
    install(DIRECTORY assets/ DESTINATION share/autovibez FILES_MATCHING PATTERN "*.milk" PATTERN "*.jpg" PATTERN "*.png")
    
elseif(APPLE)
    # macOS: Install to user's Application Support
    install(TARGETS autovibez
        RUNTIME DESTINATION bin
    )
    
    # Install assets to user's Application Support
    install(DIRECTORY assets/
        DESTINATION share/autovibez
        USE_SOURCE_PERMISSIONS
    )
    
    # Install desktop file (macOS uses .app bundles, not .desktop files)
    # But we can install a .app bundle if needed
    
    # Install data files
    install(DIRECTORY assets/ DESTINATION share/autovibez FILES_MATCHING PATTERN "*.milk" PATTERN "*.jpg" PATTERN "*.png")
    
else()
    # Linux/Unix: Install to user's .local
    install(TARGETS autovibez
        RUNTIME DESTINATION bin
    )
    
    # Install assets to user's .local/share
    install(DIRECTORY assets/
        DESTINATION share/autovibez
        USE_SOURCE_PERMISSIONS
    )
    
    # Install desktop file to user's applications directory
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/autovibez.desktop")
        install(FILES assets/autovibez.desktop DESTINATION share/applications)
    endif()
        
    # Install icon to user's icon directory (if it exists) - support multiple formats
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon.svg")
        install(FILES assets/icon.svg DESTINATION share/icons/hicolor/scalable/apps)
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon-16.png")
        install(FILES assets/icon-16.png DESTINATION share/icons/hicolor/16x16/apps)
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon-32.png")
        install(FILES assets/icon-32.png DESTINATION share/icons/hicolor/32x32/apps)
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon-256.png")
        install(FILES assets/icon-256.png DESTINATION share/icons/hicolor/256x256/apps)
    endif()
        
    # Install data files to user's .local/share
    install(DIRECTORY assets/ DESTINATION share/autovibez FILES_MATCHING PATTERN "*.milk" PATTERN "*.jpg" PATTERN "*.png")
endif()

# Include directories
target_include_directories(autovibez PRIVATE ${PROJECTM_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${TAGLIB_INCLUDE_DIRS})
target_link_directories(autovibez PRIVATE ${PROJECTM_LIBRARY_DIRS} ${GLM_LIBRARY_DIRS} ${TAGLIB_LIBRARY_DIRS})
target_compile_options(autovibez PRIVATE ${PROJECTM_CFLAGS_OTHER} ${GLM_CFLAGS_OTHER} ${TAGLIB_CFLAGS_OTHER})

# Add compile definitions
target_compile_definitions(autovibez PRIVATE DATADIR_PATH="${DATADIR_PATH}")

# Set properties for Windows
if(MSVC)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
        set_target_properties(autovibez
            PROPERTIES
            VS_DPI_AWARE "PerMonitor"
        )
    endif()
    
    # Add Windows-specific compile definitions
    target_compile_definitions(autovibez PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
    )
    
    # Link Windows libraries
    target_link_libraries(autovibez PRIVATE
        ws2_32
        winmm
        ole32
        oleaut32
    )
endif()

# Set properties for macOS
if(APPLE)
    target_compile_definitions(autovibez PRIVATE
        __APPLE__
    )
endif() 

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Create test sources list (exclude main.cpp and ui_system files)
set(AUTOVIBEZ_TEST_SOURCES
    # Core application files
    src/core/autovibez_app.cpp
    src/core/autovibez_app.hpp
    src/core/setup.cpp
    src/core/setup.hpp
    
    # Audio components
    src/audio/audio_capture.cpp
    src/audio/audio_capture.hpp
    src/audio/loopback.cpp
    src/audio/loopback.hpp
    src/audio/mix_player.cpp
    src/audio/mix_player.hpp
    src/audio/mp3_analyzer.cpp
    src/audio/mp3_analyzer.hpp
    
    # Data management
    src/data/config_manager.cpp
    src/data/config_manager.hpp
    src/data/database_interfaces.hpp
    src/data/mix_database.cpp
    src/data/mix_database.hpp
    src/data/mix_downloader.cpp
    src/data/mix_downloader.hpp
    src/data/mix_manager.cpp
    src/data/mix_manager.hpp
    src/data/mix_metadata.cpp
    src/data/mix_metadata.hpp
    src/data/mix_query_builder.cpp
    src/data/mix_query_builder.hpp
    src/data/mix_validator.cpp
    src/data/mix_validator.hpp
    src/data/smart_mix_selector.cpp
    src/data/smart_mix_selector.hpp
    src/data/sqlite_connection.cpp
    src/data/sqlite_connection.hpp
    
    # User interface
    src/core/preset_manager.cpp
    src/core/preset_manager.hpp
    src/ui/help_overlay.cpp
    src/ui/help_overlay.hpp
    
    # Platform-specific utilities
    src/platform/opengl.h
    src/platform/path_manager.cpp
    src/platform/path_manager.hpp
    
    # General utilities
    src/utils/constants.hpp
    src/utils/uuid_utils.cpp
    src/utils/uuid_utils.hpp
    src/utils/error_handler.hpp
    src/utils/audio_utils.cpp
    src/utils/audio_utils.hpp
    src/utils/datetime_utils.cpp
    src/utils/datetime_utils.hpp
    src/utils/url_utils.cpp
    src/utils/url_utils.hpp
    src/utils/path_utils.cpp
    src/utils/path_utils.hpp
    src/utils/string_utils.cpp
    src/utils/string_utils.hpp
    src/utils/json_utils.cpp
    src/utils/json_utils.hpp
    src/data/base_metadata.hpp
    
    # Unit tests - Utils
    tests/unit/utils/string_utils_test.cpp
    tests/unit/utils/uuid_utils_test.cpp
    tests/unit/utils/path_utils_test.cpp
    tests/unit/utils/url_utils_test.cpp
    tests/unit/utils/audio_utils_test.cpp
    tests/unit/utils/datetime_utils_test.cpp
    tests/unit/utils/constants_test.cpp
    tests/unit/utils/error_handler_test.cpp
    tests/unit/utils/json_utils_test.cpp
    
    # Unit tests - Data
    tests/unit/data/base_metadata_test.cpp
    tests/unit/data/mix_database_test.cpp
    tests/unit/data/config_manager_test.cpp
    tests/unit/data/mix_metadata_test.cpp
    tests/unit/data/mix_downloader_test.cpp
    tests/unit/data/mix_manager_test.cpp
    tests/unit/data/mix_validator_test.cpp
    tests/unit/data/mix_query_builder_test.cpp
    tests/unit/data/sqlite_connection_test.cpp
    tests/unit/data/smart_mix_selector_test.cpp
    
    # Unit tests - Audio
    tests/unit/audio/mp3_analyzer_test.cpp
    tests/unit/audio/mix_player_test.cpp
    tests/unit/audio/loopback_test.cpp
    
    # Unit tests - Core
    tests/unit/core/preset_manager_test.cpp
    tests/unit/core/autovibez_app_test.cpp
    tests/unit/core/setup_test.cpp
    tests/unit/core/main_test.cpp
    
    # Unit tests - Integration
    tests/unit/integration/app_workflow_test.cpp
    tests/unit/integration/data_persistence_test.cpp
    tests/unit/integration/config_chain_test.cpp
    tests/unit/integration/threading_concurrency_test.cpp
    tests/unit/integration/platform_integration_test.cpp
    
    # Unit tests - Platform
    tests/unit/platform/path_manager_test.cpp
    
    # Unit tests - Audio
    tests/unit/audio/audio_capture_test.cpp
    
    # Unit tests - UI
    tests/unit/ui/help_overlay_test.cpp
    tests/unit/ui/message_overlay_test.cpp
    
    # Source files needed for tests (autovibez_app.cpp depends on these)
    src/ui/message_overlay.cpp
    src/ui/message_overlay.hpp
    src/ui/message_overlay_wrapper.cpp
    src/ui/message_overlay_wrapper.hpp
)

# Add test executable
add_executable(autovibez_tests ${AUTOVIBEZ_TEST_SOURCES})

# Include directories for tests
target_include_directories(autovibez_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECTM_INCLUDE_DIRS} 
    ${GLM_INCLUDE_DIRS} 
    ${TAGLIB_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIRS}
)

# Link libraries for tests
target_link_libraries(autovibez_tests
    PRIVATE
    gtest
    gtest_main
    gmock
    gmock_main
    ${PROJECTM_LIBRARIES}
    ${GLM_LIBRARIES}
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_mixer::SDL2_mixer
    OpenGL::GL
    projectM-4-playlist
    SQLite::SQLite3
    yaml-cpp
    CURL::libcurl
    ${TAGLIB_LIBRARIES}
    ${IMGUI_LIBRARIES}
)

# Set compile definitions for tests
target_compile_definitions(autovibez_tests PRIVATE DATADIR_PATH="${DATADIR_PATH}")

# Set properties for Windows tests
if(MSVC)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
        set_target_properties(autovibez_tests
            PROPERTIES
            VS_DPI_AWARE "PerMonitor"
        )
    endif()
    
    target_compile_definitions(autovibez_tests PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
    )
    
    target_link_libraries(autovibez_tests PRIVATE
        ws2_32
        winmm
        ole32
        oleaut32
    )
endif()

# Set properties for macOS tests
if(APPLE)
    target_compile_definitions(autovibez_tests PRIVATE
        __APPLE__
    )
endif()

# Register the test with CTest
add_test(NAME autovibez_tests COMMAND autovibez_tests)

# Set test properties
set_tests_properties(autovibez_tests PROPERTIES
    ENVIRONMENT "GTEST_COLOR=1"
)