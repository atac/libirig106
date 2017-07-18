
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_analogf1.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_analog){
    RUN_TEST_CASE(test_analog, TestDecode_FirstAnalogF1);
    RUN_TEST_CASE(test_analog, TestDecode_NextAnalogF1);
    RUN_TEST_CASE(test_analog, TestCreateOutputBuffers_AnalogF1);
    RUN_TEST_CASE(test_analog, TestFreeOutputBuffers_AnalogF1);
    /* RUN_TEST_CASE(test_analog, TestPrintCSDW_AnalogF1); */
    /* RUN_TEST_CASE(test_analog, TestPrintAttributesfromTMATS_AnalogF1); */
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
    csdw.Factor = 8;
    csdw.Length = 16;
    csdw.Same = 1;
    memcpy(buffer, &csdw, sizeof(AnalogF1_CSDW));

    header.DataLength = 32;

    attributes.Packed = 1;
    attributes.ChannelsPerPacket = 1;
    attributes.Subchannels[0] = malloc(sizeof(AnalogF1_Subchannel));
    attributes.Subchannels[0]->CSDW = malloc(sizeof(AnalogF1_CSDW));
    memcpy(attributes.Subchannels[0]->CSDW, &csdw, sizeof(AnalogF1_CSDW));

    msg.Attributes = &attributes;
    msg.Length = 4;
    msg.BytesRead = 0;

    status = I106_Decode_FirstAnalogF1(&header, buffer, &msg);
    TEST_ASSERT_EQUAL(I106_Decode_NextAnalogF1(&msg), status);

    free(attributes.Subchannels[0]->CSDW);
    free(attributes.Subchannels[0]);
    free(buffer);
}


TEST(test_analog, TestDecode_NextAnalogF1){
    AnalogF1_Message msg;
    AnalogF1_CSDW csdw;
    AnalogF1_Attributes attributes;
    msg.Attributes = &attributes;
    msg.CSDW = &csdw;

    csdw.Length = 8;

    attributes.ChannelsPerPacket = 2;
    attributes.Packed = 1;

    attributes.Subchannels[0] = malloc(32);
    attributes.Subchannels[0]->CSDW = &csdw;
    attributes.Subchannels[1] = malloc(32);
    attributes.Subchannels[1]->CSDW = &csdw;

    TEST_ASSERT_EQUAL(I106_NO_MORE_DATA, I106_Decode_NextAnalogF1(&msg));

    attributes.ChannelsPerPacket = 1; 
    TEST_ASSERT_EQUAL(I106_NO_MORE_DATA, I106_Decode_NextAnalogF1(&msg));

    csdw.Same = 1;
    TEST_ASSERT_EQUAL(I106_NO_MORE_DATA, I106_Decode_NextAnalogF1(&msg));

    free(attributes.Subchannels[0]);
    free(attributes.Subchannels[1]);
}


TEST(test_analog, TestCreateOutputBuffers_AnalogF1){
    AnalogF1_Attributes attributes;
    uint32_t data_length = 64;

    TEST_ASSERT_EQUAL(I106_OK, CreateOutputBuffers_AnalogF1(&attributes, data_length));
}


TEST(test_analog, TestFreeOutputBuffers_AnalogF1){
    AnalogF1_Attributes attributes;

    attributes.BufferError = NULL;

    // This is kinda cheating (test should only touch FreeOutputBuffers)
    CreateOutputBuffers_AnalogF1(&attributes, 64);

    TEST_ASSERT_EQUAL(I106_OK, FreeOutputBuffers_AnalogF1(&attributes));
}
