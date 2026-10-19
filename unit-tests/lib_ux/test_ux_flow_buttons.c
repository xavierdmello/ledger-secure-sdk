#include <stdbool.h>
#include <string.h>

#include "unity.h"

#include "os.h"
#include "ux.h"

/**
 * Stubs
 */

ux_state_t        G_ux;
bolos_ux_params_t G_ux_params;

void bagl_draw_with_context(const bagl_component_t *component,
                            const void             *text,
                            unsigned short          text_length,
                            unsigned char           text_encoding)
{
    (void) component;
    (void) text;
    (void) text_length;
    (void) text_encoding;
}

void bagl_draw_glyph(const bagl_component_t *component, const bagl_icon_details_t *icon_details)
{
    (void) component;
    (void) icon_details;
}

unsigned int os_io_handle_ux_event_reject_apdu(void)
{
    return 0;
}

bolos_task_status_t os_sched_last_status(unsigned int task_idx)
{
    (void) task_idx;
    return 0;
}

unsigned int os_ux(bolos_ux_params_t *params)
{
    (void) params;
    return 0;
}

void ux_stack_redisplay(void) {}

/**
 * Test flows
 */

static const ux_flow_step_t *displayed_step;
static unsigned int          nb_approvals;

static void step_init(unsigned int stack_slot)
{
    (void) stack_slot;
    displayed_step = ux_flow_get_current();
}

static void approve_init(unsigned int stack_slot)
{
    (void) stack_slot;
    nb_approvals++;
}

static const ux_flow_step_t        step_approved   = {approve_init, NULL, NULL, NULL};
static const ux_flow_step_t *const flow_approved[] = {&step_approved, FLOW_END_STEP};

// screen displayed before the host request
static const ux_flow_step_t        step_home   = {step_init, NULL, NULL, NULL};
static const ux_flow_step_t        step_about  = {step_init, NULL, NULL, NULL};
static const ux_flow_step_t *const flow_home[] = {&step_home, &step_about, FLOW_END_STEP};

// review flow displayed on host request
static const ux_flow_step_t        step_review   = {step_init, NULL, NULL, NULL};
static const ux_flow_step_t        step_approve  = {step_init, NULL, flow_approved, NULL};
static const ux_flow_step_t *const flow_review[] = {&step_review, &step_approve, FLOW_END_STEP};

static void buttons(unsigned int mask)
{
    io_seproxyhal_button_push(ux_flow_button_callback, mask);
}

static void click(unsigned int mask)
{
    buttons(mask);
    buttons(0);
}

void setUp(void)
{
    memset(&G_ux, 0, sizeof(G_ux));
    memset(&G_ux_os, 0, sizeof(G_ux_os));
    G_ux.stack_count = 1;
    displayed_step   = NULL;
    nb_approvals     = 0;
    ux_flow_init(0, flow_home, NULL);
}

void tearDown(void) {}

/**
 * Tests
 */

// gestures started and ended on the same screen still work
static void test_fresh_gestures(void)
{
    click(BUTTON_RIGHT);
    TEST_ASSERT_EQUAL_PTR(&step_about, displayed_step);
    click(BUTTON_LEFT);
    TEST_ASSERT_EQUAL_PTR(&step_home, displayed_step);

    ux_flow_init(0, flow_review, &step_approve);
    // both buttons pressed one after the other
    buttons(BUTTON_LEFT);
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    buttons(BUTTON_RIGHT);
    buttons(0);
    TEST_ASSERT_EQUAL(1, nb_approvals);
}

// a dual-button gesture started on a previous screen must not approve a new flow
static void test_stale_dual_release_does_not_validate(void)
{
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    ux_flow_init(0, flow_review, &step_approve);
    buttons(0);
    TEST_ASSERT_EQUAL(0, nb_approvals);
    TEST_ASSERT_EQUAL_PTR(&step_approve, displayed_step);

    // a new gesture made on the displayed screen is accepted
    click(BUTTON_LEFT | BUTTON_RIGHT);
    TEST_ASSERT_EQUAL(1, nb_approvals);
}

// a single-button gesture started on a previous screen must not skip a review screen
static void test_stale_single_release_does_not_navigate(void)
{
    buttons(BUTTON_RIGHT);
    ux_flow_init(0, flow_review, NULL);
    buttons(0);
    TEST_ASSERT_EQUAL_PTR(&step_review, displayed_step);

    buttons(BUTTON_LEFT);
    ux_flow_init(0, flow_review, &step_approve);
    buttons(0);
    TEST_ASSERT_EQUAL_PTR(&step_approve, displayed_step);

    click(BUTTON_LEFT);
    TEST_ASSERT_EQUAL_PTR(&step_review, displayed_step);
}

// the gesture stays ignored as long as one of its buttons is held, held events included
static void test_stale_gesture_ignored_until_all_released(void)
{
    buttons(BUTTON_LEFT);
    ux_flow_init(0, flow_review, &step_approve);
    // held button periodic events
    buttons(BUTTON_LEFT);
    buttons(BUTTON_LEFT);
    // second button pressed and partial release while the first one is still held
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    buttons(BUTTON_LEFT);
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    buttons(0);
    TEST_ASSERT_EQUAL(0, nb_approvals);
    TEST_ASSERT_EQUAL_PTR(&step_approve, displayed_step);

    click(BUTTON_LEFT | BUTTON_RIGHT);
    TEST_ASSERT_EQUAL(1, nb_approvals);
}

// a step change done programmatically while a button is held is also protected
static void test_stale_release_after_programmatic_step_change(void)
{
    ux_flow_init(0, flow_review, NULL);
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    ux_flow_next();
    TEST_ASSERT_EQUAL_PTR(&step_approve, displayed_step);
    buttons(0);
    TEST_ASSERT_EQUAL(0, nb_approvals);
}

// redrawing the same step keeps the ongoing gesture
static void test_relayout_keeps_gesture(void)
{
    ux_flow_init(0, flow_review, &step_approve);
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    TEST_ASSERT_TRUE(ux_flow_relayout());
    buttons(0);
    TEST_ASSERT_EQUAL(1, nb_approvals);
}

// a step advanced without display is a new step when shown by the next relayout
static void test_relayout_after_next_no_display(void)
{
    ux_flow_init(0, flow_review, NULL);
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    ux_flow_next_no_display();
    TEST_ASSERT_EQUAL_PTR(&step_review, displayed_step);
    TEST_ASSERT_TRUE(ux_flow_relayout());
    TEST_ASSERT_EQUAL_PTR(&step_approve, displayed_step);
    buttons(0);
    TEST_ASSERT_EQUAL(0, nb_approvals);

    // later redraws of that step keep the ongoing gesture
    buttons(BUTTON_LEFT | BUTTON_RIGHT);
    TEST_ASSERT_TRUE(ux_flow_relayout());
    buttons(0);
    TEST_ASSERT_EQUAL(1, nb_approvals);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_fresh_gestures);
    RUN_TEST(test_stale_dual_release_does_not_validate);
    RUN_TEST(test_stale_single_release_does_not_navigate);
    RUN_TEST(test_stale_gesture_ignored_until_all_released);
    RUN_TEST(test_stale_release_after_programmatic_step_change);
    RUN_TEST(test_relayout_keeps_gesture);
    RUN_TEST(test_relayout_after_next_no_display);
    return UNITY_END();
}
