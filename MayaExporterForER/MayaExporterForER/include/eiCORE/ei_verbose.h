/* 
 * Copyright 2010 elvish render Team
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef EI_VERBOSE_H
#define EI_VERBOSE_H

/**
 * \file ei_verbose.h
 * \brief Routines for outputing messages based on verbosity levels.
 */

#include <eiCORE/ei_core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EI_MAX_MSG_LEN		512

/** \brief Verbosity levels, the default value is EI_VERBOSE_WARNING.
 */
enum {
	EI_VERBOSE_NONE = 0, 
	EI_VERBOSE_FATAL, 
	EI_VERBOSE_ERROR, 
	EI_VERBOSE_WARNING, 
	EI_VERBOSE_INFO, 
	EI_VERBOSE_DEBUG, 
	EI_VERBOSE_ALL, 
};

/** \brief Users can override this global verbosity print callback.
 */
typedef void (*ei_verbose_print)(
	const eiInt severity, 
	const char *message, 
	void *params);

/** \brief Initialize verbosity.
 */
eiCORE_API void ei_verbose_init();

/** \brief Exit verbosity.
 */
eiCORE_API void ei_verbose_exit();

/** \brief Set current verbose level.
 */
eiCORE_API void ei_verbose_set(const eiInt level);

/** \brief Get current verbose level.
 */
eiCORE_API eiInt ei_verbose_get();

/** \brief Set verbosity print callback.
 */
eiCORE_API void ei_verbose_callback(ei_verbose_print print, void *params);

/** \brief Check if the verbosity print callback is available.
 */
eiCORE_API eiBool ei_verbose_check_callback();

eiCORE_API void ei_fatal(const char* format, ...);
eiCORE_API void ei_error(const char* format, ...);
eiCORE_API void ei_warning(const char* format, ...);
eiCORE_API void ei_info(const char* format, ...);
eiCORE_API void ei_debug(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
