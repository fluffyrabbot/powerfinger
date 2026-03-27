// SPDX-License-Identifier: MIT
// PowerFinger — USB HID mouse unit tests
//
// Tests report packing (byte layout), descriptor properties, zero-report
// behavior, and the main-loop stuck-button prevention logic.

#include "unity.h"
#include "usb_hid_mouse.h"
#include "hid_report_descriptor.h"
#include "event_composer.h"
#include <string.h>

// Convenience: assert uint8_t equality (vendored Unity lacks HEX8 variant)
#define ASSERT_BYTE(expected, actual) \
    TEST_ASSERT_EQUAL((int)(uint8_t)(expected), (int)(uint8_t)(actual))

static void reset(void)
{
    usb_hid_mouse_test_reset();
    usb_hid_mouse_init();
    event_composer_init();
}

// --- Report packing ---

void test_usb_zero_report_is_all_zeros(void)
{
    reset();
    composed_report_t r;
    memset(&r, 0, sizeof(r));

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    for (int i = 0; i < USB_HID_REPORT_SIZE; i++) {
        ASSERT_BYTE(0x00, buf[i]);
    }
}

void test_usb_buttons_packed_in_byte_0(void)
{
    reset();
    composed_report_t r = {0};
    r.buttons = 0x05;  // left + middle

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    ASSERT_BYTE(0x05, buf[0]);
}

void test_usb_buttons_masked_to_3_bits(void)
{
    reset();
    composed_report_t r = {0};
    r.buttons = 0xFF;  // only lower 3 bits should survive

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    ASSERT_BYTE(0x07, buf[0]);
}

void test_usb_cursor_positive_xy(void)
{
    reset();
    composed_report_t r = {0};
    r.cursor_dx = 300;
    r.cursor_dy = 150;

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    // 300 = 0x012C LE -> buf[1]=0x2C, buf[2]=0x01
    ASSERT_BYTE(0x2C, buf[1]);
    ASSERT_BYTE(0x01, buf[2]);
    // 150 = 0x0096 LE -> buf[3]=0x96, buf[4]=0x00
    ASSERT_BYTE(0x96, buf[3]);
    ASSERT_BYTE(0x00, buf[4]);
}

void test_usb_cursor_negative_xy(void)
{
    reset();
    composed_report_t r = {0};
    r.cursor_dx = -500;
    r.cursor_dy = -1;

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    // -500 = 0xFE0C LE -> buf[1]=0x0C, buf[2]=0xFE
    ASSERT_BYTE(0x0C, buf[1]);
    ASSERT_BYTE(0xFE, buf[2]);
    // -1 = 0xFFFF LE -> buf[3]=0xFF, buf[4]=0xFF
    ASSERT_BYTE(0xFF, buf[3]);
    ASSERT_BYTE(0xFF, buf[4]);
}

void test_usb_scroll_vertical(void)
{
    reset();
    composed_report_t r = {0};
    r.scroll_v = -10;

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    ASSERT_BYTE((uint8_t)-10, buf[5]);
}

void test_usb_scroll_horizontal(void)
{
    reset();
    composed_report_t r = {0};
    r.scroll_h = 42;

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    ASSERT_BYTE(42, buf[6]);
}

void test_usb_full_report(void)
{
    reset();
    composed_report_t r = {
        .buttons = 0x03,       // left + right
        .cursor_dx = 1000,
        .cursor_dy = -2000,
        .scroll_v = 5,
        .scroll_h = -3,
    };

    usb_hid_mouse_send(&r);
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();

    ASSERT_BYTE(0x03, buf[0]);
    // 1000 = 0x03E8 LE
    ASSERT_BYTE(0xE8, buf[1]);
    ASSERT_BYTE(0x03, buf[2]);
    // -2000 = 0xF830 LE
    ASSERT_BYTE(0x30, buf[3]);
    ASSERT_BYTE(0xF8, buf[4]);
    ASSERT_BYTE(5, buf[5]);
    ASSERT_BYTE((uint8_t)-3, buf[6]);
}

// --- Report descriptor validity ---

void test_descriptor_not_empty(void)
{
    TEST_ASSERT_TRUE(usb_hid_report_descriptor_len > 0);
}

void test_descriptor_starts_with_usage_page(void)
{
    // First two bytes: Usage Page (Generic Desktop) = 0x05, 0x01
    ASSERT_BYTE(0x05, usb_hid_report_descriptor[0]);
    ASSERT_BYTE(0x01, usb_hid_report_descriptor[1]);
}

void test_descriptor_ends_with_end_collection(void)
{
    // Last byte should be End Collection (0xC0)
    ASSERT_BYTE(0xC0,
        usb_hid_report_descriptor[usb_hid_report_descriptor_len - 1]);
}

