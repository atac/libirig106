
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_index.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_decode_index){
    RUN_TEST_CASE(test_decode_index, TestDecode_FirstIndex);
    RUN_TEST_CASE(test_decode_index, TestDecode_NextIndex);
}


TEST_GROUP(test_decode_index);
TEST_SETUP(test_decode_index){}
TEST_TEAR_DOWN(test_decode_index){}


TEST(test_decode_index, TestDecode_FirstIndex){
    I106C10Header header;
    void * buffer = malloc(32);
    IndexMsg msg;
    IndexCSDW csdw;

    header.DataLength = 32;
    csdw.Count = 4;
    csdw.IndexType = 1;
    memcpy(buffer, &csdw, sizeof(IndexCSDW));

    TEST_ASSERT_EQUAL(I106_INDEX_NODE, I106_Decode_FirstIndex(&header, buffer, &msg));

    free(buffer);
}


TEST(test_decode_index, TestDecode_NextIndex){
    IndexMsg msg;
    IndexCSDW csdw;

    msg.DataLength = 32;
    msg.MessageNumber = 1;
    csdw.Count = 4;
    csdw.IndexType = 1;
    msg.CSDW = &csdw;

    TEST_ASSERT_EQUAL(I106_INDEX_NODE, I106_Decode_NextIndex(&msg));
}
