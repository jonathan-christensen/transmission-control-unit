#if !defined(MAIN_TEST_H) && defined(NATIVE)
#define MAIN_TEST_H

#include <gtest/gtest.h>
#include <Arduino.h>
#include "../lib/Storage/Storage.h"
#include "../lib/Transmission/Transmission.h"
#include "../lib/Can/Can.h"
#include "mock/MockFlexCAN_T4.h"
#include "mock/MockServo.h"
#include "mock/MockAdafruit_NeoPixel.h"

using namespace fakeit;

extern Mock<Can> mockCan;
extern Mock<Storage> mockStorage;
extern Mock<Adafruit_NeoPixel> mockPixels;
extern Mock<Transmission> mockTransmission;

extern void setup();
extern void loop();

class MainTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Storage
        When(Method(mockStorage, begin)).AlwaysReturn();

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

        // NeoPixel
        When(Method(mockPixels, begin)).AlwaysReturn();
        When(Method(mockPixels, setPixelColor)).AlwaysReturn();
        When(Method(mockPixels, show)).AlwaysReturn();

        // Can
        When(Method(mockCan, begin)).AlwaysReturn();
        When(Method(mockCan, update)).AlwaysReturn();
        When(Method(mockCan, broadcastShiftSettings)).AlwaysReturn();
        When(Method(mockCan, broadcastClutchSettings)).AlwaysReturn();
        When(Method(mockCan, broadcastClutch)).AlwaysReturn();
        
        // Transmission
        When(Method(mockTransmission, begin)).AlwaysReturn();
        When(Method(mockTransmission, shift)).AlwaysReturn();
        When(Method(mockTransmission, clutchInput)).AlwaysReturn();
        When(Method(mockTransmission, update)).AlwaysReturn();
        When(Method(mockTransmission, setRpm)).AlwaysReturn();

        When(Method(ArduinoFake(), millis)).AlwaysReturn(0);

        setup();
    }

    void TearDown() override {
        ArduinoFakeReset();
        mockStorage.ClearInvocationHistory();
        mockCan.ClearInvocationHistory();
        mockPixels.ClearInvocationHistory();
        mockTransmission.ClearInvocationHistory();
    }
};

TEST_F(MainTest, StorageBegin) {
    Verify(Method(mockStorage, begin)).Once();
}

TEST_F(MainTest, NeoPixel) {
    Verify(Method(mockPixels, begin)).Once();
    Verify(Method(mockPixels, setPixelColor).Using(0, Adafruit_NeoPixel::Color(0, 140, 0))).Once();
    Verify(Method(mockPixels, show)).Once();
}

TEST_F(MainTest, CanBegin) {
    Verify(Method(mockCan, begin)).Once();
}

TEST_F(MainTest, TransmissionUpdate) {
    loop();
    Verify(Method(mockTransmission, update)).Once();
}

TEST_F(MainTest, canLoop) {
    loop();
    Verify(Method(mockCan, update)).Once();
    Verify(Method(mockCan, broadcastShiftSettings)).Once();
    Verify(Method(mockCan, broadcastClutchSettings)).Once();
    Verify(Method(mockCan, broadcastClutch)).Once();
}

#endif