void test_descriptor_contains_consumer_page(void)
{
    // Must contain Usage Page (Consumer) = 0x05, 0x0C for horizontal scroll
    bool found = false;
    for (size_t i = 0; i + 1 < usb_hid_report_descriptor_len; i++) {
        if (usb_hid_report_descriptor[i] == 0x05 &&
            usb_hid_report_descriptor[i + 1] == 0x0C) {
            found = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found); // Must contain Usage Page (Consumer) for AC Pan
}

void test_descriptor_contains_ac_pan(void)
{
    // Must contain Usage (AC Pan) = 0x0A, 0x38, 0x02
    bool found = false;
    for (size_t i = 0; i + 2 < usb_hid_report_descriptor_len; i++) {
        if (usb_hid_report_descriptor[i] == 0x0A &&
            usb_hid_report_descriptor[i + 1] == 0x38 &&
            usb_hid_report_descriptor[i + 2] == 0x02) {
            found = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found); // Must contain Usage (AC Pan) for horizontal scroll
}

// --- Send count tracking ---

void test_usb_send_count_increments(void)
{
    reset();
    composed_report_t r = {0};
    r.cursor_dx = 1;

    TEST_ASSERT_EQUAL(0, usb_hid_mouse_test_get_send_count());
    usb_hid_mouse_send(&r);
    TEST_ASSERT_EQUAL(1, usb_hid_mouse_test_get_send_count());
    usb_hid_mouse_send(&r);
    TEST_ASSERT_EQUAL(2, usb_hid_mouse_test_get_send_count());
}

// --- Failure injection ---

void test_usb_send_failure_returns_error(void)
{
    reset();
    composed_report_t r = {0};
    r.cursor_dx = 1;

    usb_hid_mouse_test_inject_failure(HAL_ERR_IO, 2);

    TEST_ASSERT_EQUAL(HAL_ERR_IO, usb_hid_mouse_send(&r));
    TEST_ASSERT_EQUAL(HAL_ERR_IO, usb_hid_mouse_send(&r));
    // Third call succeeds (injection exhausted)
    TEST_ASSERT_EQUAL(HAL_OK, usb_hid_mouse_send(&r));
}

void test_usb_null_report_returns_error(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, usb_hid_mouse_send(NULL));
}

// --- Main loop zero-report logic (integration with event composer) ---
// Simulates the hub main loop pattern from main.c:
//   if (report_nonzero || prev_report_nonzero) { send(); }
// This ensures a zero-report is transmitted on transition to release
// stuck buttons on the host.

static bool report_is_nonzero(const composed_report_t *r)
{
    return (r->buttons != 0 ||
            r->cursor_dx != 0 || r->cursor_dy != 0 ||
            r->scroll_v != 0 || r->scroll_h != 0);
}

void test_zero_report_sent_on_transition(void)
{
    reset();
    event_composer_mark_connected(0);

    // Simulate main loop: non-zero report, then disconnect
    event_composer_feed(0, ROLE_CURSOR, 0x01, 10, 5);
    composed_report_t r;
    event_composer_compose(&r);
    bool prev_nonzero = report_is_nonzero(&r);
    TEST_ASSERT_TRUE(prev_nonzero);

    if (report_is_nonzero(&r) || prev_nonzero) {
        usb_hid_mouse_send(&r);
    }
    TEST_ASSERT_EQUAL(1, usb_hid_mouse_test_get_send_count());

    // Ring disconnects — buttons released by event_composer_ring_disconnected
    event_composer_ring_disconnected(0);
    event_composer_compose(&r);
    bool curr_nonzero = report_is_nonzero(&r);
    TEST_ASSERT_FALSE(curr_nonzero);

    // The zero report MUST be sent because prev was nonzero
    if (curr_nonzero || prev_nonzero) {
        usb_hid_mouse_send(&r);
    }
    TEST_ASSERT_EQUAL(2, usb_hid_mouse_test_get_send_count());

    // Verify the zero report has all zeros
    const uint8_t *buf = usb_hid_mouse_test_get_last_report();
    for (int i = 0; i < USB_HID_REPORT_SIZE; i++) {
        ASSERT_BYTE(0x00, buf[i]);
    }

    // Next iteration: both zero — should NOT send
    prev_nonzero = curr_nonzero;
    event_composer_compose(&r);
    curr_nonzero = report_is_nonzero(&r);
    if (curr_nonzero || prev_nonzero) {
        usb_hid_mouse_send(&r);
    }
    // Send count should NOT have incremented
    TEST_ASSERT_EQUAL(2, usb_hid_mouse_test_get_send_count());
}

void test_continuous_zero_reports_suppressed(void)
{
    reset();
    // No rings connected — all composes produce zero

    composed_report_t r;
    bool prev_nonzero = false;
    uint32_t initial_count = usb_hid_mouse_test_get_send_count();

    for (int i = 0; i < 10; i++) {
        event_composer_compose(&r);
        bool curr = report_is_nonzero(&r);
        if (curr || prev_nonzero) {
            usb_hid_mouse_send(&r);
        }
        prev_nonzero = curr;
    }

    // No sends should have occurred
    TEST_ASSERT_EQUAL(initial_count, usb_hid_mouse_test_get_send_count());
}

// --- Test runner ---

void run_usb_hid_tests(void)
{
    printf("USB HID tests:\n");
    RUN_TEST(test_usb_zero_report_is_all_zeros);
    RUN_TEST(test_usb_buttons_packed_in_byte_0);
    RUN_TEST(test_usb_buttons_masked_to_3_bits);
    RUN_TEST(test_usb_cursor_positive_xy);
    RUN_TEST(test_usb_cursor_negative_xy);
    RUN_TEST(test_usb_scroll_vertical);
    RUN_TEST(test_usb_scroll_horizontal);
    RUN_TEST(test_usb_full_report);
    RUN_TEST(test_descriptor_not_empty);
    RUN_TEST(test_descriptor_starts_with_usage_page);
    RUN_TEST(test_descriptor_ends_with_end_collection);
    RUN_TEST(test_descriptor_contains_consumer_page);
    RUN_TEST(test_descriptor_contains_ac_pan);
    RUN_TEST(test_usb_send_count_increments);
    RUN_TEST(test_usb_send_failure_returns_error);
    RUN_TEST(test_usb_null_report_returns_error);
    RUN_TEST(test_zero_report_sent_on_transition);
    RUN_TEST(test_continuous_zero_reports_suppressed);
}
