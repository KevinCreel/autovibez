cmake_minimum_required(VERSION 3.16)
project(autovibez VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set data directory path
set(DATADIR_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/autovibez")

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
include_directories(${IMGUI_INCLUDE_DIRS})

# Add executable
add_executable(autovibez
    src/config_manager.cpp
    src/config_manager.hpp
    src/audio_capture.cpp
    src/audio_capture.hpp
    src/autovibez_app.cpp
    src/autovibez_app.hpp
    src/main.cpp
    src/setup.cpp
    src/setup.hpp
    src/loopback.cpp
    src/loopback.hpp
    src/opengl.h
    src/mix_manager.cpp
    src/mix_manager.hpp
    src/mix_database.cpp
    src/mix_database.hpp
    src/mix_downloader.cpp
    src/mix_downloader.hpp
    src/mix_player.cpp
    src/mix_player.hpp
    src/mix_metadata.cpp
    src/mix_metadata.hpp
    src/mix_display.cpp
    src/mix_display.hpp
    src/mp3_analyzer.cpp
    src/mp3_analyzer.hpp
    src/simple_ui.cpp
    src/simple_ui.hpp
    # New modular components
    src/constants.hpp
    src/config_defaults.hpp
    src/string_utils.hpp
    src/path_manager.cpp
    src/path_manager.hpp
    src/app_state.cpp
    src/app_state.hpp
    src/preset_manager.cpp
    src/preset_manager.hpp
    src/audio_manager.cpp
    src/audio_manager.hpp
    src/input_handler.cpp
    src/input_handler.hpp
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

# Install executable
install(TARGETS autovibez
    RUNTIME DESTINATION bin
)

# Install assets (presets and textures)
install(DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/autovibez
    USE_SOURCE_PERMISSIONS
)

# Install rules
install(TARGETS autovibez DESTINATION bin)

# Install desktop file
install(FILES assets/autovibez.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# Install icon (if it exists) - support multiple formats
if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon.svg")
    install(FILES assets/icon.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon-16.png")
    install(FILES assets/icon-16.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/16x16/apps)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon-32.png")
    install(FILES assets/icon-32.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icon-256.png")
    install(FILES assets/icon-256.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
endif()

# Install data files
install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/autovibez FILES_MATCHING PATTERN "*.milk" PATTERN "*.jpg" PATTERN "*.png")

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
    src/config_manager.cpp
    src/config_manager.hpp
    src/audio_capture.cpp
    src/audio_capture.hpp
    src/autovibez_app.cpp
    src/autovibez_app.hpp
    src/setup.cpp
    src/setup.hpp
    src/loopback.cpp
    src/loopback.hpp
    src/opengl.h
    src/mix_manager.cpp
    src/mix_manager.hpp
    src/mix_database.cpp
    src/mix_database.hpp
    src/mix_downloader.cpp
    src/mix_downloader.hpp
    src/mix_player.cpp
    src/mix_player.hpp
    src/mix_metadata.cpp
    src/mix_metadata.hpp
    src/mix_display.cpp
    src/mix_display.hpp
    src/mp3_analyzer.cpp
    src/mp3_analyzer.hpp
    src/simple_ui.cpp
    src/simple_ui.hpp
    tests/fixtures/test_fixtures.cpp
    tests/fixtures/test_fixtures.hpp
    tests/unit/config_manager_test.cpp
    tests/unit/mix_metadata_test.cpp
    tests/unit/mix_database_test.cpp
    tests/unit/mp3_analyzer_test.cpp
    tests/integration/mix_manager_test.cpp
    tests/integration/audio_capture_test.cpp
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