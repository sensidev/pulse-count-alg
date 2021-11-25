#include <unity.h>
#include <payload.h>
#include <iostream>

#define ADC_VALUE_MULTIPLIER 100

Payload *p;


const uint16_t adc_offset = 0;

double data_set1[] = {6.713, 7.373, 2, 1, 0, 1, 2.2, 33.44, 5.4, -1, -2, 90.65};
double data_set2[] = {1.84, 1.80, 1.83, 1.82, 1.81};
double data_set3[] = {0.12, 0.16, -0.1, -0.09, 0, 1.1, -0.12};

double data_set_pulse_one_peak[] = {
        8.23, 9.56, 12.66, 50.55, 59.69, 60.21, 60.21, 57.33, 45.22, 31.11, 22.11, 11.11, 10.19, 5, 2, 1, -9, -6
};
double data_set_pulse_sine[] = {
        -6.34, -3.06, 3.75, 6.17, -0.21, -5.96, -2.31, 4.44, 6.78, -0.98, -6.26, -3.68, 5.09, 6.5, 0.58, -6.34, -3, 3.8
};

void setUp(void) {
    p = new Payload();
}

void tearDown(void) {
    delete p;
};

void print_buffer(const uint8_t *data, uint16_t length);

void prepare_values(const double *data_set, uint16_t len, uint8_t delta) {
    for (uint16_t i = 0; i < len; i++) {
        p->add(delta);
        auto value = (int16_t) (data_set[i] * ADC_VALUE_MULTIPLIER);
        p->add(((uint16_t) value) >> 8u);
        p->add(((uint16_t) value) & 0xFFu);
    }
}

