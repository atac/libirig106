
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(test_1553){
    RUN_TEST_CASE(test_1553, TestDecodeFirst1553F1);
    RUN_TEST_CASE(test_1553, TestDecodeNext1553F1);
    RUN_TEST_CASE(test_1553, TestGetCommandWord);
    RUN_TEST_CASE(test_1553, TestMS1553WordCount);
}
