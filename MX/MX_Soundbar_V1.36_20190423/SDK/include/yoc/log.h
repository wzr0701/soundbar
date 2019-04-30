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
 * @file yoc/log.h
 * @brief YOC LOG APIs
 * @version since 5.5.0
 */

#ifndef YOC_LOG_H
#define YOC_LOG_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <yoc/internal/log_impl.h>

/**
 * @brief log level definition.
 */

typedef enum {
    YOC_LL_NONE,  /**< disable log */
    YOC_LL_FATAL, /**< fatal log will output */
    YOC_LL_ERROR, /**< fatal + error log will output */
    YOC_LL_WARN,  /**< fatal + warn + error log will output(default level) */
    YOC_LL_INFO,  /**< info + warn + error log will output */
    YOC_LL_DEBUG, /**< debug + info + warn + error + fatal log will output */
} yoc_log_level_t;

/**
 * @brief log level control
 * @param[in] log_level
 * @see yoc_log_level_t.
 */
void yoc_set_log_level(yoc_log_level_t log_level);

/**
 * @brief fatal log
 * @param[in] mod string description of module
 * @param[in] fmt same as printf() usage
 */
#define LOGF(mod, fmt, ...) LOGF_IMPL(mod, fmt, ##__VA_ARGS__)

/**
 * @brief error log
 * @param[in] mod string description of module
 * @param[in] fmt same as printf() usage
 */
#define LOGE(mod, fmt, ...) LOGE_IMPL(mod, fmt, ##__VA_ARGS__)

/**
 * @brief warning log
 * @param[in] mod string description of module
 * @param[in] fmt same as printf() usage
 */
#define LOGW(mod, fmt, ...) LOGW_IMPL(mod, fmt, ##__VA_ARGS__)

/**
 * @brief information log
 * @param[in] mod string description of module
 * @param[in] fmt same as printf() usage
 */
#define LOGI(mod, fmt, ...) LOGI_IMPL(mod, fmt, ##__VA_ARGS__)

/**
 * @brief debug log
 * @param[in] mod string description of module
 * @param[in] fmt same as printf() usage
 */
#define LOGD(mod, fmt, ...) LOGD_IMPL(mod, fmt, ##__VA_ARGS__)

/**
 * @brief log at warning level
 * @param[in] fmt same as printf() usage
 */
#define LOG(fmt, ...) LOG_IMPL(fmt, ##__VA_ARGS__)

#if defined(__cplusplus)
}
#endif

#endif /* YOC_LOG_H */

