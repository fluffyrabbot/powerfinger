// SPDX-License-Identifier: MIT
// PowerFinger Hub — Companion command session

#include "companion_session.h"

#include <string.h>

#define COMMAND_TOO_LONG_RESPONSE "ERR 400 command_too_long\n"
#define INTERNAL_ERROR_RESPONSE "ERR 500 internal_error\n"

static hal_status_t emit_response(companion_session_t *session, const char *response)
{
    if (!session || !session->emit_response || !response) {
        return HAL_ERR_INVALID_ARG;
    }

    return session->emit_response(response, session->cb_arg);
}

static hal_status_t process_complete_line(companion_session_t *session)
{
    if (!session || !session->fill_hub_info) {
        return HAL_ERR_INVALID_ARG;
    }

    companion_protocol_hub_info_t hub_info = {0};
    char response[COMPANION_SESSION_RESPONSE_MAX_LEN] = {0};

    session->fill_hub_info(&hub_info, session->cb_arg);

    hal_status_t rc = companion_protocol_handle_line(session->line_buf,
                                                     &hub_info,
                                                     response,
                                                     sizeof(response));
    if (rc == HAL_OK) {
        return emit_response(session, response);
    }

    hal_status_t emit_rc = emit_response(session, INTERNAL_ERROR_RESPONSE);
    return (emit_rc == HAL_OK) ? rc : emit_rc;
}

hal_status_t companion_session_init(companion_session_t *session,
                                    companion_session_hub_info_cb_t fill_hub_info,
                                    companion_session_emit_cb_t emit_response_cb,
                                    void *cb_arg)
{
    if (!session || !fill_hub_info || !emit_response_cb) {
        return HAL_ERR_INVALID_ARG;
    }

    memset(session, 0, sizeof(*session));
    session->fill_hub_info = fill_hub_info;
    session->emit_response = emit_response_cb;
    session->cb_arg = cb_arg;
    return HAL_OK;
}

hal_status_t companion_session_feed_bytes(companion_session_t *session,
                                          const uint8_t *bytes,
                                          size_t len)
{
    if (!session || (!bytes && len != 0)) {
        return HAL_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < len; i++) {
        char ch = (char)bytes[i];

        if (ch == '\r') {
            continue;
        }

        if (session->dropping_overlong_line) {
            if (ch == '\n') {
                session->dropping_overlong_line = false;
                session->line_len = 0;
                hal_status_t rc = emit_response(session, COMMAND_TOO_LONG_RESPONSE);
                if (rc != HAL_OK) {
                    return rc;
                }
            }
            continue;
        }

        if (ch == '\n') {
            session->line_buf[session->line_len] = '\0';
            hal_status_t rc = process_complete_line(session);
            session->line_len = 0;
            if (rc != HAL_OK) {
                return rc;
            }
            continue;
        }

        if (session->line_len >= COMPANION_PROTOCOL_COMMAND_LINE_MAX_LEN) {
            session->dropping_overlong_line = true;
            session->line_len = 0;
            continue;
        }

        session->line_buf[session->line_len++] = ch;
    }

    return HAL_OK;
}
