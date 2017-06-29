
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_arinc429.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_arinc429){
    RUN_TEST_CASE(test_arinc429, TestDecode_FirstArinc429F0);
    RUN_TEST_CASE(test_arinc429, TestDecode_NextArinc429F0);
}


TEST_GROUP(test_arinc429);
TEST_SETUP(test_arinc429){}
TEST_TEAR_DOWN(test_arinc429){}


TEST(test_arinc429, TestDecode_FirstArinc429F0){
    I106C10Header header;
    void * buffer = malloc(32);
    Arinc429F0_Message msg;
    Arinc429F0_CSDW csdw;

    csdw.Count = 5;
    memcpy(buffer, &csdw, sizeof(Arinc429F0_CSDW));

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_FirstArinc429F0(&header, buffer, &msg));

    free(buffer);
}


TEST(test_arinc429, TestDecode_NextArinc429F0){
    Arinc429F0_Message msg;
    Arinc429F0_CSDW csdw;
    Arinc429F0_IPH iph;

    csdw.Count = 4;
    msg.CSDW = &csdw;
    msg.IPH = &iph;
    msg.MessageNumber = 1;

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_NextArinc429F0(&msg));
}
