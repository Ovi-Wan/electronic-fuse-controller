#include "bus_logic.h"
#include "fuse_model.h"

void bus_process(fuse_t *f, const bus_command_t *cmd, bus_status_t *status)
{
    if (cmd->cmd_enable && f->state != FUSE_STATE_FAULT)
        f->state = FUSE_STATE_ON;
    else if (!cmd->cmd_enable)
        f->state = FUSE_STATE_OFF;

    if (cmd->cmd_reset) {
        f->fault = FUSE_FAULT_NONE;
        f->state = FUSE_STATE_OFF;
        f->i2t_accum = 0.0f;
        f->retry_timer = 0.0f;
    }

    float current = 0.0f;
    float voltage = 0.0f;

    if (f->state == FUSE_STATE_ON) {
        switch (cmd->load_profile) {
            case 0: current = 5.0f;  voltage = 12.0f; break;
            case 1: current = 12.0f; voltage = 12.0f; break;
            case 2: current = 8.0f;  voltage = 0.5f;  break;
            default: current = 0.0f; voltage = 12.0f; break;
        }
    }

    fuse_update(f, current, voltage, 0.1f);

    status->fault  = f->fault;
    status->state  = f->state;
    status->current = current;
    status->i2t = f->i2t_accum;
}
