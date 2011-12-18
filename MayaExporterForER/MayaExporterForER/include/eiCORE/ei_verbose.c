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

#include <eiCORE/ei_verbose.h>
#include <eiCORE/ei_platform.h>

#include <stdarg.h>
#include <stdio.h>

/* global variable for controlling verbosity */
#ifdef _DEBUG
eiInt g_verbose_level = EI_VERBOSE_ALL;
#else
eiInt g_verbose_level = EI_VERBOSE_INFO;
#endif
/* global verbosity print callback */
ei_verbose_print g_verbose_print = NULL;
/* the user parameter of verbosity print callback */
void *g_verbose_params = NULL;
/* global lock for output stream, used for multi-threading */
eiLock g_verbose_lock;

void ei_verbose_init()
{
	ei_create_lock(&g_verbose_lock);
	g_verbose_print = NULL;
	g_verbose_params = NULL;
}

void ei_verbose_exit()
{
	ei_delete_lock(&g_verbose_lock);
}

void ei_verbose_set(const eiInt level)
{
	ei_lock(&g_verbose_lock);
	
	g_verbose_level = level;

	ei_unlock(&g_verbose_lock);
}

eiInt ei_verbose_get()
{
	eiInt result = EI_VERBOSE_NONE;

	ei_lock(&g_verbose_lock);

	result = g_verbose_level;

	ei_unlock(&g_verbose_lock);

	return result;
}

void ei_verbose_callback(ei_verbose_print print, void *params)
{
	ei_lock(&g_verbose_lock);
	
	g_verbose_print = print;
	g_verbose_params = params;

	ei_unlock(&g_verbose_lock);
}

eiBool ei_verbose_check_callback()
{
	eiBool	result = eiFALSE;

	ei_lock(&g_verbose_lock);
	
	if (g_verbose_print != NULL)
	{
		result = eiTRUE;
	}

	ei_unlock(&g_verbose_lock);

	return result;
}

void ei_fatal(const char* format, ...)
{
	va_list args;

	ei_lock(&g_verbose_lock);
	
	if (g_verbose_level < EI_VERBOSE_FATAL)
	{
		ei_unlock(&g_verbose_lock);
		return;
	}
	
	va_start(args, format);

	if (g_verbose_print != NULL)
	{
		char	szALine[ EI_MAX_MSG_LEN ];

		vsprintf(szALine, format, args);
		g_verbose_print(EI_VERBOSE_FATAL, szALine, g_verbose_params);
	}
	else
	{
		fprintf(stderr, "FATAL> ");
		vfprintf(stderr, format, args);
	}

	va_end(args);

	ei_unlock(&g_verbose_lock);
}

void ei_error(const char* format, ...)
{
	va_list args;

	ei_lock(&g_verbose_lock);

	if (g_verbose_level < EI_VERBOSE_ERROR)
	{
		ei_unlock(&g_verbose_lock);
		return;
	}
	
	va_start(args, format);

	if (g_verbose_print != NULL)
	{
		char	szALine[ EI_MAX_MSG_LEN ];

		vsprintf(szALine, format, args);
		g_verbose_print(EI_VERBOSE_ERROR, szALine, g_verbose_params);
	}
	else
	{
		fprintf(stderr, "ERROR> ");
		vfprintf(stderr, format, args);
	}

	va_end(args);

	ei_unlock(&g_verbose_lock);
}

void ei_warning(const char* format, ...)
{
	va_list args;

	ei_lock(&g_verbose_lock);

	if (g_verbose_level < EI_VERBOSE_WARNING)
	{
		ei_unlock(&g_verbose_lock);
		return;
	}
	
	va_start(args, format);

	if (g_verbose_print != NULL)
	{
		char	szALine[ EI_MAX_MSG_LEN ];

		vsprintf(szALine, format, args);
		g_verbose_print(EI_VERBOSE_WARNING, szALine, g_verbose_params);
	}
	else
	{
		fprintf(stderr, "WARNING> ");
		vfprintf(stderr, format, args);
	}

	va_end(args);

	ei_unlock(&g_verbose_lock);
}

void ei_info(const char* format, ...)
{
	va_list args;

	ei_lock(&g_verbose_lock);
	
	if (g_verbose_level < EI_VERBOSE_INFO)
	{
		ei_unlock(&g_verbose_lock);
		return;
	}
	
	va_start(args, format);

	if (g_verbose_print != NULL)
	{
		char	szALine[ EI_MAX_MSG_LEN ];

		vsprintf(szALine, format, args);
		g_verbose_print(EI_VERBOSE_INFO, szALine, g_verbose_params);
	}
	else
	{
		fprintf(stdout, "INFO> ");
		vfprintf(stdout, format, args);
	}

	va_end(args);

	ei_unlock(&g_verbose_lock);
}

void ei_debug(const char* format, ...)
{
	va_list args;

	ei_lock(&g_verbose_lock);
	
	if (g_verbose_level < EI_VERBOSE_DEBUG)
	{
		ei_unlock(&g_verbose_lock);
		return;
	}
	
	va_start(args, format);

	if (g_verbose_print != NULL)
	{
		char	szALine[ EI_MAX_MSG_LEN ];

		vsprintf(szALine, format, args);
		g_verbose_print(EI_VERBOSE_DEBUG, szALine, g_verbose_params);
	}
	else
	{
		fprintf(stdout, "DEBUG> ");
		vfprintf(stdout, format, args);
	}

	va_end(args);

	ei_unlock(&g_verbose_lock);
}
