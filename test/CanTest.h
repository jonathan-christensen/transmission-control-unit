#if !defined(CAN_TEST_H) && defined(NATIVE)
#define CAN_TEST_H

#include <gtest/gtest.h>
#include <Arduino.h>
#include "../lib/Can/Can.h"
#include "../lib/Storage/Storage.h"
#include "mock/MockFlexCAN_T4.h"

using namespace fakeit;
using namespace std;

class CanTest : public ::testing::Test {
  protected:
    Mock<FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>> mockCan1;
    FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>& can1;

    Mock<FlexCAN_T4<CAN2, RX_SIZE_16, TX_SIZE_16>> mockCan2;
    FlexCAN_T4<CAN2, RX_SIZE_16, TX_SIZE_16>& can2;
    
    Mock<Storage> mockStorage;
    Storage& storage;

    Mock<Transmission> mockTransmission;
    Transmission& transmission;

    Can can;

    CanTest() :
        can1(mockCan1.get()),
        can2(mockCan2.get()),
        storage(mockStorage.get()),
        transmission(mockTransmission.get()),
        can(can1, can2, storage, transmission) {

    }

    void SetUp() override {
        // Storage
        When(Method(mockStorage, setUpDelay)).AlwaysReturn(true);
        When(Method(mockStorage, setDownDelay)).AlwaysReturn(true);
        When(Method(mockStorage, setOutput)).AlwaysReturn(true);
        When(Method(mockStorage, setTimeout)).AlwaysReturn(true);
        When(Method(mockStorage, setStart)).AlwaysReturn(true);
        When(Method(mockStorage, setEnd)).AlwaysReturn(true);
        When(Method(mockStorage, setFriction)).AlwaysReturn(true);
        When(Method(mockStorage, setAutoLaunch)).AlwaysReturn(true);

        When(Method(mockStorage, upDelay)).AlwaysReturn(50);
        When(Method(mockStorage, downDelay)).AlwaysReturn(200);
        When(Method(mockStorage, output)).AlwaysReturn(90);
        When(Method(mockStorage, timeout)).AlwaysReturn(50);
        When(Method(mockStorage, start)).AlwaysReturn(1600);
        When(Method(mockStorage, end)).AlwaysReturn(1300);
        When(Method(mockStorage, friction)).AlwaysReturn(1500);
        When(Method(mockStorage, autoLaunch)).AlwaysReturn(1);

        // Transmission
        When(Method(mockTransmission, setRpm)).AlwaysReturn();
        When(Method(mockTransmission, shift)).AlwaysReturn();
        When(Method(mockTransmission, clutchInput)).AlwaysReturn();
        When(Method(mockTransmission, setClutchPosition)).AlwaysReturn();
        When(Method(mockTransmission, clutchPosition)).AlwaysReturn();
        When(Method(mockTransmission, clutchPercentage)).AlwaysReturn();

        // FlexCAN_T4
        When(Method(mockCan1, begin)).AlwaysReturn();
        When(Method(mockCan1, setBaudRate)).AlwaysReturn();
        When(Method(mockCan1, read)).AlwaysReturn(0);
        When(Method(mockCan1, write)).AlwaysReturn(1);

        When(Method(mockCan2, begin)).AlwaysReturn();
        When(Method(mockCan2, setBaudRate)).AlwaysReturn();
        When(Method(mockCan2, read)).AlwaysReturn(0);
        When(Method(mockCan2, write)).AlwaysReturn(1);

        When(Method(ArduinoFake(), millis)).AlwaysReturn(0);
        When(Method(ArduinoFake(), analogRead)).AlwaysReturn(0);

        can.begin();
    }

    void TearDown() override {
        ArduinoFakeReset();
    }
};

TEST_F(CanTest, begin) {
    Verify(Method(mockCan1, begin)).Once();
    Verify(Method(mockCan1, setBaudRate).Using(1000000)).Once();

    Verify(Method(mockCan2, begin)).Once();
    Verify(Method(mockCan2, setBaudRate).Using(1000000)).Once();
}

TEST_F(CanTest, group0) {
    When(Method(mockCan1, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x360u,
            .buf = {0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        };

        return 1;
    });

    can.update();

    Verify(Method(mockTransmission, setRpm).Using(0x1234)).Once();
}

TEST_F(CanTest, shiftSettings) {
    When(Method(mockCan1, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x656u,
            .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}
        };

        return 1;
    });

    can.update();

    Verify(Method(mockStorage, setUpDelay).Using(0x1122)).Once();
    Verify(Method(mockStorage, setDownDelay).Using(0x3344)).Once();
    Verify(Method(mockStorage, setOutput).Using(0x5566)).Once();
    Verify(Method(mockStorage, setTimeout).Using(0x7788)).Once();
}

TEST_F(CanTest, clutchSettings) {
    When(Method(mockCan1, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x658u,
            .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00, 0x01}
        };
        
        return 1;
    });

    can.update();

    Verify(Method(mockStorage, setStart).Using(0x1122)).Once();
    Verify(Method(mockStorage, setEnd).Using(0x3344)).Once();
    Verify(Method(mockStorage, setFriction).Using(0x5566)).Once();
    Verify(Method(mockStorage, setAutoLaunch).Using(1)).Once();
}

TEST_F(CanTest, setClutch) {
    When(OverloadedMethod(mockTransmission, state, void(int))).AlwaysReturn();
    When(Method(mockCan1, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x65au,
            .buf = {0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00}
        };

        return 1;
    });
    
    can.update();

    Verify(Method(mockTransmission, setClutchPosition).Using(0x1122)).Once();
    Verify(OverloadedMethod(mockTransmission, state, void(int)).Using(0x3344)).Once();
}

