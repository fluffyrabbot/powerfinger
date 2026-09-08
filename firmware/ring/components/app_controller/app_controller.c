// SPDX-License-Identifier: MIT
// PowerFinger — Ring app controller implementation

#include "app_controller.h"

#include <string.h>

#include "ble_config.h"
#include "hal_ble.h"
#include "ring_config.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "app_ctrl";
#define APP_CTRL_LOGI(...) ESP_LOGI(TAG, __VA_ARGS__)
#define APP_CTRL_LOGW(...) ESP_LOGW(TAG, __VA_ARGS__)
#define APP_CTRL_LOGE(...) ESP_LOGE(TAG, __VA_ARGS__)
#else
#define APP_CTRL_LOGI(...) (void)0
#define APP_CTRL_LOGW(...) (void)0
#define APP_CTRL_LOGE(...) (void)0
#endif

static void execute_actions(app_controller_t *controller,
                            const ring_actions_t *actions,
                            uint32_t now_ms)
{
    if (!controller || !actions) {
        return;
    }

    if (actions->stop_advertising || actions->enter_deep_sleep) {
        controller->advertising_started_ms = 0;
    }
    if (actions->start_advertising) {
        controller->advertising_started_ms = now_ms;
    }

    if (actions->stop_advertising) {
        hal_ble_stop_advertising();
    }
    if (actions->start_advertising) {
        if (hal_ble_start_advertising(BLE_ADVERTISE_TIMEOUT_MS) != HAL_OK) {
            controller->advertising_started_ms = 0;
            APP_CTRL_LOGE("advertising start failed — entering deep sleep");
            power_manager_enter_sleep(true);
            return;
        }
    }
    if (actions->enter_deep_sleep) {
        power_manager_enter_sleep(true);
    }
}

static void note_connected_transition(app_controller_t *controller,
                                      uint16_t conn_interval_1_25ms,
                                      const char *reason)
{
    if (!controller) {
        return;
    }

    power_manager_on_connect();

    if (controller->runtime_health) {
        ring_runtime_health_reset_hid_send(controller->runtime_health);
    }

    if (controller->diagnostics) {
        ring_diagnostics_note_connected(controller->diagnostics,
                                        conn_interval_1_25ms);
        app_controller_log_diagnostics_snapshot(reason, controller->diagnostics);
    }

    controller->previous_buttons = 0;
}

static void note_disconnected_transition(app_controller_t *controller,
                                         const char *reason)
{
    if (!controller) {
        return;
    }

    if (controller->primary_dead_zone) {
        dead_zone_reset(controller->primary_dead_zone);
    }
    if (controller->secondary_dead_zone) {
        dead_zone_reset(controller->secondary_dead_zone);
    }

    power_manager_on_disconnect();

    if (controller->runtime_health) {
        ring_runtime_health_reset_hid_send(controller->runtime_health);
    }

    if (controller->diagnostics) {
        ring_diagnostics_note_disconnected(controller->diagnostics);
        app_controller_log_diagnostics_snapshot(reason, controller->diagnostics);
    }

    controller->previous_buttons = 0;
}

void app_controller_init(app_controller_t *controller,
                         ring_diagnostics_t *diagnostics,
                         ring_runtime_health_t *runtime_health,
                         dead_zone_ctx_t *primary_dead_zone,
                         dead_zone_ctx_t *secondary_dead_zone)
{
    if (!controller) {
        return;
    }

    memset(controller, 0, sizeof(*controller));
    controller->diagnostics = diagnostics;
    controller->runtime_health = runtime_health;
    controller->primary_dead_zone = primary_dead_zone;
    controller->secondary_dead_zone = secondary_dead_zone;
}

