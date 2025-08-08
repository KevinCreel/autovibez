#!/bin/bash

# AutoVibez Build Script
# Wraps CMake commands for common development workflows

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Debug"
BUILD_DIR="build"
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)
INSTALL_PREFIX=""

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    # Check for required commands
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists pkg-config; then
        missing_deps+=("pkg-config")
    fi
    
    # Check for required libraries via pkg-config
    local required_libs=("sdl2" "sqlite3" "yaml-cpp" "libprojectM-4" "glm" "imgui" "taglib" "libcurl")
    
    for lib in "${required_libs[@]}"; do
        if ! pkg-config --exists "$lib" 2>/dev/null; then
            missing_deps+=("$lib")
        fi
    done
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install the missing packages and try again."
        exit 1
    fi
    
    print_success "All dependencies found!"
}

# Function to configure the project
configure_project() {
    print_status "Configuring project..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
        print_status "Created build directory: $BUILD_DIR"
    fi
    
    cd "$BUILD_DIR"
    
    local cmake_args=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")
    
    if [ -n "$INSTALL_PREFIX" ]; then
        cmake_args+=("-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX")
    fi
    
    cmake "${cmake_args[@]}" ..
    
    if [ $? -eq 0 ]; then
        print_success "Project configured successfully!"
    else
        print_error "Configuration failed!"
        exit 1
    fi
    
    cd ..
}

# Function to build the project
build_project() {
    print_status "Building project..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Run 'configure' first."
        exit 1
    fi
    
    cd "$BUILD_DIR"
    
    cmake --build . --parallel "$PARALLEL_JOBS"
    
    if [ $? -eq 0 ]; then
        print_success "Build completed successfully!"
    else
        print_error "Build failed!"
        exit 1
    fi
    
    cd ..
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Run 'configure' and 'build' first."
        exit 1
    fi
    
    cd "$BUILD_DIR"
    
    # Check if test executable exists
    if [ ! -f "autovibez_tests" ]; then
        print_error "Test executable not found. Run 'build' first."
        exit 1
    fi
    
    # Run tests with CTest
    ctest --output-on-failure --verbose
    
    if [ $? -eq 0 ]; then
        print_success "All tests passed!"
    else
        print_error "Some tests failed!"
        exit 1
    fi
    
    cd ..
}

# Function to clean build files
clean_build() {
    print_status "Cleaning build files..."
    
    if [ -d "$BUILD_DIR" ]; then
        cd "$BUILD_DIR"
        cmake --build . --target clean
        cd ..
        print_success "Build files cleaned!"
    else
        print_warning "Build directory not found. Nothing to clean."
    fi
}

# Function to install the application
install_app() {
    print_status "Installing application..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Run 'configure' and 'build' first."
        exit 1
    fi
    
    cd "$BUILD_DIR"
    
    cmake --build . --target install
    
    if [ $? -eq 0 ]; then
        print_success "Application installed successfully!"
    else
        print_error "Installation failed!"
        exit 1
    fi
    
    cd ..
}

# Function to format code with clang-format
format_code() {
    print_status "Formatting code with clang-format..."
    
    # Check if clang-format is available
    if ! command_exists clang-format; then
        print_error "clang-format not found. Please install it first."
        exit 1
    fi
    
    # Find all C++ source files and format them
    local source_files=($(find src/ -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | grep -v build))
    local test_files=($(find tests/ -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | grep -v build))
    
    local all_files=("${source_files[@]}" "${test_files[@]}")
    
    if [ ${#all_files[@]} -eq 0 ]; then
        print_warning "No C++ files found to format."
        return 0
    fi
    
    # Format all files
    clang-format -i "${all_files[@]}"
    
    if [ $? -eq 0 ]; then
        print_success "Code formatting completed!"
    else
        print_error "Code formatting failed!"
        exit 1
    fi
}

# Function to show help
show_help() {
    echo "AutoVibez Build Script"
    echo ""
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  configure    Configure the project with CMake"
    echo "  build        Build the project"
    echo "  test         Run the test suite"
    echo "  clean        Clean build files"
    echo "  install      Install the application"
    echo "  format       Format code with clang-format"
    echo "  all          Configure, build, and test in one go"
    echo "  help         Show this help message"
    echo ""
    echo "Options:"
    echo "  --build-type TYPE    Set build type (Debug|Release) [default: Debug]"
    echo "  --build-dir DIR      Set build directory [default: build]"
    echo "  --jobs N             Set number of parallel jobs [default: auto]"
    echo "  --prefix PATH        Set installation prefix"
    echo "  --check-deps         Check dependencies before building"
    echo ""
    echo "Examples:"
    echo "  $0 configure"
    echo "  $0 build"
    echo "  $0 format"
    echo "  $0 all --build-type Release"
    echo "  $0 install --prefix /usr/local"
}

# Function to do everything
do_all() {
    print_status "Running full build process..."
    configure_project
    build_project
    run_tests
    print_success "Full build process completed successfully!"
}

# Parse command line arguments
COMMAND=""
CHECK_DEPS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        configure|build|test|clean|install|format|all|help)
            COMMAND="$1"
            shift
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --check-deps)
            CHECK_DEPS=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    print_error "Invalid build type: $BUILD_TYPE. Use Debug or Release."
    exit 1
fi

# Check dependencies if requested
if [ "$CHECK_DEPS" = true ]; then
    check_dependencies
fi

# Execute command
case $COMMAND in
    configure)
        configure_project
        ;;
    build)
        build_project
        ;;
    test)
        run_tests
        ;;
    clean)
        clean_build
        ;;
    install)
        install_app
        ;;
    format)
        format_code
        ;;
    all)
        do_all
        ;;
    help|"")
        show_help
        ;;
    *)
        print_error "Unknown command: $COMMAND"
        show_help
        exit 1
        ;;
esac
