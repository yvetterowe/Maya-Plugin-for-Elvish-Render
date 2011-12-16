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

#include <eiCORE/ei_platform.h>
#include <eiCORE/ei_assert.h>

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef EI_OS_WINDOWS

#include <io.h>
#include <direct.h>
#include <process.h> /* _beginthread / _endthread */

#define fseeko64	_fseeki64
#define ftello64	_ftelli64

eiModuleHandle ei_load_module(const char* path)
{
	return LoadLibrary(path);
}

eiSymbolHandle ei_get_symbol(eiModuleHandle module, const char* name)
{
	if (module == NULL)
	{
		return NULL;
	}

	return GetProcAddress((HMODULE)module, (LPCSTR)name);
}

void ei_free_module(eiModuleHandle module)
{
	if (module == NULL)
	{
		return;
	}

	FreeLibrary((HMODULE)module);
}

eiUint ei_get_number_threads()
{
	SYSTEM_INFO sys_info;

	memset(&sys_info, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&sys_info);

	return sys_info.dwNumberOfProcessors;
}

eiThreadHandle ei_get_current_thread()
{
	return GetCurrentThread();
}

void ei_sleep(const eiUint milliseconds)
{
	Sleep(milliseconds);
}

eiThreadHandle ei_create_thread(eiThreadFunction func, void *param, eiUint *tid)
{
	eiThreadHandle thread;
	eiUint _tid;

	thread = (eiThreadHandle)_beginthreadex(NULL, EI_RENDER_THREAD_STACK_SIZE, func, param, 0, &_tid);

	if (tid != NULL)
	{
		*tid = _tid;
	}

	return thread;
}

void ei_wait_thread(eiThreadHandle thread)
{
	WaitForSingleObject(thread, INFINITE);
}

void ei_wait_multiple_threads(const eiUint num_threads, eiThreadHandle threads[])
{
	WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
}

void ei_delete_thread(eiThreadHandle thread)
{
	CloseHandle(thread);
}

void ei_create_lock(eiLock* lock)
{
	InitializeCriticalSection(lock);
}

void ei_delete_lock(eiLock* lock)
{
	DeleteCriticalSection(lock);
}

eiBool ei_try_lock(eiLock* lock)
{
	return TryEnterCriticalSection(lock);
}

void ei_lock(eiLock* lock)
{
	EnterCriticalSection(lock);
}

void ei_unlock(eiLock* lock)
{
	LeaveCriticalSection(lock);
}

void ei_create_event(eiEvent* event)
{
	*event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void ei_delete_event(eiEvent* event)
{
	CloseHandle(*event);
}

void ei_signal_event(eiEvent* event)
{
	SetEvent(*event);
}

void ei_wait_event(eiEvent* event)
{
	WaitForSingleObject(*event, INFINITE);
}

void ei_time_wait_event(eiEvent* event, const eiUint milliseconds)
{
	WaitForSingleObject(*event, milliseconds);
}

eiTlsTag ei_tls_create()
{
	return TlsAlloc();
}

void ei_tls_set(eiTlsTag tag, void* value)
{
	TlsSetValue(tag, value);
}

void* ei_tls_get(eiTlsTag tag)
{
	return TlsGetValue(tag);
}

void ei_tls_delete(eiTlsTag tag)
{
	TlsFree(tag);
}

void ei_create_directory(const char* dir)
{
	mkdir(dir);
}

void ei_enumerate(const char* dir, eiFileEnumerator func, void* param)
{
	struct _finddata_t c_file;
	intptr_t hFile;

	if ((hFile = _findfirst(dir, &c_file)) != -1L)
	{
		do
		{
			if (!func(c_file.name, param, (c_file.attrib & _A_SUBDIR)))
			{
				break;
			}
		} while (_findnext(hFile, &c_file) == 0);
		_findclose(hFile);
	}
}

void ei_get_current_directory(char* dir)
{
	size_t pathlen = 0;

	GetModuleFileName(NULL, dir, EI_MAX_FILE_NAME_LEN - 1);

	pathlen = strlen(dir);

	while (1)
	{
		if (dir[pathlen--] == '\\')
		{
			break;
		}
	}

	dir[++pathlen] = '\0';
}

void ei_truncate_file(eiFileHandle file, const eiUint64 size)
{
	DWORD max_length_low, max_length_high;
	HANDLE hFile, hFileMapping;

	max_length_high = (DWORD)(size >> 32);
	max_length_low = (DWORD)(size & 0xFFFFFFFF);

	hFile = (HANDLE)_get_osfhandle(_fileno(file));
	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, max_length_high, max_length_low, NULL);

	CloseHandle(hFileMapping);
}