void print_buffer(const uint8_t *data, uint16_t length) {
    printf("Buffer: ");
    for (uint16_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}

void test_get_value_from_adc_raw_value() {
    prepare_values(data_set1, 12, 4);
    p->consume();
    p->compute_aggregates();

    TEST_ASSERT_EQUAL(-10638, p->get_value_from_adc_raw_value(0, adc_offset));
    TEST_ASSERT_EQUAL(0, p->get_value_from_adc_raw_value(1650, adc_offset));
    TEST_ASSERT_EQUAL(10638, p->get_value_from_adc_raw_value(3300, adc_offset));
    TEST_ASSERT_EQUAL(-4191, p->get_value_from_adc_raw_value(1000, adc_offset));
    TEST_ASSERT_EQUAL(-5319, p->get_value_from_adc_raw_value(825, adc_offset));
    TEST_ASSERT_EQUAL(5480, p->get_value_from_adc_raw_value(2500, adc_offset));
}

void test_get_adc_raw_value_from_value() {
    prepare_values(data_set1, 12, 4);
    p->consume();
    p->compute_aggregates();

    TEST_ASSERT_EQUAL(0, p->get_adc_raw_value_from_value(-10638, adc_offset));
    TEST_ASSERT_EQUAL(1650, p->get_adc_raw_value_from_value(0, adc_offset));
    TEST_ASSERT_EQUAL(3300, p->get_adc_raw_value_from_value(10638, adc_offset));
    TEST_ASSERT_EQUAL(1000, p->get_adc_raw_value_from_value(-4191, adc_offset));
    TEST_ASSERT_EQUAL(825, p->get_adc_raw_value_from_value(-5319, adc_offset));
    TEST_ASSERT_EQUAL(2500, p->get_adc_raw_value_from_value(5480, adc_offset));
}

void test_get_avg() {
    prepare_values(data_set1, 12, 4);
    p->consume();
    p->compute_aggregates();

    printf("AVG: %.2f\r\n", p->get_avg());
    TEST_ASSERT_EQUAL(12.23, p->get_avg());
}

void test_get_rms() {
    prepare_values(data_set1, 12, 4);
    p->consume();
    p->compute_aggregates();

    printf("RMS: %.2f\r\n", p->get_rms());
    TEST_ASSERT_EQUAL(28.1, p->get_rms());
}

void test_get_min() {
    prepare_values(data_set1, 12, 4);
    p->consume();
    p->compute_aggregates();

    printf("MIN: %.2f\r\n", p->get_min());
    TEST_ASSERT_EQUAL(-2, p->get_min());
}

void test_get_max() {
    prepare_values(data_set1, 12, 4);
    p->consume();
    p->compute_aggregates();

    printf("MAX: %.2f\r\n", p->get_max());
    TEST_ASSERT_EQUAL(90.65, p->get_max());
}

void test_get_agg_zero_pulses() {
    prepare_values(data_set3, 6, 4);
    p->consume();
    p->compute_aggregates();

    const uint8_t *agg = p->get_agg();
    print_buffer(agg, p->get_agg_length());
    TEST_ASSERT_EQUAL(p->get_pulses(), ((agg[0] << 8u) | (agg[1] & 0xFFu)));
    TEST_ASSERT_EQUAL(p->get_pulses_avg(),
                      (int16_t) ((agg[2] << 8u) | (agg[3] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_avg(), (int16_t) ((agg[4] << 8u) | (agg[5] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_rms(), ((agg[6] << 8u) | (agg[7] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_min(), (int16_t) ((agg[8] << 8u) | (agg[9] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_max(), (int16_t) ((agg[10] << 8u) | (agg[11] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
}

void test_get_agg_two_pulses() {
    prepare_values(data_set_pulse_one_peak, 18, 4);
    p->set_pulse_threshold(50);
    p->set_pulse_length(8);
    p->consume();
    p->compute_aggregates();

    const uint8_t *agg = p->get_agg();
    print_buffer(agg, p->get_agg_length());
    TEST_ASSERT_EQUAL(p->get_pulses(), ((agg[0] << 8u) | (agg[1] & 0xFFu)));
    TEST_ASSERT_EQUAL(p->get_pulses_avg(),
                      (int16_t) ((agg[2] << 8u) | (agg[3] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_avg(), (int16_t) ((agg[4] << 8u) | (agg[5] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_rms(), ((agg[6] << 8u) | (agg[7] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_min(), (int16_t) ((agg[8] << 8u) | (agg[9] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
    TEST_ASSERT_EQUAL(p->get_max(), (int16_t) ((agg[10] << 8u) | (agg[11] & 0xFFu)) / (double) ADC_VALUE_MULTIPLIER);
}

void test_get_adc_offset() {
    prepare_values(data_set2, 5, 4);
    p->consume();
    p->compute_aggregates();

    printf("OFFSET: %d\r\n", p->get_adc_offset());
    TEST_ASSERT_EQUAL(28, p->get_adc_offset());
}

void test_get_pulses_one_peak_1() {
    prepare_values(data_set_pulse_one_peak, 18, 4);
    p->set_pulse_threshold(50);
    p->set_pulse_length(16);
    p->consume();
    p->compute_aggregates();

    printf("Pulse one peak 1: %d\r\n", p->get_pulses());
    TEST_ASSERT_EQUAL(1, p->get_pulses());
}

void test_get_pulses_one_peak_2() {
    prepare_values(data_set_pulse_one_peak, 18, 4);
    p->set_pulse_threshold(50);
    p->set_pulse_length(8);
    p->consume();
    p->compute_aggregates();

    printf("Pulse one peak 2: %d\r\n", p->get_pulses());
    TEST_ASSERT_EQUAL(2, p->get_pulses());
}

void test_get_pulses_one_peak_3() {
    prepare_values(data_set_pulse_one_peak, 18, 4);
    p->set_pulse_threshold(50);
    p->set_pulse_length(2);
    p->consume();
    p->compute_aggregates();

    printf("Pulse one peak 3: %d\r\n", p->get_pulses());
    TEST_ASSERT_EQUAL(3, p->get_pulses());
}

void test_get_pulses_sine_1() {
    prepare_values(data_set_pulse_sine, 18, 4);
    p->set_pulse_threshold(6);
    p->set_pulse_length(40);
    p->consume();
    p->compute_aggregates();

    printf("Pulse sine 1: %d\r\n", p->get_pulses());
    TEST_ASSERT_EQUAL(1, p->get_pulses());
}

void test_get_pulses_sine_2() {
    prepare_values(data_set_pulse_sine, 18, 4);
    p->set_pulse_threshold(6);
    p->set_pulse_length(20);
    p->consume();
    p->compute_aggregates();

    printf("Pulse sine 2: %d\r\n", p->get_pulses());
    TEST_ASSERT_EQUAL(2, p->get_pulses());
}

void test_get_pulses_sine_3() {
    prepare_values(data_set_pulse_sine, 18, 4);
    p->set_pulse_threshold(6);
    p->set_pulse_length(4);
    p->consume();
    p->compute_aggregates();

    printf("Pulse sine 3: %d\r\n", p->get_pulses());
    TEST_ASSERT_EQUAL(3, p->get_pulses());
}

void test_get_pulses_avg() {
    prepare_values(data_set_pulse_one_peak, 18, 4);
    p->set_pulse_threshold(50);
    p->set_pulse_length(16);
    p->consume();
    p->compute_aggregates();

    printf("Pulse avg: %.02f\r\n", p->get_pulses_avg());
    TEST_ASSERT_EQUAL(57.59, p->get_pulses_avg());
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_get_pulses_avg);

    RUN_TEST(test_get_pulses_sine_1);
    RUN_TEST(test_get_pulses_sine_2);
    RUN_TEST(test_get_pulses_sine_3);

    RUN_TEST(test_get_pulses_one_peak_1);
    RUN_TEST(test_get_pulses_one_peak_2);
    RUN_TEST(test_get_pulses_one_peak_3);

    RUN_TEST(test_get_value_from_adc_raw_value);
    RUN_TEST(test_get_adc_raw_value_from_value);

    RUN_TEST(test_get_rms);
    RUN_TEST(test_get_avg);
    RUN_TEST(test_get_min);
    RUN_TEST(test_get_max);
    RUN_TEST(test_get_agg_zero_pulses);
    RUN_TEST(test_get_agg_two_pulses);

    RUN_TEST(test_get_adc_offset);

    return UNITY_END();
}