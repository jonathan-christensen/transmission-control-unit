#ifndef CAN_H
#define CAN_H

#ifdef NATIVE
#include "../../test/mock/MockFlexCAN_T4.h"
#else
#include <FlexCAN_T4.h>
#endif

#include <Arduino.h>
#include <functional>
#include "../Storage/Storage.h"
#include "../Transmission/Transmission.h"
#include <r3.h>
#include <tcs.h>

class Can {
  public:
    FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>& can1;
    FlexCAN_T4<CAN2, RX_SIZE_16, TX_SIZE_16>& can2;
    Storage& storage;
    Transmission& transmission;
    
    Can(
        FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>& can1Ref,
        FlexCAN_T4<CAN2, RX_SIZE_16, TX_SIZE_16>& can2Ref,
        Storage& storageRef,
        Transmission& transmissionRef
    ) : 
        can1(can1Ref),
        can2(can2Ref),
        storage(storageRef),
        transmission(transmissionRef) {
    }
    
    virtual void begin();
    virtual void update();

    virtual void broadcastShiftSettings();
    virtual void broadcastClutchSettings();
    virtual void broadcastClutch();

  private:
    void handleGroup0(const CAN_message_t &msg);
    void handleShiftSettings(const CAN_message_t &msg);
    void handleClutchSettings(const CAN_message_t &msg);
    void handleSetClutch(const CAN_message_t &msg);
    void handleShiftController(const CAN_message_t &msg);

    unsigned long lastEcuUpdate = 0;
};

#endif
