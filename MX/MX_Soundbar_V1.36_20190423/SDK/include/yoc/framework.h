/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file yoc/framework.h
 * @brief YOC Framework APIs
 * @version since 5.5.0
 */

#ifndef YOC_FRAMEWORK_API_H
#define YOC_FRAMEWORK_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <yoc/internal/event_type_code.h>

#define yoc_post_event yoc_local_event_post

/** @defgroup yoc_framework Framework API
 *  @{
 */

#ifndef YOC_DOXYGEN_MODE

/** system event */
#define EV_SYS                     0x0001
#define CODE_SYS_ON_STARTING         1
#define CODE_SYS_ON_START_COMPLETED  2
#define CODE_SYS_ON_START_FAILED     4
#define CODE_SYS_ON_IDLE             3

/** WiFi event */
#define  EV_WIFI                0x0002
#define  CODE_WIFI_CMD_RECONNECT  1
#define  CODE_WIFI_ON_CONNECTED   2
#define  CODE_WIFI_ON_DISCONNECT  3
#define  CODE_WIFI_ON_PRE_GOT_IP  4
#define  CODE_WIFI_ON_GOT_IP      5

/** Mesh event */
#define  EV_MESH                0x0003
#define  CODE_MESH_STARTED        1
#define  CODE_MESH_CONNECTED      2
#define  CODE_MESH_ATTACHED       3
#define  CODE_MESH_DETACHED       4

/** user app start */
#define EV_USER     0x1000

#endif

#include <nuttx/input.h>
#if 0
/**
 * @struct input_event_t
 * @brief yoc event structure
 */
typedef struct {
    /** The time event is generated, auto filled by yoc event system */
    uint32_t time;
    /** Event type, value < 0x1000 are used by yoc system */
    uint16_t type;
    /** Defined according to type */
    uint16_t code;
    /** Defined according to type/code */
    unsigned long value;
    /** Defined according to type/code */
    unsigned long extra;
} input_event_t;
#endif
/** Free callback  */
typedef void (*yoc_free_cb)(void *private_data);
/** Event callback */
typedef void (*yoc_event_cb)(input_event_t *event, void *private_data);
/** Delayed execution callback */
typedef void (*yoc_call_t)(void *arg);
/** Delayed execution callback */
typedef void (*yoc_poll_call_t)(int fd, void *arg);

/**
 * @brief Register system event callback
 * @param yoc_event_cb system event callback
 * @param yoc_free_cb free data callback
 * @param private_data data to be bypassed to cb
 * @return None
 */
void yoc_local_event_listener_register(yoc_event_cb cb, void *priv);

/**
 * @brief Unregister native event callback
 * @param yoc_event_cb system event callback
 */
void yoc_local_event_listener_unregister(yoc_event_cb cb, void *priv);

/**
 * @brief Post local event.
 * @param type event type
 * @param code event code
 * @param value event value
 * @retval >0 success
 * @retval <=0 failure
 * @see input_event_t
 */
int yoc_local_event_post(uint16_t type, uint16_t code, unsigned long  value);

/**
 * @brief Register a poll event in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @returns None
 */
void yoc_poll_read_fd(int fd, yoc_poll_call_t action, void *param);

/**
 * @brief Cancel a poll event to be executed in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @returns None
 * @note all the parameters must be the same as yoc_poll_read_fd
 */
void yoc_cancel_poll_read_fd(int fd, yoc_poll_call_t action, void *param);

/**
 * @brief Post a delayed action to be executed in main loop
 * @param ms milliseconds to wait
 * @param action action to be executed
 * @param arg private data past to action
 * @return none
 * @note This function must be called under main loop context.
 *       after 'action' is fired, resource will be automatically released.
 */
void yoc_post_delayed_action(int ms, yoc_call_t action, void *arg);

/**
 * @brief Cancel a delayed action to be executed in main loop
 * @param ms milliseconds to wait, -1 means don't care
 * @param action action to be executed
 * @param arg private data past to action
 * @returns None
 * @note all the parameters must be the same as yoc_post_delayed_action
 */
void yoc_cancel_delayed_action(int ms, yoc_call_t action, void *arg);

/**
 * @brief Schedule a callback in next event loop
 * @param action action to be executed
 * @param arg private data past to action
 * @retval >=0 success
 * @retval <0  failure
 * @note Unlike yoc_post_delayed_action,
 *       this function can be called from non-yoc-main-loop context.
 */
int yoc_schedule_call(yoc_call_t action, void *arg);

typedef void *yoc_loop_t;

/**
 * @brief init a per-task event loop
 * @param None
 * @retval ==NULL failure
 * @retval !=NULL success
 */
yoc_loop_t yoc_loop_init(void);

/**
 * @brief get current event loop
 * @param None
 * @retval default event loop
 */
yoc_loop_t yoc_current_loop(void);

/**
 * @brief start event loop
 * @param None
 * @retval None
 * @note this function won't return until yoc_loop_exit called
 */
void yoc_loop_run(void);

/**
 * @brief exit event loop, yoc_loop_run() will return
 * @param None
 * @retval None
 * @note this function must be called from the task runninng the event loop
 */
void yoc_loop_exit(void);

/**
 * @brief free event loop resources
 * @param None
 * @retval None
 * @note this function should be called after yoc_loop_run() return
 */
void yoc_loop_destroy(void);

/**
 * @brief Schedule a callback specified event loop
 * @param loop event loop to be scheduled, NULL for default main loop
 * @param action action to be executed
 * @param arg private data past to action
 * @retval >=0 success
 * @retval <0  failure
 * @note Unlike yoc_post_delayed_action,
 *       this function can be called from non-yoc-main-loop context.
 */
int yoc_loop_schedule_call(yoc_loop_t *loop, yoc_call_t action, void *arg);

int yoc_kv_set(const char *key, const void *value, int len, int sync);
int yoc_kv_get(const char *key, void *buffer, int *buffer_len);
int yoc_kv_del(const char *key);

/** @} */ //end of Framework API

#ifdef __cplusplus
}
#endif

#endif /* YOC_FRAMEWORK_API_H */

