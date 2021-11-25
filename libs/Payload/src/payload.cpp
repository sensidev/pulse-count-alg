#include "payload.h"
#include <cmath>

static double round_to_decimals(double value);


Payload::Payload() {
    ring_buffer_init(&this->ring_buffer);
    this->pulses = 0;
    this->pulses_avg = 0;
    this->avg = 0;
    this->rms = 0;
    this->min = INT32_MAX;
    this->max = INT32_MIN;
    this->data_length = 0;
    this->last_data_length = 0;
    this->pulse_threshold = 50;
    this->pulse_length = 8;
    this->is_pulse_started = false;
}

void Payload::add(uint8_t value) {
    ring_buffer_queue(&this->ring_buffer, value);
    this->data_length += 1;
}

void Payload::reset() {
    ring_buffer_init(&this->ring_buffer);
}

uint16_t Payload::get_data_length() {
    return this->data_length;
}

uint16_t Payload::get_agg_length() {
    return PAYLOAD_AGG_LENGTH;
}

uint16_t Payload::consume() {
    ring_buffer_size_t cnt = ring_buffer_dequeue_arr(&this->ring_buffer, (char *) this->data, this->data_length);
    this->last_data_length = cnt;
    this->data_length = 0;
    return cnt;
}

const uint8_t *Payload::get_data() {
    return (const uint8_t *) this->data;
}

const uint8_t *Payload::get_agg() {
    int16_t pulses_avg_multiplied = this->pulses_avg * ADC_VALUE_MULTIPLIER;
    int16_t avg_multiplied = this->avg * ADC_VALUE_MULTIPLIER;
    uint16_t rms_multiplied = this->rms * ADC_VALUE_MULTIPLIER;
    int16_t min_multiplied = this->min * ADC_VALUE_MULTIPLIER;
    int16_t max_multiplied = this->max * ADC_VALUE_MULTIPLIER;

    // u16 pulse, i16 pulses avg, i16 avg, u16 rms, i16 min, i16 max.
    this->agg[0] = this->pulses >> 8u;
    this->agg[1] = this->pulses & 0xFFu;
    this->agg[2] = pulses_avg_multiplied >> 8u;
    this->agg[3] = pulses_avg_multiplied & 0xFFu;
    this->agg[4] = avg_multiplied >> 8u;
    this->agg[5] = avg_multiplied & 0xFFu;
    this->agg[6] = rms_multiplied >> 8u;
    this->agg[7] = rms_multiplied & 0xFFu;
    this->agg[8] = min_multiplied >> 8u;
    this->agg[9] = min_multiplied & 0xFFu;
    this->agg[10] = max_multiplied >> 8u;
    this->agg[11] = max_multiplied & 0xFFu;

    return this->agg;
}

void Payload::compute_aggregates() {
    uint16_t values_count = 0;
    uint16_t pulse_values_count = 0;
    double sum_of_squares = 0;
    double sum = 0;
    uint16_t current_pulse_length = 0;

    this->is_pulse_started = false;
    this->pulses = 0;
    this->pulses_avg = 0;
    this->rms = 0;
    this->avg = 0;
    this->min = INT32_MAX;
    this->max = INT32_MIN;

    for (uint16_t i = 0; i < this->last_data_length; i += PAYLOAD_DATA_ITEM_LENGTH) {
        uint8_t delta = this->data[i];
        uint16_t raw_value_msb = this->data[i + 1] << 8u;
        uint16_t raw_value_lsb = this->data[i + 2] & 0xFFu;
        int16_t raw_value = (raw_value_msb | raw_value_lsb);
        double value = (double) raw_value / ADC_VALUE_MULTIPLIER;

        start_new_pulse(value);

        if (this->is_pulse_started) {
            if (current_pulse_length > this->pulse_length) {
                current_pulse_length = 0;
                this->is_pulse_started = false;
                start_new_pulse(value);
            } else {
                current_pulse_length += delta;
                this->pulses_avg += value;
                pulse_values_count++;
            }
        }

        sum += value;
        sum_of_squares += value * value;

        if (value < this->min) this->min = value;
        if (value > this->max) this->max = value;

        values_count++;
    }

    this->avg = sum / values_count;
    this->rms = sqrt(sum_of_squares / (double) values_count);

    if (pulse_values_count > 0) {
        this->pulses_avg = this->pulses_avg / (double) pulse_values_count;
    }
}

uint16_t Payload::get_pulses() {
    return this->pulses;
}

double Payload::get_pulses_avg() {
    return round_to_decimals(this->pulses_avg);
}

double Payload::get_avg() {
    return round_to_decimals(this->avg);
}

double Payload::get_rms() {
    return round_to_decimals(this->rms);
}

double Payload::get_min() {
    return round_to_decimals(this->min);
}

double Payload::get_max() {
    return round_to_decimals(this->max);
}


int16_t Payload::get_value_from_adc_raw_value(uint16_t adc_raw_value, uint16_t offset) {
    return round(((adc_raw_value - ADC_VALUE_ZERO - offset) * ADC_VALUE_STEP) * ADC_VALUE_MULTIPLIER);
}

uint16_t Payload::get_adc_raw_value_from_value(int16_t value, uint16_t offset) {
    return round((((double) value / ADC_VALUE_MULTIPLIER) / ADC_VALUE_STEP + offset + ADC_VALUE_ZERO));
}

uint16_t Payload::get_adc_offset() {
    int16_t value = this->get_avg() * ADC_VALUE_MULTIPLIER;
    uint16_t offset = this->get_adc_raw_value_from_value(value, 0);
    return offset - ADC_VALUE_ZERO;
}

void Payload::set_pulse_threshold(uint8_t threshold) {
    this->pulse_threshold = threshold;
}

void Payload::set_pulse_length(uint8_t length) {
    this->pulse_length = length;
}

void Payload::start_new_pulse(double value) {
    if (value > this->pulse_threshold) {
        if (!this->is_pulse_started) {
            is_pulse_started = true;
            this->pulses++;
        }
    }
}

static double round_to_decimals(double value) {
    int64_t x = value * ADC_VALUE_MULTIPLIER;
    return (double) x / ADC_VALUE_MULTIPLIER;
}