void app_controller_log_diagnostics_snapshot(const char *reason,
                                             const ring_diagnostics_t *diagnostics)
{
    if (!reason || !diagnostics) {
        return;
    }

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(diagnostics);
    uint32_t interval_centims = (uint32_t)snapshot.conn_interval_1_25ms * 125U;
    uint32_t interval_ms_whole = interval_centims / 100U;
    uint32_t interval_ms_frac = interval_centims % 100U;

    if (snapshot.conn_interval_1_25ms == 0) {
        APP_CTRL_LOGI(
            "diag[%s]: state=%s sensor=%s cal=%s bond=%s conn=unknown rejected=%s batt=%u%% (%lumV)"
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
            " drv5032=%s spur=%u"
#endif
            ,
            reason,
            ring_state_name(snapshot.ring_state),
            ring_diag_sensor_state_name(snapshot.sensor_state),
            snapshot.calibration_valid ? "valid" : "pending",
            ring_diag_bond_state_name(snapshot.bond_state),
            snapshot.conn_param_rejected ? "yes" : "no",
            snapshot.battery_pct,
            (unsigned long)snapshot.battery_mv
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
            ,
            snapshot.drv5032_wake_enabled ? "enabled" : "disabled",
            snapshot.spurious_wake_count
#endif
        );
        return;
    }

    APP_CTRL_LOGI(
        "diag[%s]: state=%s sensor=%s cal=%s bond=%s conn=%lu.%02lums rejected=%s batt=%u%% (%lumV)"
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
        " drv5032=%s spur=%u"
#endif
        ,
        reason,
        ring_state_name(snapshot.ring_state),
        ring_diag_sensor_state_name(snapshot.sensor_state),
        snapshot.calibration_valid ? "valid" : "pending",
        ring_diag_bond_state_name(snapshot.bond_state),
        (unsigned long)interval_ms_whole,
        (unsigned long)interval_ms_frac,
        snapshot.conn_param_rejected ? "yes" : "no",
        snapshot.battery_pct,
        (unsigned long)snapshot.battery_mv
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
        ,
        snapshot.drv5032_wake_enabled ? "enabled" : "disabled",
        snapshot.spurious_wake_count
#endif
    );
}

void app_controller_dispatch_ring_event(app_controller_t *controller,
                                        ring_event_t event,
                                        uint16_t conn_interval_1_25ms,
                                        uint32_t now_ms,
                                        const char *reason)
{
    if (!controller) {
        return;
    }

    ring_actions_t actions;
    memset(&actions, 0, sizeof(actions));

    ring_state_dispatch(event, &actions);
    if (controller->diagnostics) {
        ring_diagnostics_note_ring_state(controller->diagnostics, ring_state_get());
    }

    bool transition_logs_after = (event == RING_EVT_BLE_CONNECTED ||
                                  event == RING_EVT_BLE_DISCONNECTED);
    bool log_before_actions = (reason != NULL &&
                               controller->diagnostics != NULL &&
                               !transition_logs_after &&
                               actions.enter_deep_sleep);
    if (log_before_actions) {
        app_controller_log_diagnostics_snapshot(reason, controller->diagnostics);
    }

    execute_actions(controller, &actions, now_ms);

    if (event == RING_EVT_BLE_CONNECTED) {
        note_connected_transition(controller,
                                  conn_interval_1_25ms,
                                  reason ? reason : "connect");
        return;
    }

    if (event == RING_EVT_BLE_DISCONNECTED) {
        note_disconnected_transition(controller,
                                     reason ? reason : "disconnect");
        return;
    }

    if (reason && controller->diagnostics && !log_before_actions) {
        app_controller_log_diagnostics_snapshot(reason, controller->diagnostics);
    }
}

