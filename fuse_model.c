#include "fuse_model.h"
#include <stdbool.h>

void fuse_init(fuse_t *f)
{
    if (!f) return;

    f->current_nominal = 10.0f;
    f->current_limit = 15.0f;
    f->i2t_threshold = 200.0f;

    f->i2t_accum = 0.0f;

    f->state = FUSE_STATE_OFF;
    f->fault = FUSE_FAULT_NONE;

    f->auto_retry_enabled = true;
    f->retry_delay = 2.0f;
    f->retry_timer = 0.0f;

    f->cooldown_rate = 10.0f;
}

void fuse_update(fuse_t *f, float current, float voltage, float dt)
{
    if (!f) return;

    if (f->state == FUSE_STATE_FAULT) {
        if (f->i2t_accum > 0.0f) {
            f->i2t_accum -= f->cooldown_rate * dt;
            if (f->i2t_accum < 0.0f)
                f->i2t_accum = 0.0f;
        }

        if (f->auto_retry_enabled) {
            f->retry_timer += dt;
            if (f->retry_timer >= f->retry_delay) {
                f->retry_timer = 0.0f;
                f->state = FUSE_STATE_ON;
                f->fault = FUSE_FAULT_NONE;
            }
        }
        return;
    }

    if (f->i2t_accum > 0.0f) {
        f->i2t_accum -= f->cooldown_rate * dt;
        if (f->i2t_accum < 0.0f)
            f->i2t_accum = 0.0f;
    }

    if (f->state == FUSE_STATE_OFF) {
        if (voltage > 10.0f) {
            f->fault = FUSE_FAULT_STUCK_HIGH;
            f->state = FUSE_STATE_FAULT;
        }
        return;
    }

    if (current > f->current_limit) {
        f->fault = FUSE_FAULT_OVERCURRENT;
        f->state = FUSE_STATE_FAULT;
        return;
    }

    f->i2t_accum += current * current * dt;
    if (f->i2t_accum > f->i2t_threshold) {
        f->fault = FUSE_FAULT_OVERCURRENT;
        f->state = FUSE_STATE_FAULT;
        return;
    }

    if (voltage < 1.0f && current > (f->current_nominal * 0.5f)) {
        f->fault = FUSE_FAULT_S2G;
        f->state = FUSE_STATE_FAULT;
        return;
    }

    if (voltage > 16.0f && current > (f->current_nominal * 0.5f)) {
        f->fault = FUSE_FAULT_S2B;
        f->state = FUSE_STATE_FAULT;
        return;
    }

    if (current < 0.1f) {
        f->fault = FUSE_FAULT_OPEN_LOAD;
        f->state = FUSE_STATE_FAULT;
        return;
    }

    f->fault = FUSE_FAULT_NONE;
}