eiSizet ei_get_page_size()
{
	SYSTEM_INFO sys_info;

	memset(&sys_info, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&sys_info);

	return sys_info.dwAllocationGranularity;
}

void ei_map_file(eiFileMap* file_map, eiFileHandle file, const eiFileMode mode, const eiUint64 offset, const eiSizet length)
{
	DWORD mappingMode = PAGE_READWRITE;
	DWORD viewMode = FILE_MAP_READ | FILE_MAP_WRITE;

	eiUint64 max_length = offset + length;

	DWORD offset_high = (DWORD)(offset >> 32);
	DWORD offset_low = (DWORD)(offset & 0xFFFFFFFF);
	DWORD max_length_high = (DWORD)(max_length >> 32);
	DWORD max_length_low = (DWORD)(max_length & 0xFFFFFFFF);

	HANDLE hFile;

	switch (mode)
	{
	case EI_FILE_READ:
		mappingMode = PAGE_READONLY;
		viewMode = FILE_MAP_READ;
		break;
	case EI_FILE_WRITE:
		mappingMode = PAGE_READWRITE;
		viewMode = FILE_MAP_WRITE;
		break;
	case EI_FILE_APPEND:
		mappingMode = PAGE_READWRITE;
		viewMode = FILE_MAP_WRITE;
		break;
	case EI_FILE_READ_UPDATE:
		mappingMode = PAGE_READWRITE;
		viewMode = FILE_MAP_READ | FILE_MAP_WRITE;
		break;
	case EI_FILE_WRITE_UPDATE:
		mappingMode = PAGE_READWRITE;
		viewMode = FILE_MAP_READ | FILE_MAP_WRITE;
		break;
	case EI_FILE_APPEND_UPDATE:
		mappingMode = PAGE_READWRITE;
		viewMode = FILE_MAP_READ | FILE_MAP_WRITE;
		break;
	default:
		break;
	}

	hFile = (HANDLE)_get_osfhandle(_fileno(file));
	file_map->hFileMapping = CreateFileMapping(hFile, NULL, mappingMode, max_length_high, max_length_low, NULL);
	
	file_map->data = (eiByte *)MapViewOfFile(file_map->hFileMapping, viewMode, offset_high, offset_low, length);
	file_map->length = length;
}

void ei_unmap_file(eiFileMap* file_map)
{
	UnmapViewOfFile((LPVOID)file_map->data);
	CloseHandle(file_map->hFileMapping);
}

#else

#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <glob.h>

eiModuleHandle ei_load_module(const char* path)
{
	return dlopen(path, RTLD_NOW);
}

eiSymbolHandle ei_get_symbol(eiModuleHandle module, const char* name)
{
	if (module == NULL)
	{
		return NULL;
	}

	return dlsym(module, name);
}

void ei_free_module(eiModuleHandle module)
{
	if (module == NULL)
	{
		return;
	}
	
	dlclose(module);
}

eiUint ei_get_number_threads()
{
	eiUint number = 0;
	FILE* fp = NULL;
	char buf[128];

	fp = fopen("/proc/cpuinfo", "r");

	while (fgets(buf, 127, fp))
	{
		if (strstr(buf, "processor"))
		{
			++number;
		}
	}

	fclose(fp);

	return number;
}

eiThreadHandle ei_get_current_thread()
{
	return pthread_self();
}

void ei_sleep(const eiUint milliseconds)
{
	usleep(milliseconds * 1000);
}

eiThreadHandle ei_create_thread(eiThreadFunction func, void *param, eiUint *tid)
{
	eiThreadHandle thread;
	eiUint _tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, EI_RENDER_THREAD_STACK_SIZE);
	_tid = pthread_create(&thread, &attr, func, param);
	pthread_attr_destroy(&attr);

	if (tid != NULL)
	{
		*tid = _tid;
	}

	return thread;
}

void ei_wait_thread(eiThreadHandle thread)
{
	pthread_join(thread, NULL);
}

void ei_wait_multiple_threads(const eiUint num_threads, eiThreadHandle threads[])
{
	eiUint i;

	for(i = 0; i < num_threads; ++i)
	{
		ei_wait_thread(threads[i]);
	}
}

