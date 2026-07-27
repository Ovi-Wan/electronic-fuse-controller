#include "ui_logic.h"
#include "bus_logic.h"
#include <Arduino.h>

void ui_update_leds(const bus_status_t *status)
{
    switch (status->fault) {

        case FUSE_FAULT_NONE:
            fuse_set_color(0, 1, 0);
            break;

        case FUSE_FAULT_OVERCURRENT:
            fuse_set_color(1, 0, 0);
            break;

        case FUSE_FAULT_S2G:
            fuse_set_color(0, 0, 1);
            break;

        case FUSE_FAULT_S2B:
            fuse_set_color(1, 0, 1);
            break;

        case FUSE_FAULT_OPEN_LOAD:
            fuse_set_color(1, 1, 1);
            break;

        case FUSE_FAULT_STUCK_HIGH:
            fuse_set_color(1, 1, 0);
            break;
    }
}
