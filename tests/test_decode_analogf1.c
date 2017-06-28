
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_analogf1.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_analog){
    RUN_TEST_CASE(test_analog, TestDecode_FirstAnalogF1);
}


TEST_GROUP(test_analog);
TEST_SETUP(test_analog){}
TEST_TEAR_DOWN(test_analog){}


TEST(test_analog, TestDecode_FirstAnalogF1){
    I106Status status;
    I106C10Header header;
    AnalogF1_Message msg;
    AnalogF1_Attributes attributes;
    AnalogF1_CSDW csdw;
    void * buffer = malloc(32);

    csdw.Subchannels = 1;
    csdw.Mode = ANALOG_PACKED;
    memcpy(buffer, &csdw, sizeof(AnalogF1_CSDW));

    header.DataLength = 32;

    attributes.ChannelsPerPacket = 1;
    attributes.Subchannels[0] = malloc(sizeof(AnalogF1_Subchannel));
    attributes.Subchannels[0]->CSDW = malloc(sizeof(AnalogF1_CSDW));
    memcpy(attributes.Subchannels[0]->CSDW, &csdw, sizeof(AnalogF1_CSDW));

    msg.Attributes = &attributes;
    msg.BytesRead = 0;

    status = I106_Decode_FirstAnalogF1(&header, buffer, &msg);
    TEST_ASSERT_EQUAL(DecodeBuffer_AnalogF1(&msg), status);

    free(attributes.Subchannels[0]->CSDW);
    free(attributes.Subchannels[0]);
    free(buffer);
}
