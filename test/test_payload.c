#include <unity.h>

void setUp(void){};
void tearDown(void){};

void test_my_method(void) {
//    uchar result = my_method();
    // assertion and other logic would go here
    TEST_ASSERT_EQUAL(1, 1);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_my_method);

    return UNITY_END();
}