TEST_F(CanTest, canTimeout) {
    When(Method(ArduinoFake(), millis)).AlwaysReturn(100);
    
    can.update();
    
    Verify(Method(mockTransmission, setRpm).Using(0)).Once();
}

#pragma region broadcastShiftSettings
TEST_F(CanTest, broadcastShiftSettings) {
    const CAN_message_t reference = {
        .id = 0x655u,
        .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}
    };

    CAN_message_t written;

    When(Method(mockCan1, write)).Do([&](const CAN_message_t& msg) -> int {
        written = msg;
        return 1;
    });

    can.broadcastShiftSettings();

    When(Method(ArduinoFake(), millis)).AlwaysReturn(200);
    When(Method(mockStorage, upDelay)).AlwaysReturn(0x1122);
    When(Method(mockStorage, downDelay)).AlwaysReturn(0x3344);
    When(Method(mockStorage, output)).AlwaysReturn(0x5566);
    When(Method(mockStorage, timeout)).AlwaysReturn(0x7788);

    can.broadcastShiftSettings();

    Verify(Method(mockCan1, write)).Once();
    EXPECT_EQ(written, reference) << "Incorrect 'shift_settings' message written to CAN interface";
}

TEST_F(CanTest, broadcastShiftSettingsFrequency) {
    for(int i=0; i<1000; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        can.broadcastShiftSettings();
    }

    Verify(Method(mockCan1, write)).Exactly(5);
}
#pragma endregion

#pragma region broadcastClutchSettings
TEST_F(CanTest, broadcastClutchSettings) {
    const CAN_message_t reference = {
        .id = 0x0657,
        .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00, 0x01}
    };

    CAN_message_t written;

    When(Method(mockCan1, write)).Do([&](const CAN_message_t& msg) -> int {
        written = msg;
        return 1;
    });

    can.broadcastClutchSettings();

    When(Method(ArduinoFake(), millis)).AlwaysReturn(200);
    When(Method(mockStorage, start)).AlwaysReturn(0x1122);
    When(Method(mockStorage, end)).AlwaysReturn(0x3344);
    When(Method(mockStorage, friction)).AlwaysReturn(0x5566);
    When(Method(mockStorage, autoLaunch)).AlwaysReturn(1);

    can.broadcastClutchSettings();

    Verify(Method(mockCan1, write)).Once();
    EXPECT_EQ(written, reference) << "Incorrect 'clutch_settings' message written to CAN interface";
}

TEST_F(CanTest, broadcastClutchSettingsFrequency) {
    for(int i=0; i<1000; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        can.broadcastClutchSettings();
    }

    Verify(Method(mockCan1, write)).Exactly(5);
}
#pragma endregion

#pragma region broadcastClutch
TEST_F(CanTest, broadcastClutch) {
    const CAN_message_t reference = {
        .id = 0x659u,
        .buf = {0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00}
    };

    CAN_message_t written;

    When(Method(mockCan1, write)).Do([&](const CAN_message_t& msg) -> int {
        written = msg;
        return 1;
    });

    can.broadcastClutch();

    When(Method(ArduinoFake(), millis)).AlwaysReturn(50);
    When(Method(mockTransmission, clutchPosition)).AlwaysReturn(0x1122);
    When(Method(mockTransmission, clutchPercentage)).AlwaysReturn(0x3344);

    can.broadcastClutch();

    Verify(Method(mockCan1, write)).Once();
    EXPECT_EQ(written, reference) << "Incorrect 'clutch' message written to CAN interface";
}

TEST_F(CanTest, broadcastClutchFrequency) {
    for(int i=0; i<1000; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        can.broadcastClutch();
    }

    Verify(Method(mockCan1, write)).Exactly(20);
}
#pragma endregion

#pragma region Shift Controller
TEST_F(CanTest, upshift) {
    When(Method(mockCan2, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x65cu,
            .buf = {0b00000001, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        };

        return 1;
    });

    can.update();

    Verify(Method(mockTransmission, shift).Using(Transmission::Direction::UP)).Once();
}

TEST_F(CanTest, downshift) {
    When(Method(mockCan2, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x65cu,
            .buf = {0b00000010, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        };

        return 1;
    });

    can.update();

    Verify(Method(mockTransmission, shift).Using(Transmission::Direction::DOWN)).Once();
}

TEST_F(CanTest, clutchInput) {
    struct Bound {
        int count;
        uint8_t high;
        uint8_t low;
    };

    vector<Bound> bounds = {
        {0, 0xFC, 0x18}, // -10
        {0, 0xFF, 0x9C}, // -1
        {1, 0x00, 0x00}, // 0
        {1, 0x13, 0x88}, // 50
        {1, 0x27, 0x10}, // 100
        {0, 0x2A, 0xF8}, // 110
    };

    for(auto bound : bounds) {
        mockTransmission.ClearInvocationHistory();

        When(Method(mockCan2, read)).Do([&](CAN_message_t &msg) -> int {
            msg = CAN_message_t {
                .id = 0x65cu,
                .buf = {0x00, bound.high, bound.low, 0x00, 0x00, 0x00, 0x00, 0x00}
            };

            return 1;
        });

        can.update();

        Verify(Method(mockTransmission, clutchInput)).Exactly(bound.count);
    }
}

#endif
