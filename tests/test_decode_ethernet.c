
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_ethernet.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_ethernet){
    RUN_TEST_CASE(test_ethernet, TestDecode_FirstEthernetF0);
    RUN_TEST_CASE(test_ethernet, TestDecode_NextEthernetF0);
}


TEST_GROUP(test_ethernet);
TEST_SETUP(test_ethernet){}
TEST_TEAR_DOWN(test_ethernet){}


TEST(test_ethernet, TestDecode_FirstEthernetF0){
    I106C10Header header;
    char * buffer = malloc(100);
    EthernetF0_Message msg;
    EthernetF0_CSDW csdw;
    csdw.Frames = 10;
    memcpy(buffer, &csdw, sizeof(EthernetF0_CSDW));

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_FirstEthernetF0(&header, buffer, &msg));

    free(buffer);
}


TEST(test_ethernet, TestDecode_NextEthernetF0){
    EthernetF0_Message msg;
    EthernetF0_IPH iph;
    EthernetF0_CSDW csdw;

    csdw.Frames = 4;
    msg.CSDW = &csdw;
    msg.IPH = &iph;
    msg.FrameNumber = 1;

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_NextEthernetF0(&msg));
}