void ei_delete_thread(eiThreadHandle thread)
{
	/* nothing to do */
}

void ei_create_lock(eiLock* lock)
{
	pthread_mutex_init(lock, NULL);
}

void ei_delete_lock(eiLock* lock)
{
	pthread_mutex_destroy(lock);
}

eiBool ei_try_lock(eiLock* lock)
{
	return (pthread_mutex_trylock(lock) == 0);
}

void ei_lock(eiLock* lock)
{
	pthread_mutex_lock(lock);
}

void ei_unlock(eiLock* lock)
{
	pthread_mutex_unlock(lock);
}

void ei_create_event(eiEvent* event)
{
	pthread_mutex_init(&event->mutex, NULL);
	pthread_cond_init(&event->condition, NULL);
	event->signaled = eiFALSE;
}

void ei_delete_event(eiEvent* event)
{
	pthread_cond_destroy(&event->condition);
	pthread_mutex_destroy(&event->mutex);
}

void ei_signal_event(eiEvent* event)
{
	pthread_mutex_lock(&event->mutex);
	event->signaled = eiTRUE;
	pthread_cond_signal(&event->condition);
	pthread_mutex_unlock(&event->mutex);
}

void ei_wait_event(eiEvent* event)
{
	pthread_mutex_lock(&event->mutex);
	/* It's said that Linux wait has fake wake-up issues,
	   so it seems better to use while rather than if here
	   for infinite waits. */
	while (!event->signaled)
	{
		pthread_cond_wait(&event->condition, &event->mutex);
	}
	/* Auto-reset the signaled status. */
	event->signaled = eiFALSE;
	pthread_mutex_unlock(&event->mutex);
}

void ei_time_wait_event(eiEvent* event, const eiUint milliseconds)
{
	struct timespec timeout;
	struct timeval tp;

	gettimeofday(&tp, NULL);

	/* Convert from timeval to timespec */
	timeout.tv_sec = (milliseconds / 1000) + tp.tv_sec;
	timeout.tv_nsec = ((milliseconds % 1000) * 1000000) + (tp.tv_usec * 1000);

	while (timeout.tv_nsec >= 1000000000)
	{
		timeout.tv_nsec -= 1000000000;
		timeout.tv_sec++;
	}

	pthread_mutex_lock( &event->mutex );
	if (!event->signaled)
	{
		pthread_cond_timedwait(&event->condition, &event->mutex, &timeout);
	}
	/* Auto-reset the signaled status. */
	event->signaled = eiFALSE;
	pthread_mutex_unlock(&event->mutex);
}

eiTlsTag ei_tls_create()
{
	eiTlsTag tag;

	pthread_key_create(&tag, NULL);

	return tag;
}

void ei_tls_set(eiTlsTag tag, void* value)
{
	pthread_setspecific(tag, value);
}

void* ei_tls_get(eiTlsTag tag)
{
	return pthread_getspecific(tag);
}

void ei_tls_delete(eiTlsTag tag)
{
	pthread_key_delete(tag);
}

