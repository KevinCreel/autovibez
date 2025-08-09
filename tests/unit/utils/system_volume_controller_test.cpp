#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "system_volume_controller.hpp"

using namespace AutoVibez::Utils;
using ::testing::Return;
using ::testing::StrictMock;

class MockSystemVolumeController : public ISystemVolumeController {
public:
    MOCK_METHOD(int, getCurrentVolume, (), (override));
    MOCK_METHOD(bool, setVolume, (int volumeLevel), (override));
    MOCK_METHOD(bool, increaseVolume, (int step), (override));
    MOCK_METHOD(bool, decreaseVolume, (int step), (override));
    MOCK_METHOD(bool, isAvailable, (), (override));
    MOCK_METHOD(std::string, getLastError, (), (override));
};

class SystemVolumeControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockController = std::make_unique<StrictMock<MockSystemVolumeController>>();
    }

    std::unique_ptr<MockSystemVolumeController> mockController;
};

TEST_F(SystemVolumeControllerTest, FactoryCreatesSupportedController) {
    bool supported = SystemVolumeControllerFactory::isSupported();
    EXPECT_TRUE(supported);

    auto controller = SystemVolumeControllerFactory::create();
    ASSERT_NE(controller, nullptr);
}

TEST_F(SystemVolumeControllerTest, IsAvailableReturnsBooleanValue) {
    EXPECT_CALL(*mockController, isAvailable())
        .WillOnce(Return(true));

    bool available = mockController->isAvailable();
    EXPECT_TRUE(available);
}

TEST_F(SystemVolumeControllerTest, IsAvailableCanReturnFalse) {
    EXPECT_CALL(*mockController, isAvailable())
        .WillOnce(Return(false));

    bool available = mockController->isAvailable();
    EXPECT_FALSE(available);
}

TEST_F(SystemVolumeControllerTest, GetCurrentVolumeReturnsValidRange) {
    EXPECT_CALL(*mockController, getCurrentVolume())
        .WillOnce(Return(50));

    int volume = mockController->getCurrentVolume();
    EXPECT_GE(volume, 0);
    EXPECT_LE(volume, 100);
}

TEST_F(SystemVolumeControllerTest, GetCurrentVolumeCanReturnError) {
    EXPECT_CALL(*mockController, getCurrentVolume())
        .WillOnce(Return(-1));

    int volume = mockController->getCurrentVolume();
    EXPECT_EQ(volume, -1);
}

TEST_F(SystemVolumeControllerTest, SetVolumeSucceedsWithValidInput) {
    EXPECT_CALL(*mockController, setVolume(50))
        .WillOnce(Return(true));

    bool result = mockController->setVolume(50);
    EXPECT_TRUE(result);
}

TEST_F(SystemVolumeControllerTest, SetVolumeFailsWithInvalidInputTooLow) {
    EXPECT_CALL(*mockController, setVolume(-1))
        .WillOnce(Return(false));

    bool result = mockController->setVolume(-1);
    EXPECT_FALSE(result);
}

TEST_F(SystemVolumeControllerTest, SetVolumeFailsWithInvalidInputTooHigh) {
    EXPECT_CALL(*mockController, setVolume(101))
        .WillOnce(Return(false));

    bool result = mockController->setVolume(101);
    EXPECT_FALSE(result);
}

TEST_F(SystemVolumeControllerTest, IncreaseVolumeSucceeds) {
    EXPECT_CALL(*mockController, increaseVolume(5))
        .WillOnce(Return(true));

    bool result = mockController->increaseVolume(5);
    EXPECT_TRUE(result);
}

TEST_F(SystemVolumeControllerTest, IncreaseVolumeCanFail) {
    EXPECT_CALL(*mockController, increaseVolume(5))
        .WillOnce(Return(false));

    bool result = mockController->increaseVolume(5);
    EXPECT_FALSE(result);
}

TEST_F(SystemVolumeControllerTest, DecreaseVolumeSucceeds) {
    EXPECT_CALL(*mockController, decreaseVolume(10))
        .WillOnce(Return(true));

    bool result = mockController->decreaseVolume(10);
    EXPECT_TRUE(result);
}

TEST_F(SystemVolumeControllerTest, DecreaseVolumeCanFail) {
    EXPECT_CALL(*mockController, decreaseVolume(10))
        .WillOnce(Return(false));

    bool result = mockController->decreaseVolume(10);
    EXPECT_FALSE(result);
}

TEST_F(SystemVolumeControllerTest, GetLastErrorReturnsEmptyStringWhenNoError) {
    EXPECT_CALL(*mockController, getLastError())
        .WillOnce(Return(""));

    std::string error = mockController->getLastError();
    EXPECT_TRUE(error.empty());
}

TEST_F(SystemVolumeControllerTest, GetLastErrorReturnsErrorMessage) {
    EXPECT_CALL(*mockController, getLastError())
        .WillOnce(Return("Audio system not available"));

    std::string error = mockController->getLastError();
    EXPECT_EQ(error, "Audio system not available");
}