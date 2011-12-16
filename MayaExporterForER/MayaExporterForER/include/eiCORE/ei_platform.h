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

#ifndef EI_PLATFORM_H
#define EI_PLATFORM_H

/** \brief Platform independent APIs for file I/O, multi-threading etc.
 * \file ei_platform.h
 * \author Bo Zhou
 * \def EI_DIR_SEPERATOR Users should always use '/' under any platforms.
 */

#include <eiCORE/ei_core.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ensure MIN, MAX available */
#ifdef EI_OS_WINDOWS
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* system configurations */
#define EI_MAX_MSG_GROUP_SIZE			1024
#define EI_MAX_OPEN_FILES				512
#define EI_MAX_FILE_NAME_LEN			512
#define EI_MAX_HOST_NAME_LEN			128
#define EI_MAX_REMOTE_PARAMS_LEN		128
#define EI_RENDER_THREAD_STACK_SIZE		(1024 * 1024 * 8)
#define EI_MAX_NUM_THREADS				32
#define EI_MAX_WAIT_TIME				500
#define EI_STACK_OFFSET					1024
#define EI_CACHE_LINE_SIZE				128
#define EI_DIR_SEPERATOR				'/'

typedef void *	eiModuleHandle;
typedef void *	eiSymbolHandle;
typedef FILE *	eiFileHandle;
typedef eiInt	(*eiFileEnumerator)(const char* name, void* param, const eiBool is_dir);

typedef enum eiFileMode {
	EI_FILE_READ = 0,
	EI_FILE_WRITE,
	EI_FILE_APPEND,
	EI_FILE_READ_UPDATE,
	EI_FILE_WRITE_UPDATE,
	EI_FILE_APPEND_UPDATE,
} eiFileMode;

#if defined EI_OS_WINDOWS

/* include socket header before windows.h to prevent from macro redefinitions */
#include <winsock2.h>
#include <windows.h>

#define eiTHREAD_FUNC			eiUint WINAPI
#define eiTHREAD_FUNC_RESULT	eiUint

typedef eiUint				eiTlsTag;
typedef HANDLE				eiThreadHandle;
typedef eiUint				(eiSTDCALL *eiThreadFunction)(void *param);
typedef CRITICAL_SECTION	eiLock;
typedef HANDLE				eiEvent;

typedef struct eiFileMap {
	HANDLE		hFileMapping;
	void		*data;
	eiSizet		length;
} eiFileMap;

#else

#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64 1
#endif

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64 1
#endif

#include <pthread.h>
#include <sys/stat.h>

#define eiTHREAD_FUNC			void *
#define eiTHREAD_FUNC_RESULT	void *

typedef pthread_key_t		eiTlsTag;
typedef pthread_t			eiThreadHandle;
typedef void *				(*eiThreadFunction)(void *param);
typedef pthread_mutex_t		eiLock;
typedef struct eiEvent {
	pthread_mutex_t		mutex;
	pthread_cond_t		condition;
	eiBool				signaled;
} eiEvent;

typedef struct eiFileMap {
	void		*padding;
	void		*data;
	eiUint64	length;
} eiFileMap;

#endif

/* endianness of platform */
enum {
	EI_LITTLE_ENDIAN = 0,
	EI_BIG_ENDIAN,
};

/** \brief Get endianness.
 */
eiCORE_API eiInt ei_get_endian();

/** \brief Byte-swap eiShort or eiUshort.
 */
eiCORE_API void ei_byteswap_short(void *v);

/** \brief Byte-swap eiInt or eiUint.
 */
eiCORE_API void ei_byteswap_int(void *v);

/** \brief Byte-swap eiLong or eiUlong.
 */
eiCORE_API void ei_byteswap_long(void *v);

/** \brief Byte-swap eiScalar.
 */
eiCORE_API void ei_byteswap_scalar(void *v);

/** \brief Byte-swap eiGeoScalar.
 */
eiCORE_API void ei_byteswap_geoscalar(void *v);


/** \brief Load a DSO by file path.
 */
eiCORE_API eiModuleHandle ei_load_module(const char* path);

