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

#ifndef EI_FILESYS_H
#define EI_FILESYS_H

/** \brief A generic file system which manages maximum open file handles 
 * for transparent accesses to files.
 * \file ei_filesys.h
 * \author Elvic Liang
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief A generic file representation. */
typedef struct eiFile			eiFile;
/** \brief A file system for managing maximum open file handles. */
typedef struct eiFileSystem		eiFileSystem;

/** \brief Get the resolved filename.
 */
eiCORE_API const char *ei_file_get_name(eiFile *file);

/** \brief Get current offset of file pointer.
 */
eiCORE_API eiInt64 ei_file_get_offset(eiFile *file);

/** \brief Get current file length using platform API.
 */
eiCORE_API eiUint64 ei_file_get_length(eiFile *file);

/** \brief Truncate the file.
 */
eiCORE_API void ei_file_truncate(eiFile *file, 
									const eiUint64 size);

/** \brief Move current file pointer to another offset.
 */
eiCORE_API void ei_file_seek(eiFile *file, 
								const eiInt64 offset);

/** \brief Read a chunk of data.
 */
eiCORE_API void ei_file_read(eiFile *file, 
								void *buf, 
								const eiSizet size);

/** \brief Write a chunk of data.
 */
eiCORE_API void ei_file_write(eiFile *file, 
								 const void *buf, 
								 const eiSizet size);

/** \brief Initialize a file system.
 * @param max_open_files The maximum number of open file handles.
 */
eiCORE_API void ei_filesys_init(eiFileSystem *filesys, 
								   const eiUint max_open_files);

/** \brief Cleanup a file system.
 */
eiCORE_API void ei_filesys_exit(eiFileSystem *filesys);

/** \brief Lock the file system. Users must call the lock/unlock 
 * functions for any block of operations with this file system.
 */
eiCORE_API void ei_filesys_lock(eiFileSystem *filesys);

/** \brief Unlock the file system.
 */
eiCORE_API void ei_filesys_unlock(eiFileSystem *filesys);

/** \brief Create a temporary file, on closing, it will be deleted 
 * deleted automatically.
 */
eiCORE_API eiFile *ei_filesys_create_temp(eiFileSystem *filesys, 
											 const eiUint64 max_length);

/** \brief Open a file by name with certain mode, it should be deleted 
 * manually if needed.
 */
eiCORE_API eiFile *ei_filesys_open(eiFileSystem *filesys, 
									  const char *filename, 
									  const eiFileMode mode);

/** \brief Close a file, temporary file will be deleted automatically.
 */
eiCORE_API eiBool ei_filesys_close(eiFileSystem *filesys, 
									  eiFile *pFile);

/** \brief Get the number of files managed by this file system.
 */
eiCORE_API eiSizet ei_filesys_num_files(eiFileSystem *filesys);

/** \brief Get a file handle by index.
 */
eiCORE_API eiFile *ei_filesys_get_file(eiFileSystem *filesys, 
										  const eiSizet index);

#ifdef __cplusplus
}
#endif

#endif
