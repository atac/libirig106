
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_tmats.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_decode_tmats){
    RUN_TEST_CASE(test_decode_tmats, Test_Decode_TMATS);
    RUN_TEST_CASE(test_decode_tmats, Test_Decode_TMATS_Text);
    RUN_TEST_CASE(test_decode_tmats, Test_Encode_TMATS);
    RUN_TEST_CASE(test_decode_tmats, Test_TMATS_Signature);
}


TEST_GROUP(test_decode_tmats);
TEST_SETUP(test_decode_tmats){}
TEST_TEAR_DOWN(test_decode_tmats){}


TEST(test_decode_tmats, Test_Decode_TMATS){
    I106C10Header header;
    char * buffer = malloc(32);
    TMATS_Info tmats;

    header.DataLength = 0;
    memset(&tmats, 0, sizeof(TMATS_Info));
    
    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_TMATS(&header, buffer, &tmats));

    free(buffer);
}


TEST(test_decode_tmats, Test_Decode_TMATS_Text){
    char * buffer = malloc(32);
    TMATS_Info tmats;

    memset(&tmats, 0, sizeof(TMATS_Info));

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_TMATS_Text(buffer, 0, &tmats));

    free(buffer);
}


TEST(test_decode_tmats, Test_Encode_TMATS){
    I106C10Header header;
    char * buffer = malloc(32);
    char * tmats = "Testing";

    TEST_ASSERT_EQUAL(I106_OK, I106_Encode_TMATS(&header, buffer, tmats));
    TEST_ASSERT_EQUAL_STRING((char *)buffer + 4, tmats);

    free(buffer);
}


TEST(test_decode_tmats, Test_TMATS_Signature){
    char * buffer = malloc(32);
    uint16_t opcode;
    uint32_t signature;

    TEST_ASSERT_EQUAL(I106_OK, I106_TMATS_Signature(&buffer, 0, 0, 0, &opcode,
                &signature));

    free(buffer);
}