/** \brief Get a symbol by name from a DSO.
 */
eiCORE_API eiSymbolHandle ei_get_symbol(eiModuleHandle module, const char* name);

/** \brief Unload a DSO.
 */
eiCORE_API void ei_free_module(eiModuleHandle module);


/** \brief Get the number of available threads.
 */
eiCORE_API eiUint ei_get_number_threads();

/** \brief Get the handle of the current thread in which this function is invoked.
 */
eiCORE_API eiThreadHandle ei_get_current_thread();

/** \brief Sleep current thread for a period in milliseconds.
 */
eiCORE_API void ei_sleep(const eiUint milliseconds);

/** \brief Fork a new thread.
 */
eiCORE_API eiThreadHandle ei_create_thread(eiThreadFunction func, void *param, eiUint32 *tid);

/** \brief Wait a thread to exit for infinite time.
 */
eiCORE_API void ei_wait_thread(eiThreadHandle thread);

/** \brief Wait multiple threads to exit for infinite time.
 */
eiCORE_API void ei_wait_multiple_threads(const eiUint num_threads, eiThreadHandle threads[]);

/** \brief Delete a thread.
 */
eiCORE_API void ei_delete_thread(eiThreadHandle thread);


/** \brief Create a mutex lock for synchronization.
 */
eiCORE_API void ei_create_lock(eiLock* lock);

/** \brief Delete a mutex lock.
 */
eiCORE_API void ei_delete_lock(eiLock* lock);

/** \brief Try to acquire a mutex lock.
 */
eiCORE_API eiBool ei_try_lock(eiLock* lock);

/** \brief Acquire a mutex lock.
 */
eiCORE_API void ei_lock(eiLock* lock);

/** \brief Release a mutex lock.
 */
eiCORE_API void ei_unlock(eiLock* lock);


/** \brief Create an event.
 */
eiCORE_API void ei_create_event(eiEvent* event);

/** \brief Delete an event.
 */
eiCORE_API void ei_delete_event(eiEvent* event);

/** \brief Signal an event, wake up waiting threads.
 */
eiCORE_API void ei_signal_event(eiEvent* event);

/** \brief Wait an event to be signaled for infinite time.
 */
eiCORE_API void ei_wait_event(eiEvent* event);

/** \brief Wait an event to be signaled with time-out in milliseconds.
 */
eiCORE_API void ei_time_wait_event(eiEvent* event, const eiUint milliseconds);


/** \brief Allocate a thread local storage(TLS) index.
 */
eiCORE_API eiTlsTag ei_tls_create();

/** \brief Set a value to a TLS slot of the calling thread by index.
 */
eiCORE_API void ei_tls_set(eiTlsTag tag, void* value);

/** \brief Get the value from a TLS slot of the calling thread by index.
 */
eiCORE_API void* ei_tls_get(eiTlsTag tag);

/** \brief Free a TLS index.
 */
eiCORE_API void ei_tls_delete(eiTlsTag tag);


#ifdef _DEBUG
#define ei_allocate(size)			ei_allocate_dbg(size, (const char *)__FILE__, __LINE__)
#define ei_free(ptr)				ei_free_dbg(ptr, (const char *)__FILE__, __LINE__)
#define ei_reallocate(ptr, newsize)	ei_reallocate_dbg(ptr, newsize, (const char *)__FILE__, __LINE__)
#else
#define ei_allocate(size)			ei_allocate_rel(size)
#define ei_free(ptr)				ei_free_rel(ptr)
#define ei_reallocate(ptr, newsize)	ei_reallocate_rel(ptr, newsize)
#endif

#define eiCHECK_FREE(p)		if ((p) != NULL) {\
								ei_free(p);\
								(p) = NULL;\
							}

/** \brief Allocate a block of memory on heap with debug information.
 */
eiCORE_API void *ei_allocate_dbg(const eiUint size, const char *file, eiInt line);

/** \brief Free the memory block allocated on heap with debug information.
 */
eiCORE_API void ei_free_dbg(void *ptr, const char *file, eiInt line);