void ei_create_directory(const char* dir)
{
	mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void ei_enumerate(const char* dir, eiFileEnumerator func, void* param)
{
	eiInt i;
	glob_t globbuf;
	struct stat stbuf;

	globbuf.gl_offs = 0;
	glob(dir, GLOB_DOOFFS, NULL, &globbuf);

	for (i = 0; i < globbuf.gl_pathc; ++i)
	{
		stat(globbuf.gl_pathv[i], &stbuf);

		if (!func(globbuf.gl_pathv[i], param, S_ISDIR(stbuf.st_mode)))
		{
			break;
		}
	}

	globfree(&globbuf);
}

void ei_get_current_directory(char* dir)
{
	char* pwd = getenv("PWD");
	if (pwd != NULL)
	{
		strncpy(dir, pwd, EI_MAX_FILE_NAME_LEN - 1);
	}
}

void ei_truncate_file(eiFileHandle file, const eiUint64 size)
{
	ftruncate64(fileno(file), size);
}

eiSizet ei_get_page_size()
{
	return sysconf(_SC_PAGESIZE);
}

void ei_map_file(eiFileMap* file_map, eiFileHandle file, const eiFileMode mode, const eiUint64 offset, const eiSizet length)
{
	eiInt protMode = PROT_READ | PROT_WRITE;

	switch (mode)
	{
	case EI_FILE_READ:
		protMode = PROT_READ;
		break;
	case EI_FILE_WRITE:
		protMode = PROT_WRITE;
		break;
	case EI_FILE_APPEND:
		protMode = PROT_WRITE;
		break;
	case EI_FILE_READ_UPDATE:
		protMode = PROT_READ | PROT_WRITE;
		break;
	case EI_FILE_WRITE_UPDATE:
		protMode = PROT_READ | PROT_WRITE;
		break;
	case EI_FILE_APPEND_UPDATE:
		protMode = PROT_READ | PROT_WRITE;
		break;
	default:
		break;
	}

	file_map->data = mmap64(NULL, length, protMode, MAP_SHARED, fileno(file), offset);
	file_map->length = length;
}

void ei_unmap_file(eiFileMap* file_map)
{
	munmap((void *)file_map->data, file_map->length);
}

#endif

eiInt ei_get_endian()
{
#ifdef EI_OS_LITTLE_ENDIAN
	return EI_LITTLE_ENDIAN;
#else
	return EI_BIG_ENDIAN;
#endif
}

void ei_byteswap_short(void *v)
{
	eiShort result;
	eiByte *dst = (eiByte *)&result;
	eiByte *src = (eiByte *)&v;
	dst[0] = src[1];
	dst[1] = src[0];
	*((eiShort *)v) = result;
}

void ei_byteswap_int(void *v)
{
	eiInt result;
	eiByte *dst = (eiByte *)&result;
	eiByte *src = (eiByte *)&v;
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	*((eiInt *)v) = result;
}

void ei_byteswap_long(void *v)
{
	eiLong result;
	eiByte *dst = (eiByte *)&result;
	eiByte *src = (eiByte *)&v;
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	dst[4] = src[7];
	dst[5] = src[6];
	dst[6] = src[5];
	dst[7] = src[4];
	*((eiLong *)v) = result;
}

void ei_byteswap_scalar(void *v)
{
	eiScalar result;
	eiByte *dst = (eiByte *)&result;
	eiByte *src = (eiByte *)&v;
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	*((eiScalar *)v) = result;
}

void ei_byteswap_geoscalar(void *v)
{
	eiGeoScalar result;
	eiByte *dst = (eiByte *)&result;
	eiByte *src = (eiByte *)&v;
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	dst[4] = src[7];
	dst[5] = src[6];
	dst[6] = src[5];
	dst[7] = src[4];
	*((eiGeoScalar *)v) = result;
}

void *ei_allocate_dbg(const eiUint size, const char *file, eiInt line)
{
	void *ptr;

	if (size == 0)
	{
		eiASSERT(0);
		return NULL;
	}
	
	ptr = malloc(size);
	eiASSERT(ptr != NULL);

	return ptr;
}

void ei_free_dbg(void *ptr, const char *file, eiInt line)
{
	if (ptr == NULL)
	{
		eiASSERT(0);
		return;
	}

	free(ptr);
}

void *ei_reallocate_dbg(void *ptr, const eiUint newsize, const char *file, eiInt line)
{
	void *newptr;

	if (ptr == NULL || newsize == 0)
	{
		eiASSERT(0);
		return NULL;
	}

	newptr = realloc(ptr, newsize);
	eiASSERT(newptr != NULL);

	return newptr;
}

void *ei_allocate_rel(const eiUint size)
{
	void *ptr;

	if (size == 0)
	{
		eiASSERT(0);
		return NULL;
	}
	
	ptr = malloc(size);
	eiASSERT(ptr != NULL);

	return ptr;
}

void ei_free_rel(void *ptr)
{
	if (ptr == NULL)
	{
		eiASSERT(0);
		return;
	}

	free(ptr);
}

void *ei_reallocate_rel(void *ptr, const eiUint newsize)
{
	void *newptr;

	if (ptr == NULL || newsize == 0)
	{
		eiASSERT(0);
		return NULL;
	}

	newptr = realloc(ptr, newsize);
	eiASSERT(newptr != NULL);

	return newptr;
}

eiInt ei_get_time()
{
	return (eiInt)clock();
}

void ei_get_environment(char* str, const char* name)
{
	char *value = getenv(name);

	if (value != NULL)
	{
		strcpy(str, value);
	}
}

eiBool ei_file_exists(const char* name)
{
	if (name == NULL || strcmp(name, "") == 0)
	{
		return eiFALSE;
	}

	if (access(name, 0) == 0)
	{
		return eiTRUE;
	}

	return eiFALSE;
}

eiFileHandle ei_open_file(const char* path, const eiFileMode mode)
{
	switch (mode)
	{
	case EI_FILE_READ:
		return fopen(path, "rb");
	case EI_FILE_WRITE:
		return fopen(path, "wb");
	case EI_FILE_APPEND:
		return fopen(path, "ab");
	case EI_FILE_READ_UPDATE:
		return fopen(path, "r+b");
	case EI_FILE_WRITE_UPDATE:
		return fopen(path, "w+b");
	case EI_FILE_APPEND_UPDATE:
		return fopen(path, "a+b");
	default:
		break;
	}

	return NULL;
}

eiSizet ei_read_file(eiFileHandle file, void* buf, const eiSizet size)
{
	return fread(buf, 1, size, file);
}

eiSizet ei_write_file(eiFileHandle file, const void* buf, const eiSizet size)
{
	return fwrite(buf, 1, size, file);
}

eiInt64 ei_seek_file(eiFileHandle file, const eiInt64 offset)
{
	return fseeko64(file, offset, SEEK_SET);
}

eiInt64 ei_get_file_offset(eiFileHandle file)
{
	return (eiInt64)ftello64(file);
}

void ei_close_file(eiFileHandle file)
{
	if (file != NULL)
	{
		fclose(file);
	}
}

void ei_delete_file(const char* path)
{
	unlink(path);
}

#define RAND_CHAR (char)((rand() % 26) + (int)('a'))

void ei_get_temp_filename(char* result, const char* dir, const char* prefix)
{
	eiSizet dirLen = strlen(dir);

	if (dir[dirLen - 1] != '/' && dir[dirLen - 1] != '\\')
	{
		sprintf(result, "%s%c%s_%c%c%c%c%c%c%c%cXXXXXX", dir, EI_DIR_SEPERATOR, prefix,
				RAND_CHAR, RAND_CHAR, RAND_CHAR, RAND_CHAR,
				RAND_CHAR, RAND_CHAR, RAND_CHAR, RAND_CHAR);
	}
	else
	{
		sprintf(result, "%s%s_%c%c%c%c%c%c%c%cXXXXXX", dir, prefix,
				RAND_CHAR, RAND_CHAR, RAND_CHAR, RAND_CHAR,
				RAND_CHAR, RAND_CHAR, RAND_CHAR, RAND_CHAR);
	}

	mktemp(result);
}

void ei_append_filename(char* result, const char* dir, const char* filename)
{
	eiSizet dirLen = strlen(dir);

	if (dirLen == 0)
	{
		strcpy(result, filename);
		return;
	}

	if (dir[dirLen - 1] != '/' && dir[dirLen - 1] != '\\')
	{
		sprintf(result, "%s%c%s", dir, EI_DIR_SEPERATOR, filename);
	}
	else
	{
		sprintf(result, "%s%s", dir, filename);
	}
}

void ei_delete_directory(const char* path)
{
	rmdir(path);
}

void ei_split_name(
	char *partA, 
	char *partB, 
	const char *name, 
	const eiSizet max_len, 
	const char splitter)
{
	eiSizet	i;
	eiSizet	len = strlen(name);

	for (i = 0; i < len; ++i)
	{
		if (name[i] == splitter)
		{
			strncpy(partA, name, MIN(i, max_len - 1));
			partA[i] = '\0';
			strncpy(partB, name + i + 1, max_len - 1);
			return;
		}
	}

	memset(partA, 0, max_len);
	strncpy(partB, name, max_len - 1);
}

void ei_get_file_extension(char* ext, const char* name)
{
	eiIntptr i; /* we must use signed integer for decreasing loop */
	eiSizet len = strlen(name);

	for (i = (len - 1); i >= 0; --i)
	{
		if (name[i] == '.')
		{
			strncpy(ext, name + i + 1, EI_MAX_FILE_NAME_LEN - 1);
			return;
		}
	}
}

eiUint64 ei_get_file_length(eiFileHandle file)
{
	eiInt64 current_pos;
	eiUint64 file_len;

	current_pos = ftello64(file);

	fseeko64(file, 0, SEEK_END);
	file_len = (eiUint64)ftello64(file);

	fseeko64(file, current_pos, SEEK_SET);

	return file_len;
}
