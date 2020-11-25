

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_1553f1.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_1553){
    RUN_TEST_CASE(test_1553, TestDecodeFirst1553F1);
    RUN_TEST_CASE(test_1553, TestDecodeNext1553F1);
    RUN_TEST_CASE(test_1553, TestGetCommandWord);
    RUN_TEST_CASE(test_1553, TestMS1553WordCount);
}


TEST_GROUP(test_1553);
TEST_SETUP(test_1553){}
TEST_TEAR_DOWN(test_1553){}


TEST(test_1553, TestDecodeFirst1553F1){
    I106C10Header       header;
    I106Status          status;
    char              * buffer;
    MS1553F1_Message    msg;
    MS1553F1_CSDW       csdw;
    MS1553F1_IPH        iph;
    int msg_size = sizeof(MS1553F1_IPH) + 3;

    header.DataLength = sizeof(MS1553F1_CSDW) + (msg_size * 4);
    buffer = malloc(header.DataLength);
    csdw.MessageCount = 4;
    iph.Length = 3;

    memcpy(buffer, (void *)&csdw, sizeof(MS1553F1_CSDW));
    memcpy(buffer + sizeof(MS1553F1_CSDW), (void *)&iph, sizeof(MS1553F1_IPH));

    status = I106_Decode_First1553F1(&header, buffer, &msg);
    TEST_ASSERT_EQUAL(status, I106_OK);
    TEST_ASSERT_EQUAL(3, msg.IPH->Length);

    free(buffer);
}


TEST(test_1553, TestDecodeNext1553F1){
    I106C10Header       header;
    I106Status          status;
    char              * buffer;
    MS1553F1_Message    msg;
    MS1553F1_CSDW       csdw;
    MS1553F1_IPH        iph;
    int msg_size = sizeof(MS1553F1_IPH) + 3;
    int offset = 0;

    header.DataLength = sizeof(MS1553F1_CSDW) + (msg_size * 4);
    buffer = malloc(header.DataLength);
    csdw.MessageCount = 4;
    msg.CSDW = &csdw;
    msg.Offset = 0;
    msg.DataLength = header.DataLength;
    msg.MessageNumber = 0;
    iph.Length = 3;

    memcpy(buffer, (void *)&iph, sizeof(MS1553F1_IPH));
    msg.IPH = (MS1553F1_IPH *)buffer;
    offset = sizeof(MS1553F1_IPH) + iph.Length;

    iph.Length = 4;
    memcpy(buffer + offset, (void *)&iph, sizeof(MS1553F1_IPH));

    status = I106_Decode_Next1553F1(&msg);
    TEST_ASSERT_EQUAL(I106_OK, status);
    TEST_ASSERT_EQUAL(4, msg.IPH->Length);

    free(buffer);
}


TEST(test_1553, TestGetCommandWord){
    CommandWord     cmd;
    unsigned int  * raw = (unsigned int *)&cmd;

    cmd.RT = 12;
    cmd.TR = 0;
    cmd.SubAddress = 20;
    cmd.WordCount = 15;
    TEST_ASSERT_EQUAL_STRING("12-R-20-15", GetCommandWord(*raw));

    cmd.RT = 10;
    cmd.TR = 1;
    cmd.SubAddress = 21;
    cmd.WordCount = 0;
    TEST_ASSERT_EQUAL_STRING("10-T-21-32", GetCommandWord(*raw));
}


TEST(test_1553, TestMS1553WordCount){
    CommandWordUnion cmd;

    // Subaddress 0 or 0x001f should return WordCount & 0x0010
    cmd.CommandWord.SubAddress = 0x0000;
    cmd.CommandWord.WordCount = 17;
    TEST_ASSERT_EQUAL(16, MS1553WordCount((CommandWordUnion *)&cmd));
    cmd.CommandWord.SubAddress = 0x001f;
    TEST_ASSERT_EQUAL(16, MS1553WordCount((CommandWordUnion *)&cmd));

    // Otherwise, should get WordCount (or 32 if WordCount is 0)
    cmd.CommandWord.SubAddress = 0x0010;
    cmd.CommandWord.WordCount = 0;
    TEST_ASSERT_EQUAL(32, MS1553WordCount((CommandWordUnion *)&cmd));
    cmd.CommandWord.WordCount = 24;
    TEST_ASSERT_EQUAL(24, MS1553WordCount((CommandWordUnion *)&cmd));
}
