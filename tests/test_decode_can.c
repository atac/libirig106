
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_can.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_can){
    RUN_TEST_CASE(test_can, TestDecode_FirstCAN);
    RUN_TEST_CASE(test_can, TestDecode_NextCAN);
}


TEST_GROUP(test_can);
TEST_SETUP(test_can){}
TEST_TEAR_DOWN(test_can){}


TEST(test_can, TestDecode_FirstCAN){
    I106C10Header header;
    void * buffer = malloc(32);
    CAN_Message msg;
    CAN_CSDW csdw;

    csdw.Count = 4;
    memcpy(buffer, &csdw, sizeof(CAN_CSDW));

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_FirstCAN(&header, buffer, &msg));

    free(buffer);
}


TEST(test_can, TestDecode_NextCAN){
    I106C10Header header;
    CAN_Message msg;
    CAN_CSDW csdw;

    csdw.Count = 4;
    header.DataLength = 32;
    msg.BytesRead = 12;
    msg.Header = &header;
    msg.MessageNumber = 1;
    msg.CSDW = &csdw;

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_NextCAN(&msg));
}
