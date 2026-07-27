#ifndef FUSE_MODEL_H
#define FUSE_MODEL_H
#include <stdbool.h>

typedef enum {
    FUSE_STATE_OFF,
    FUSE_STATE_ON,
    FUSE_STATE_FAULT,
} fuse_state_t;

typedef enum {
    FUSE_FAULT_NONE,
    FUSE_FAULT_OVERCURRENT,
    FUSE_FAULT_S2G,
    FUSE_FAULT_S2B,
    FUSE_FAULT_OPEN_LOAD,
    FUSE_FAULT_STUCK_HIGH
} fuse_fault_t;

typedef struct {
    float current_nominal;   // A
    float current_limit;     // A
    float i2t_threshold;     // A^2 * s

    float i2t_accum;         // A^2 * s
    fuse_state_t state;
    fuse_fault_t fault;

    bool auto_retry_enabled;
    float retry_delay;
    float retry_timer;

    float cooldown_rate;

} fuse_t;

void fuse_init(fuse_t *f);
void fuse_update(fuse_t *f, float current, float voltage, float dt);

#endif
