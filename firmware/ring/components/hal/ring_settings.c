// SPDX-License-Identifier: MIT
// PowerFinger — Persisted ring runtime settings

#include "ring_settings.h"
#include "hal_storage.h"
#include "ring_config.h"

#include <limits.h>
#include <stdatomic.h>
#include <string.h>

#define RING_SETTINGS_STORAGE_KEY     "ring_cfg"
#define RING_SETTINGS_BLOB_VERSION    1U

typedef struct {
    uint8_t version;
    uint8_t dpi_multiplier;
    uint8_t dead_zone_time_le[2];
    uint8_t dead_zone_distance;
} ring_settings_blob_t;

_Static_assert(sizeof(ring_settings_blob_t) == 5,
               "ring settings blob size must stay stable");

static atomic_uint_least8_t s_dpi_multiplier = ATOMIC_VAR_INIT(DPI_MULTIPLIER_DEFAULT);
static atomic_uint_least16_t s_dead_zone_time_ms = ATOMIC_VAR_INIT(DEAD_ZONE_TIME_MS);
static atomic_uint_least8_t s_dead_zone_distance = ATOMIC_VAR_INIT(DEAD_ZONE_DISTANCE);
static atomic_uint s_generation = ATOMIC_VAR_INIT(0);
static atomic_uint s_flushed_generation = ATOMIC_VAR_INIT(0);
static atomic_bool s_storage_ready = ATOMIC_VAR_INIT(false);

static ring_settings_snapshot_t ring_settings_defaults(void)
{
    ring_settings_snapshot_t defaults = {
        .dpi_multiplier = DPI_MULTIPLIER_DEFAULT,
        .dead_zone_time_ms = DEAD_ZONE_TIME_MS,
        .dead_zone_distance = DEAD_ZONE_DISTANCE,
    };
    return defaults;
}

static bool ring_settings_valid(const ring_settings_snapshot_t *settings)
{
    if (!settings) {
        return false;
    }

    if (settings->dpi_multiplier < DPI_MULTIPLIER_MIN) {
        return false;
    }
    if (settings->dead_zone_time_ms > DEAD_ZONE_TIME_MS_MAX) {
        return false;
    }
    return true;
}

static void ring_settings_store_snapshot(const ring_settings_snapshot_t *settings)
{
    atomic_store(&s_dpi_multiplier, settings->dpi_multiplier);
    atomic_store(&s_dead_zone_time_ms, settings->dead_zone_time_ms);
    atomic_store(&s_dead_zone_distance, settings->dead_zone_distance);
}

static void ring_settings_mark_dirty(void)
{
    atomic_fetch_add(&s_generation, 1U);
}

static ring_settings_blob_t ring_settings_blob_from_snapshot(
    const ring_settings_snapshot_t *settings)
{
    ring_settings_blob_t blob = {
        .version = RING_SETTINGS_BLOB_VERSION,
        .dpi_multiplier = settings->dpi_multiplier,
        .dead_zone_time_le = {
            (uint8_t)(settings->dead_zone_time_ms & 0xFFU),
            (uint8_t)((settings->dead_zone_time_ms >> 8) & 0xFFU),
        },
        .dead_zone_distance = settings->dead_zone_distance,
    };
    return blob;
}

static bool ring_settings_snapshot_from_blob(const ring_settings_blob_t *blob,
                                             ring_settings_snapshot_t *settings)
{
    if (!blob || !settings || blob->version != RING_SETTINGS_BLOB_VERSION) {
        return false;
    }

    ring_settings_snapshot_t candidate = {
        .dpi_multiplier = blob->dpi_multiplier,
        .dead_zone_time_ms = (uint16_t)blob->dead_zone_time_le[0] |
                             ((uint16_t)blob->dead_zone_time_le[1] << 8),
        .dead_zone_distance = blob->dead_zone_distance,
    };

    if (!ring_settings_valid(&candidate)) {
        return false;
    }

    *settings = candidate;
    return true;
}

