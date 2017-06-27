
#include <stdint.h>
#include "i106_decode_1553f1.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(test_1553);
TEST_SETUP(test_1553){}
TEST_TEAR_DOWN(test_1553){}


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
