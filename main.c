#include <stdio.h>
#include "fuse_model.h"
#include "bus_logic.h"

static const char *state_name(fuse_state_t s)
{
    switch (s) {
        case FUSE_STATE_OFF:   return "OFF";
        case FUSE_STATE_ON:    return "ON";
        case FUSE_STATE_FAULT: return "FAULT";
    }
    return "?";
}

static const char *fault_name(fuse_fault_t f)
{
    switch (f) {
        case FUSE_FAULT_NONE:        return "NONE";
        case FUSE_FAULT_OVERCURRENT: return "OVERCURRENT";
        case FUSE_FAULT_S2G:         return "S2G";
        case FUSE_FAULT_S2B:         return "S2B";
        case FUSE_FAULT_OPEN_LOAD:   return "OPEN_LOAD";
        case FUSE_FAULT_STUCK_HIGH:  return "STUCK_HIGH";
    }
    return "?";
}

static void print_menu(const bus_command_t *cmd)
{
    printf("\n=======================================\n");
    printf(" FUSE CONTROL MENU\n");
    printf(" e = toggle ENABLE (currently %s)\n", cmd->cmd_enable ? "ON" : "OFF");
    printf(" r = pulse RESET\n");
    printf(" 0 = load profile NORMAL\n");
    printf(" 1 = load profile HEAVY\n");
    printf(" 2 = load profile SHORT (S2G)\n");
    printf(" h = show this menu\n");
    printf(" q = quit\n");
    printf("=======================================\n");
}

static void print_status(const bus_status_t *status, float t)
{
    printf("[t=%.1fs] state=%-6s fault=%-12s current=%.1fA i2t=%.1f\n",
           t, state_name(status->state), fault_name(status->fault),
           status->current, status->i2t);
}

int main(void)
{
    fuse_t fuse;
    fuse_init(&fuse);

    bus_command_t cmd = {0};
    bus_status_t status;

    float dt = 0.1f;
    float t = 0.0f;

    print_menu(&cmd);
    bus_process(&fuse, &cmd, &status);
    print_status(&status, t);

    int c;
    while ((c = getchar()) != EOF) {
        if (c == '\n') continue;

        switch (c) {
            case 'e':
                cmd.cmd_enable = !cmd.cmd_enable;
                printf("[CMD] enable = %s\n", cmd.cmd_enable ? "ON" : "OFF");
                break;
            case 'r':
                printf("[CMD] reset pulse\n");
                cmd.cmd_reset = true;
                break;
            case '0':
                cmd.load_profile = 0;
                printf("[CMD] load_profile = NORMAL\n");
                break;
            case '1':
                cmd.load_profile = 1;
                printf("[CMD] load_profile = HEAVY\n");
                break;
            case '2':
                cmd.load_profile = 2;
                printf("[CMD] load_profile = SHORT (S2G)\n");
                break;
            case 'h':
                print_menu(&cmd);
                continue;
            case 'q':
                printf("Bye.\n");
                return 0;
            default:
                printf("Comanda necunoscuta. Apasa 'h' pentru meniu.\n");
                continue;
        }

        t += dt;
        bus_process(&fuse, &cmd, &status);
        print_status(&status, t);

        if (cmd.cmd_reset)
            cmd.cmd_reset = false;
    }

    return 0;
}