hal_status_t ring_settings_init(void)
{
    ring_settings_snapshot_t defaults = ring_settings_defaults();
    ring_settings_store_snapshot(&defaults);
    atomic_store(&s_generation, 0);
    atomic_store(&s_flushed_generation, 0);
    atomic_store(&s_storage_ready, false);

    hal_status_t rc = hal_storage_init();
    if (rc != HAL_OK) {
        return rc;
    }
    atomic_store(&s_storage_ready, true);

    ring_settings_blob_t blob = {0};
    size_t blob_len = sizeof(blob);
    rc = hal_storage_get(RING_SETTINGS_STORAGE_KEY, &blob, &blob_len);
    if (rc == HAL_ERR_NOT_FOUND) {
        return HAL_OK;
    }
    if (rc != HAL_OK || blob_len != sizeof(blob)) {
        ring_settings_mark_dirty();
        return (rc == HAL_OK) ? HAL_ERR_INVALID_ARG : rc;
    }

    ring_settings_snapshot_t loaded = {0};
    if (!ring_settings_snapshot_from_blob(&blob, &loaded)) {
        ring_settings_mark_dirty();
        return HAL_ERR_INVALID_ARG;
    }

    ring_settings_store_snapshot(&loaded);
    return HAL_OK;
}

ring_settings_snapshot_t ring_settings_snapshot(void)
{
    ring_settings_snapshot_t snapshot = {
        .dpi_multiplier = atomic_load(&s_dpi_multiplier),
        .dead_zone_time_ms = atomic_load(&s_dead_zone_time_ms),
        .dead_zone_distance = atomic_load(&s_dead_zone_distance),
    };
    return snapshot;
}

uint8_t ring_settings_get_dpi_multiplier(void)
{
    return atomic_load(&s_dpi_multiplier);
}

uint16_t ring_settings_get_dead_zone_time_ms(void)
{
    return atomic_load(&s_dead_zone_time_ms);
}

uint8_t ring_settings_get_dead_zone_distance(void)
{
    return atomic_load(&s_dead_zone_distance);
}

hal_status_t ring_settings_set_dpi_multiplier(uint8_t value)
{
    if (value < DPI_MULTIPLIER_MIN) {
        return HAL_ERR_INVALID_ARG;
    }

    if (atomic_exchange(&s_dpi_multiplier, value) != value) {
        ring_settings_mark_dirty();
    }
    return HAL_OK;
}

hal_status_t ring_settings_set_dead_zone_time_ms(uint16_t value)
{
    if (value > DEAD_ZONE_TIME_MS_MAX) {
        return HAL_ERR_INVALID_ARG;
    }

    if (atomic_exchange(&s_dead_zone_time_ms, value) != value) {
        ring_settings_mark_dirty();
    }
    return HAL_OK;
}

hal_status_t ring_settings_set_dead_zone_distance(uint8_t value)
{
    if (atomic_exchange(&s_dead_zone_distance, value) != value) {
        ring_settings_mark_dirty();
    }
    return HAL_OK;
}

bool ring_settings_needs_flush(void)
{
    return atomic_load(&s_generation) != atomic_load(&s_flushed_generation);
}

hal_status_t ring_settings_flush(void)
{
    if (!atomic_load(&s_storage_ready)) {
        return HAL_ERR_IO;
    }

    unsigned int target_generation = atomic_load(&s_generation);
    if (target_generation == atomic_load(&s_flushed_generation)) {
        return HAL_OK;
    }

    ring_settings_snapshot_t snapshot = ring_settings_snapshot();
    ring_settings_blob_t blob = ring_settings_blob_from_snapshot(&snapshot);

    hal_status_t rc = hal_storage_set(RING_SETTINGS_STORAGE_KEY, &blob, sizeof(blob));
    if (rc != HAL_OK) {
        return rc;
    }

    rc = hal_storage_commit();
    if (rc != HAL_OK) {
        return rc;
    }

    if (atomic_load(&s_generation) == target_generation) {
        atomic_store(&s_flushed_generation, target_generation);
    }

    return HAL_OK;
}

int16_t ring_settings_scale_delta(int16_t raw_delta)
{
    int32_t scaled = ((int32_t)raw_delta * (int32_t)ring_settings_get_dpi_multiplier()) / 10;

    if (scaled > INT16_MAX) {
        return INT16_MAX;
    }
    if (scaled < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)scaled;
}
