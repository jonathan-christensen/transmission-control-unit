#include <Arduino.h>
#include "../lib/Storage/Storage.h"
#include "../lib/Transmission/Transmission.h"
#include "../lib/Can/Can.h"

#ifdef NATIVE
    #include "../test/mock/MockAdafruit_NeoPixel.h"

    using namespace fakeit;

    Mock<Can> mockCan;
    Mock<Storage> mockStorage;
    Mock<Adafruit_NeoPixel> mockPixels;
    Mock<Transmission> mockTransmission;

    Storage& storage = mockStorage.get();
    Adafruit_NeoPixel& pixels = mockPixels.get();
    Can& can = mockCan.get();
    Transmission& transmission = mockTransmission.get();
#else
    #include <FlexCAN_T4.h>
    #include <Servo.h>
    #include <Adafruit_NeoPixel.h>

    Storage storage;
    FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16> can1;
    FlexCAN_T4<CAN2, RX_SIZE_16, TX_SIZE_16> can2;
    Servo servo;
    Adafruit_NeoPixel pixels(1, 5, NEO_GRB + NEO_KHZ800);
    Transmission transmission(storage, servo);
    Can can(can1, can2, storage, transmission);
#endif

void setup() {
    storage.begin();

    pixels.begin();
    pixels.setPixelColor(0, pixels.Color(0, 140, 0));
    pixels.show();

    can.begin();

    transmission.begin();
}

void loop() {
    // Read CAN
    can.update();

    transmission.update();
    
    // Send data over CAN
    can.broadcastShiftSettings();
    can.broadcastClutchSettings();
    can.broadcastClutch();
}
