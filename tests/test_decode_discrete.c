

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_discrete.h"
#include "i106_util.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_discrete){
    RUN_TEST_CASE(test_discrete, TestDecode_FirstDiscreteF1);
    RUN_TEST_CASE(test_discrete, TestDecode_NextDiscreteF1);
}


TEST_GROUP(test_discrete);
TEST_SETUP(test_discrete){}
TEST_TEAR_DOWN(test_discrete){}


TEST(test_discrete, TestDecode_FirstDiscreteF1){
    I106C10Header header;
    void * buffer = malloc(32);
    DiscreteF1_Message msg;
    DiscreteF1_CSDW csdw;
    TimeRef time;

    header.DataLength = 32;
    memcpy(buffer, &csdw, sizeof(DiscreteF1_CSDW));

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_FirstDiscreteF1(&header, buffer,
                &msg, &time));

    free(buffer);
}


TEST(test_discrete, TestDecode_NextDiscreteF1){}
