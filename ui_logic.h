#ifndef UI_LOGIC_H
#define UI_LOGIC_H

#include "bus_logic.h"

void ui_update_leds(const bus_status_t *status);
void fuse_set_color(int r, int g, int b);

#endif
