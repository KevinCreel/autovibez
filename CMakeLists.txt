cmake_minimum_required(VERSION 3.16)
project(autovibez VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set data directory path
set(DATADIR_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/autovibez")

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

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

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
    ${TAGLIB_LIBRARIES}
    ${CURL_LIBRARIES}
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