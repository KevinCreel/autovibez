# AutoVibez Testing Suite

This directory contains a comprehensive testing suite for the AutoVibez application, following C++ testing best practices using Google Test (gtest) and Google Mock (gmock).

## Directory Structure

```
tests/
├── README.md                    # This file
├── fixtures/                    # Test fixtures and utilities
│   ├── test_fixtures.hpp       # Common test utilities header
│   └── test_fixtures.cpp       # Common test utilities implementation
├── mocks/                      # Mock objects for testing
│   ├── mock_audio_capture.hpp  # Mock audio capture class
│   ├── mock_mix_downloader.hpp # Mock mix downloader class
│   └── mock_projectm.hpp       # Mock ProjectM class
├── unit/                       # Unit tests
│   ├── config_manager_test.cpp # Config manager unit tests
│   ├── mix_metadata_test.cpp   # Mix metadata unit tests
│   ├── mix_database_test.cpp   # Mix database unit tests
│   └── mp3_analyzer_test.cpp   # MP3 analyzer unit tests
├── integration/                # Integration tests
│   ├── mix_manager_test.cpp    # Mix manager integration tests
│   └── audio_capture_test.cpp  # Audio capture integration tests
└── feature/                    # Feature tests
    ├── help_system_test.cpp    # Help system feature tests
    └── audio_device_test.cpp   # Audio device feature tests
```

## Test Categories

### Unit Tests (`tests/unit/`)
- **config_manager_test.cpp**: Tests for configuration file parsing and management
- **mix_metadata_test.cpp**: Tests for YAML mix metadata loading and parsing
- **mix_database_test.cpp**: Tests for SQLite database operations
- **mp3_analyzer_test.cpp**: Tests for MP3 metadata extraction

### Integration Tests (`tests/integration/`)
- **mix_manager_test.cpp**: Tests for the interaction between mix metadata, database, and management systems
- **audio_capture_test.cpp**: Tests for audio capture system integration

### Feature Tests (`tests/feature/`)
- **help_system_test.cpp**: Tests for help overlay and user interface features
- **audio_device_test.cpp**: Tests for audio device selection and cycling features

### Test Fixtures (`tests/fixtures/`)
- **test_fixtures.hpp/cpp**: Common utilities for creating test data, temporary files, and mock objects

### Mock Objects (`tests/mocks/`)
- **mock_audio_capture.hpp**: Mock for audio capture system
- **mock_mix_downloader.hpp**: Mock for mix download functionality
- **mock_projectm.hpp**: Mock for ProjectM visualization engine

## Building and Running Tests

### Prerequisites
- CMake 3.16 or higher
- Google Test (automatically downloaded by CMake)
- All AutoVibez dependencies (SDL2, SQLite3, yaml-cpp, etc.)

### Building Tests
```bash
# From the project root
mkdir -p build
cd build
cmake -DBUILD_TESTING=ON ..
make
```

### Running Tests
```bash
# Run all tests
make test

# Run tests with verbose output
ctest --verbose

# Run specific test executable
./autovibez_tests

# Run tests with specific filter
./autovibez_tests --gtest_filter="ConfigManagerTest*"
```

### Test Output
Tests provide detailed output including:
- Test case names and descriptions
- Pass/fail status
- Error messages for failed tests
- Performance metrics (where applicable)

## Test Coverage

### Configuration Management
- ✅ Config file parsing and validation
- ✅ Default value handling
- ✅ Error handling for missing/invalid files
- ✅ Template specialization testing
- ✅ Whitespace and comment handling

### Mix Metadata System
- ✅ YAML file loading and parsing
- ✅ Mix object creation and validation
- ✅ Error handling for invalid data
- ✅ Remote file loading (structure tested)

### Database Operations
- ✅ SQLite database initialization
- ✅ Mix CRUD operations
- ✅ Query operations (by genre, artist, etc.)
- ✅ Random mix selection
- ✅ Smart random mix with prioritization
- ✅ Play statistics tracking
- ✅ Favorite management

### Integration Testing
- ✅ Mix manager initialization
- ✅ YAML to database synchronization
- ✅ Random mix selection with preferences
- ✅ Genre-based filtering
- ✅ Artist-based filtering

### Feature Testing
- ✅ Help system structure and organization
- ✅ Audio device functionality
- ✅ User interface features
- ✅ Error handling and edge cases

## Testing Best Practices

### Test Organization
- **Arrange**: Set up test data and conditions
- **Act**: Execute the code being tested
- **Assert**: Verify the expected outcomes

### Test Naming
- Tests are named descriptively: `TestClass_TestMethod_ExpectedBehavior`
- Example: `ConfigManagerTest_ConstructorWithValidFile_LoadsConfigurationSuccessfully`

### Test Isolation
- Each test is independent and can run in any order
- Tests use temporary files and directories
- Cleanup is automatic via RAII and test fixtures

### Mock Usage
- Mocks are used for external dependencies (SDL, ProjectM, etc.)
- Mock objects provide controlled test environments
- Mock expectations verify correct interaction patterns

### Error Testing
- Tests cover both success and failure scenarios
- Error conditions are explicitly tested
- Exception handling is verified

### Performance Testing
- Tests verify performance characteristics where relevant
- Memory usage is monitored
- Resource cleanup is verified

## Adding New Tests

### Unit Tests
1. Create test file in `tests/unit/`
2. Include necessary headers and test fixtures
3. Create test class inheriting from `::testing::Test`
4. Implement test methods with descriptive names
5. Use appropriate assertions (EXPECT_*, ASSERT_*)

### Integration Tests
1. Create test file in `tests/integration/`
2. Test interactions between multiple components
3. Use real dependencies where possible
4. Mock external systems (network, audio hardware)

### Feature Tests
1. Create test file in `tests/feature/`
2. Test complete user workflows
3. Verify end-to-end functionality
4. Test user interface interactions

### Mock Objects
1. Create mock header in `tests/mocks/`
2. Use Google Mock macros (MOCK_METHOD)
3. Provide helper methods for test setup
4. Document mock behavior and expectations

## Continuous Integration

The testing suite is designed to work with CI/CD systems:
- Tests run automatically on code changes
- Coverage reports are generated
- Performance regression testing
- Cross-platform compatibility testing

## Debugging Tests

### Verbose Output
```bash
./autovibez_tests --gtest_verbose
```

### Debugging Specific Tests
```bash
# Run with debugger
gdb --args ./autovibez_tests --gtest_filter="ConfigManagerTest*"
```

### Test Logging
- Tests use `printf` for debug output
- Log files are created in temporary directories
- Error messages include context information

## Performance Considerations

### Test Execution Speed
- Unit tests run quickly (< 1 second each)
- Integration tests may take longer due to file I/O
- Feature tests are the slowest due to UI simulation

### Memory Usage
- Tests use minimal memory
- Temporary files are cleaned up automatically
- No memory leaks in test code

### Resource Management
- Database connections are properly closed
- File handles are managed via RAII
- Temporary directories are cleaned up

## Contributing

When adding new tests:
1. Follow the existing naming conventions
2. Use the provided test fixtures
3. Add appropriate documentation
4. Ensure tests pass on all supported platforms
5. Update this README if adding new test categories

## Troubleshooting

### Common Issues
- **Missing dependencies**: Ensure all required libraries are installed
- **Permission errors**: Tests create temporary files in system temp directory
- **Platform differences**: Some tests may behave differently on different OS

### Getting Help
- Check test output for specific error messages
- Review test logs in temporary directories
- Consult Google Test documentation for advanced features 