#include <stdio.h>
#include "fuse_model.h"
#include "bus_logic.h"

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) static void name(void)
#define ASSERT_TRUE(cond) do { \
    tests_run++; \
    if (!(cond)) { \
        tests_failed++; \
        printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)
#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))
#define RUN(t) do { printf("-- %s\n", #t); t(); } while (0)

TEST(test_init_defaults)
{
    fuse_t f;
    fuse_init(&f);
    ASSERT_EQ(f.state, FUSE_STATE_OFF);
    ASSERT_EQ(f.fault, FUSE_FAULT_NONE);
    ASSERT_TRUE(f.i2t_accum == 0.0f);
}

TEST(test_normal_operation_no_fault)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    fuse_update(&f, 5.0f, 12.0f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_NONE);
    ASSERT_EQ(f.state, FUSE_STATE_ON);
}

TEST(test_overcurrent_trips)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    fuse_update(&f, 20.0f, 12.0f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_OVERCURRENT);
    ASSERT_EQ(f.state, FUSE_STATE_FAULT);
}

TEST(test_i2t_overload_trips)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    for (int i = 0; i < 50 && f.fault == FUSE_FAULT_NONE; i++)
        fuse_update(&f, 14.0f, 12.0f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_OVERCURRENT);
    ASSERT_EQ(f.state, FUSE_STATE_FAULT);
}

TEST(test_s2g_trips)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    fuse_update(&f, 8.0f, 0.5f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_S2G);
}

TEST(test_s2b_trips)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    fuse_update(&f, 8.0f, 18.0f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_S2B);
}

TEST(test_open_load_trips_and_persists)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    fuse_update(&f, 0.0f, 12.0f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_OPEN_LOAD);
    ASSERT_EQ(f.state, FUSE_STATE_FAULT);
}

TEST(test_stuck_high_when_off)
{
    fuse_t f;
    fuse_init(&f);
    fuse_update(&f, 0.0f, 12.0f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_STUCK_HIGH);
    ASSERT_EQ(f.state, FUSE_STATE_FAULT);
}

TEST(test_off_no_voltage_no_fault)
{
    fuse_t f;
    fuse_init(&f);
    fuse_update(&f, 0.0f, 0.0f, 0.1f);
    ASSERT_EQ(f.fault, FUSE_FAULT_NONE);
    ASSERT_EQ(f.state, FUSE_STATE_OFF);
}

TEST(test_auto_retry_recovers_after_delay)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    fuse_update(&f, 20.0f, 12.0f, 0.1f);
    ASSERT_EQ(f.state, FUSE_STATE_FAULT);

    for (int i = 0; i < 19; i++)
        fuse_update(&f, 0.0f, 0.0f, 0.1f);
    ASSERT_EQ(f.state, FUSE_STATE_FAULT);

    fuse_update(&f, 0.0f, 0.0f, 0.1f);
    ASSERT_EQ(f.state, FUSE_STATE_ON);
    ASSERT_EQ(f.fault, FUSE_FAULT_NONE);
}

TEST(test_reset_clears_fault)
{
    fuse_t f;
    fuse_init(&f);
    f.state = FUSE_STATE_ON;
    fuse_update(&f, 20.0f, 12.0f, 0.1f);
    ASSERT_EQ(f.state, FUSE_STATE_FAULT);

    bus_command_t cmd = {0};
    cmd.cmd_reset = true;
    bus_status_t status;
    bus_process(&f, &cmd, &status);
    ASSERT_EQ(status.fault, FUSE_FAULT_NONE);
    ASSERT_EQ(status.state, FUSE_STATE_OFF);
}

TEST(test_bus_process_s2g_scenario)
{
    fuse_t f;
    fuse_init(&f);
    bus_command_t cmd = {0};
    cmd.cmd_enable = true;
    cmd.load_profile = 2;
    bus_status_t status;
    bus_process(&f, &cmd, &status);
    ASSERT_EQ(status.fault, FUSE_FAULT_S2G);
}

TEST(test_bus_process_current_zero_when_disabled)
{
    fuse_t f;
    fuse_init(&f);
    bus_command_t cmd = {0};
    cmd.cmd_enable = false;
    cmd.load_profile = 1;
    bus_status_t status;
    bus_process(&f, &cmd, &status);
    ASSERT_TRUE(status.current == 0.0f);
}

int main(void)
{
    RUN(test_init_defaults);
    RUN(test_normal_operation_no_fault);
    RUN(test_overcurrent_trips);
    RUN(test_i2t_overload_trips);
    RUN(test_s2g_trips);
    RUN(test_s2b_trips);
    RUN(test_open_load_trips_and_persists);
    RUN(test_stuck_high_when_off);
    RUN(test_off_no_voltage_no_fault);
    RUN(test_auto_retry_recovers_after_delay);
    RUN(test_reset_clears_fault);
    RUN(test_bus_process_s2g_scenario);
    RUN(test_bus_process_current_zero_when_disabled);

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