void app_controller_handle_event(app_controller_t *controller,
                                 const app_controller_event_t *event,
                                 uint32_t now_ms)
{
    if (!controller || !event) {
        return;
    }

    switch (event->type) {
    case APP_CONTROLLER_EVT_RING_STATE:
        app_controller_dispatch_ring_event(
            controller,
            event->ring_evt,
            event->conn_interval_1_25ms,
            now_ms,
            event->ring_evt == RING_EVT_BLE_CONNECTED ? "connect" :
            event->ring_evt == RING_EVT_BLE_DISCONNECTED ? "disconnect" : NULL);
        break;

    case APP_CONTROLLER_EVT_BOND_RESTORED:
        if (controller->diagnostics) {
            ring_diagnostics_note_bond_restored(controller->diagnostics);
            app_controller_log_diagnostics_snapshot("bond-restored",
                                                    controller->diagnostics);
        }
        break;

    case APP_CONTROLLER_EVT_BOND_FAILED:
        if (controller->diagnostics) {
            ring_diagnostics_note_bond_failed(controller->diagnostics);
            app_controller_log_diagnostics_snapshot("bond-failed",
                                                    controller->diagnostics);
        }
        break;

    case APP_CONTROLLER_EVT_CONN_PARAMS_UPDATED:
        power_manager_on_conn_params_updated(event->conn_interval_1_25ms);
        if (controller->diagnostics) {
            ring_diagnostics_note_conn_params_updated(controller->diagnostics,
                                                      event->conn_interval_1_25ms);
            app_controller_log_diagnostics_snapshot("conn-update",
                                                    controller->diagnostics);
        }
        break;

    case APP_CONTROLLER_EVT_CONN_PARAMS_REJECTED:
        power_manager_on_conn_params_rejected();
        if (controller->diagnostics) {
            ring_diagnostics_note_conn_param_rejected(controller->diagnostics);
            app_controller_log_diagnostics_snapshot("conn-rejected",
                                                    controller->diagnostics);
        }
        break;
    }
}

void app_controller_reconcile_ble_event_drops(app_controller_t *controller,
                                              unsigned int dropped,
                                              bool connected,
                                              uint32_t now_ms)
{
    if (!controller || dropped == 0U) {
        return;
    }

    ring_state_t state = ring_state_get();

    APP_CTRL_LOGW("BLE event queue dropped %u event(s) — reconciling state=%s connected=%s",
                  dropped,
                  ring_state_name(state),
                  connected ? "yes" : "no");

    if (connected && state == RING_STATE_ADVERTISING) {
        app_controller_dispatch_ring_event(controller,
                                           RING_EVT_BLE_CONNECTED,
                                           0,
                                           now_ms,
                                           "queue-reconcile-connect");
        return;
    }

    if (!connected &&
        (state == RING_STATE_CONNECTED_ACTIVE || state == RING_STATE_CONNECTED_IDLE)) {
        app_controller_dispatch_ring_event(controller,
                                           RING_EVT_BLE_DISCONNECTED,
                                           0,
                                           now_ms,
                                           "queue-reconcile-disconnect");
        return;
    }

    APP_CTRL_LOGW("BLE queue drop did not require topology repair (state=%s connected=%s)",
                  ring_state_name(state),
                  connected ? "yes" : "no");
}

void app_controller_check_advertising_timeout(app_controller_t *controller,
                                              uint32_t now_ms)
{
    if (!controller || controller->advertising_started_ms == 0) {
        return;
    }

    if (ring_state_get() == RING_STATE_ADVERTISING &&
        (now_ms - controller->advertising_started_ms) >= RECONNECT_TIMEOUT_MS) {
        app_controller_dispatch_ring_event(controller,
                                           RING_EVT_BLE_ADV_TIMEOUT,
                                           0,
                                           now_ms,
                                           "adv-timeout");
    }
}

void app_controller_handle_power_event(app_controller_t *controller,
                                       power_event_t event,
                                       uint32_t now_ms)
{
    ring_event_t ring_event = RING_EVT_NONE;

    switch (event) {
    case POWER_EVT_IDLE_TIMEOUT:
        ring_event = RING_EVT_IDLE_TIMEOUT;
        break;
    case POWER_EVT_LOW_BATTERY:
    case POWER_EVT_THERMAL_SHUTDOWN:
        ring_event = RING_EVT_LOW_BATTERY;
        break;
    case POWER_EVT_SLEEP_TIMEOUT:
        ring_event = RING_EVT_SLEEP_TIMEOUT;
        break;
    case POWER_EVT_NONE:
    default:
        break;
    }

    if (ring_event != RING_EVT_NONE) {
        app_controller_dispatch_ring_event(controller,
                                           ring_event,
                                           0,
                                           now_ms,
                                           "power-event");
    }
}

uint8_t app_controller_previous_buttons(const app_controller_t *controller)
{
    return controller ? controller->previous_buttons : 0;
}

void app_controller_set_previous_buttons(app_controller_t *controller,
                                         uint8_t buttons)
{
    if (!controller) {
        return;
    }

    controller->previous_buttons = buttons;
}
