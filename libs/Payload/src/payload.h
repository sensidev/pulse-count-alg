#ifndef ESP32_PULSE_POC_PAYLOAD_H
#define ESP32_PULSE_POC_PAYLOAD_H

#include <cstdint>
#include "ringbuffer.h"

#define PAYLOAD_DATA_MAX_LENGTH 1024
#define PAYLOAD_DATA_ITEM_LENGTH 3  // 0: delta, 1:MSB value, 2:LSB value
#define PAYLOAD_AGG_LENGTH 2 * 6  // u16 pulse, i16 pulses avg, i16 avg, u16 rms, i16 min, i16 max.

#define ADC_VALUE_MULTIPLIER 100
#define ADC_VALUE_ZERO 13200
#define ADC_VALUE_STEP 0.01136370882

/**
 * Payload manager responsible with data handling and aggregation.
 */
class Payload {
public:
    Payload();

    void add(uint8_t value);

    void reset();

    void set_pulse_threshold(uint8_t threshold);

    void set_pulse_length(uint8_t length);

    uint16_t consume();

    void compute_aggregates();

    const uint8_t *get_data();

    const uint8_t *get_agg();

    uint16_t get_data_length();

    uint16_t get_agg_length();

    uint16_t get_pulses();

    double get_pulses_avg();

    double get_avg();

    double get_rms();

    double get_min();

    double get_max();

    int16_t get_value_from_adc_raw_value(uint16_t adc_raw_value, uint16_t offset);

    uint16_t get_adc_raw_value_from_value(int16_t value, uint16_t offset);

    uint16_t get_adc_offset();

protected:
    uint16_t pulses;
    double pulses_avg;
    double avg;
    double rms;
    double min;
    double max;

    uint8_t pulse_threshold;
    uint8_t pulse_length;

    bool is_pulse_started;

    void start_new_pulse(double value);

private:
    ring_buffer_t ring_buffer;
    volatile uint8_t data[3 * PAYLOAD_DATA_MAX_LENGTH] = {};
    volatile uint16_t data_length;
    uint16_t last_data_length;
    uint8_t agg[PAYLOAD_AGG_LENGTH];

};


#endif
