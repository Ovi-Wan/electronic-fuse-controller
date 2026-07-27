#ifndef BUS_LOGIC_H
#define BUS_LOGIC_H

#include <stdbool.h>
#include <stdint.h>
#include "fuse_model.h"

typedef struct {
    bool cmd_enable;
    bool cmd_reset;
    uint8_t load_profile;
} bus_command_t;

typedef struct {
    fuse_fault_t fault;
    fuse_state_t state;
    float current;
    float i2t;
} bus_status_t;

float read_current_sensor();
float read_voltage_adc();

void bus_process(fuse_t *f, const bus_command_t *cmd, bus_status_t *status);

#endif
