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
    src/audio/audio_manager.cpp
    src/audio/audio_manager.hpp
    src/audio/loopback.cpp
    src/audio/loopback.hpp
    src/audio/mix_player.cpp
    src/audio/mix_player.hpp
    src/audio/mp3_analyzer.cpp
    src/audio/mp3_analyzer.hpp
    
    # Data management
    src/data/config_manager.cpp
    src/data/config_manager.hpp
    src/data/mix_database.cpp
    src/data/mix_database.hpp
    src/data/mix_display.cpp
    src/data/mix_display.hpp
    src/data/mix_downloader.cpp
    src/data/mix_downloader.hpp
    src/data/mix_manager.cpp
    src/data/mix_manager.hpp
    src/data/mix_metadata.cpp
    src/data/mix_metadata.hpp
    
    # User interface
    src/ui/app_state.cpp
    src/ui/app_state.hpp
    src/ui/input_handler.cpp
    src/ui/input_handler.hpp
    src/ui/preset_manager.cpp
    src/ui/preset_manager.hpp
    src/ui/simple_ui.cpp
    src/ui/simple_ui.hpp
    
    # Platform-specific utilities
    src/platform/opengl.h
    src/platform/path_manager.cpp
    src/platform/path_manager.hpp
    
    # General utilities
    src/utils/config_defaults.hpp
    src/utils/constants.hpp
    src/utils/string_utils.hpp
    src/utils/logger.cpp
    src/utils/logger.hpp
    src/utils/resource_guard.hpp
    src/utils/input_validator.cpp
    src/utils/input_validator.hpp
    src/utils/console_output.cpp
    src/utils/console_output.hpp
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
    src/audio/audio_manager.cpp
    src/audio/audio_manager.hpp
    src/audio/loopback.cpp
    src/audio/loopback.hpp
    src/audio/mix_player.cpp
    src/audio/mix_player.hpp
    src/audio/mp3_analyzer.cpp
    src/audio/mp3_analyzer.hpp
    
    # Data management
    src/data/config_manager.cpp
    src/data/config_manager.hpp
    src/data/mix_database.cpp
    src/data/mix_database.hpp
    src/data/mix_downloader.cpp
    src/data/mix_downloader.hpp
    src/data/mix_manager.cpp
    src/data/mix_manager.hpp
    src/data/mix_metadata.cpp
    src/data/mix_metadata.hpp
    src/data/mix_display.cpp
    src/data/mix_display.hpp
    
    # User interface
    src/ui/app_state.cpp
    src/ui/app_state.hpp
    src/ui/preset_manager.cpp
    src/ui/preset_manager.hpp
    src/ui/simple_ui.cpp
    src/ui/simple_ui.hpp
    src/ui/input_handler.cpp
    src/ui/input_handler.hpp
    
    # Platform-specific utilities
    src/platform/opengl.h
    src/platform/path_manager.cpp
    src/platform/path_manager.hpp
    
    # General utilities
    src/utils/constants.hpp
    src/utils/config_defaults.hpp
    src/utils/string_utils.hpp
    src/utils/logger.cpp
    src/utils/logger.hpp
    src/utils/resource_guard.hpp
    src/utils/input_validator.cpp
    src/utils/input_validator.hpp
    src/utils/console_output.cpp
    src/utils/console_output.hpp
    
    # Test fixtures
    tests/fixtures/test_fixtures.cpp
    tests/fixtures/test_fixtures.hpp
    
    # Unit tests
    tests/unit/config_manager_test.cpp
    tests/unit/mix_database_test.cpp
    tests/unit/mix_metadata_test.cpp
    tests/unit/mix_player_test.cpp
    tests/unit/mp3_analyzer_test.cpp
    tests/unit/path_manager_test.cpp
    tests/unit/logger_test.cpp
    tests/unit/input_validator_test.cpp
    tests/unit/resource_guard_test.cpp
    tests/unit/console_output_test.cpp
    
    # Integration tests
    tests/integration/audio_capture_test.cpp
    tests/integration/mix_manager_test.cpp
    tests/integration/memory_cleanup_test.cpp
    tests/integration/thread_safety_test.cpp
    
    # Feature tests
    tests/feature/help_system_test.cpp
    tests/feature/audio_device_test.cpp
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