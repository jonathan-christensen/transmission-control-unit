#ifdef NATIVE

#include <gtest/gtest.h>
#include "CanTest.h"
#include "FiniteStateMachineTest.h"
#include "StorageTest.h"
#include "TransmissionTest.h"
#include "MainTest.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