/** \brief Re-allocate memory block to a new size on heap with debug information,
 * existing content in the old block will be copied or truncated, it's considered
 * to be more efficient than malloc, memcpy, free sequence.
 */
eiCORE_API void *ei_reallocate_dbg(void *ptr, const eiUint newsize, const char *file, eiInt line);

/** \brief Allocate a block of memory on heap for release mode.
 */
eiCORE_API void *ei_allocate_rel(const eiUint size);

/** \brief Free the memory block allocated on heap for release mode.
 */
eiCORE_API void ei_free_rel(void *ptr);

/** \brief Re-allocate memory block to a new size on heap for release mode,
 * existing content in the old block will be copied or truncated, it's considered
 * to be more efficient than malloc, memcpy, free sequence.
 */
eiCORE_API void *ei_reallocate_rel(void *ptr, const eiUint newsize);


/** \brief Get the current time in milliseconds.
 * \warning In glibc, clock_t is defined as size_t which is 8-bytes long under
 * x86_64, but it's always of 4 bytes length under Windows.
 */
eiCORE_API eiInt ei_get_time();


/** \brief Get the value of an environment variable by name.
 */
eiCORE_API void ei_get_environment(char* str, const char* name);


/** \brief Check whether a file exsits by name.
 */
eiCORE_API eiBool ei_file_exists(const char* path);

/** \brief Open a file by name.
 */
eiCORE_API eiFileHandle ei_open_file(const char* path, const eiFileMode mode);

/** \brief Read a file.
 */
eiCORE_API eiSizet ei_read_file(eiFileHandle file, void* buf, const eiSizet size);

/** \brief Write a file.
 */
eiCORE_API eiSizet ei_write_file(eiFileHandle file, const void* buf, const eiSizet size);

/** \brief Seek a file.
 */
eiCORE_API eiInt64 ei_seek_file(eiFileHandle file, const eiInt64 offset);

/** \brief Get the offset of current file pointer.
 */
eiCORE_API eiInt64 ei_get_file_offset(eiFileHandle file);

/** \brief Close a file.
 */
eiCORE_API void ei_close_file(eiFileHandle file);

/** \brief Delete a file by name.
 */
eiCORE_API void ei_delete_file(const char* path);

/** \brief Generate a full path to a temporary file in specified directory.
 */
eiCORE_API void ei_get_temp_filename(char *result, const char* dir, const char* prefix);

/** \brief Append file name to a specified directory.
 */
eiCORE_API void ei_append_filename(char* result, const char* dir, const char* filename);


/** \brief Create a new directory.
 */
eiCORE_API void ei_create_directory(const char* dir);

/** \brief Remove a directory.
 */
eiCORE_API void ei_delete_directory(const char* dir);

/** \brief Enumerate all files and directories in a directory.
 */
eiCORE_API void ei_enumerate(const char* dir, eiFileEnumerator func, void* param);

/** \brief Get current directory of the executable.
 */
eiCORE_API void ei_get_current_directory(char* dir);

/** \brief Split name into partA[splitter]partB format.
 */
eiCORE_API void ei_split_name(
	char *partA, 
	char *partB, 
	const char *name, 
	const eiSizet max_len, 
	const char splitter);

/** \brief Extract the file extension from a file name.
 */
eiCORE_API void ei_get_file_extension(char* ext, const char* name);

/** \brief Get file length.
 * \warning We only support 64-bit file operations.
 */
eiCORE_API eiUint64 ei_get_file_length(eiFileHandle file);

/** \brief Extend or truncate a file to a new size.
 */
eiCORE_API void ei_truncate_file(eiFileHandle file, const eiUint64 size);


/** \brief Get the page size of current system.
 */
eiCORE_API eiSizet ei_get_page_size();

/** \brief Map a portion of a file into memory.
 */
eiCORE_API void ei_map_file(eiFileMap* file_map, eiFileHandle file, const eiFileMode mode, const eiUint64 offset, const eiSizet length);

/** \brief Unmap a portion of a file from memory.
 */
eiCORE_API void ei_unmap_file(eiFileMap* file_map);

#ifdef __cplusplus
}
#endif

#endif
