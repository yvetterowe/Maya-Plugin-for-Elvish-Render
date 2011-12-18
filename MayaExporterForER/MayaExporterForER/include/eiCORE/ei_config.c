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

#include <eiCORE/ei_config.h>
#include <eiCORE/ei_dataflow.h>
#include <eiCORE/ei_assert.h>

#define EI_MAX_CONFIG_COMMAND_LEN		128
#define EI_MAX_CONFIG_PARAM_LEN			512

void ei_config_init(eiConfig *config)
{
	config->nthreads = 0;
	config->memlimit = EI_DEFAULT_MEMORY_LIMIT;
	config->distributed = eiTRUE;
	config->port = 6666;
	config->maxclients = 5;
	
	ei_array_init(&config->servers, sizeof(eiHostDesc));
	ei_array_init(&config->searchpaths, sizeof(eiSearchPath));
}

void ei_config_exit(eiConfig *config)
{
	ei_array_clear(&config->searchpaths);
	ei_array_clear(&config->servers);
}

eiBool ei_config_load(eiConfig *config, const char *filename)
{
	FILE		*fp;
	char		command[ EI_MAX_CONFIG_COMMAND_LEN ];
	char		sval[ EI_MAX_CONFIG_PARAM_LEN ];
	eiInt		ival;
	eiIntptr	i;

	fp = fopen(filename, "r");
	
	if (fp == NULL)
	{
		return eiFALSE;
	}

	while (!feof(fp))
	{
		memset(command, 0, sizeof(command));
		memset(sval, 0, sizeof(sval));
		ival = 0;

		if (fscanf(fp, "%s", command) <= 0)
		{
			break;
		}

		if (strcmp(command, "nthreads") == 0)
		{
			if (fscanf(fp, "%s", sval) <= 0)
			{
				break;
			}

			if (strcmp(sval, "auto") == 0)
			{
				config->nthreads = 0;
			}
			else
			{
				config->nthreads = atoi(sval);
			}
		}
		else if (strcmp(command, "memlimit") == 0)
		{
			if (fscanf(fp, "%d", &ival) <= 0)
			{
				break;
			}

			config->memlimit = MAX(0, ival);
		}
		else if (strcmp(command, "distributed") == 0)
		{
			if (fscanf(fp, "%s", sval) <= 0)
			{
				break;
			}

			if (strcmp(sval, "on") == 0)
			{
				config->distributed = eiTRUE;
			}
			else
			{
				config->distributed = eiFALSE;
			}
		}
		else if (strcmp(command, "port") == 0)
		{
			if (fscanf(fp, "%d", &ival) <= 0)
			{
				break;
			}

			config->port = (eiUshort)ival;
		}
		else if (strcmp(command, "maxclients") == 0)
		{
			if (fscanf(fp, "%d", &ival) <= 0)
			{
				break;
			}

			config->maxclients = ival;
		}
		else if (strcmp(command, "server") == 0)
		{
			eiHostDesc	host_desc;
			char		port_str[ EI_MAX_HOST_NAME_LEN ];

			if (fscanf(fp, "%s", sval) <= 0)
			{
				break;
			}

			ei_split_name(host_desc.host_name, port_str, sval, EI_MAX_HOST_NAME_LEN, ':');

			if (strlen(host_desc.host_name) > 0)
			{
				host_desc.port_number = atoi(port_str);

				ei_array_push_back(&config->servers, &host_desc);
			}
		}
		else if (strcmp(command, "searchpath") == 0)
		{
			eiSearchPath	searchpath;
			char			*pstr;
			eiIntptr		i;

			if (fgets(sval, sizeof(sval), fp) == NULL)
			{
				break;
			}

			/* look for the first non-whitespace character */
			pstr = sval;
			while ((*pstr == ' ' || *pstr == '\t') && *pstr != '\0')
			{
				++pstr;
			}

			memset(&searchpath, 0, sizeof(searchpath));
			strncpy(searchpath.path, pstr, EI_MAX_FILE_NAME_LEN - 1);

			/* clean return characters */
			for (i = strlen(searchpath.path) - 1; i >= 0; --i)
			{
				if (searchpath.path[i] == '\r' || searchpath.path[i] == '\n')
				{
					searchpath.path[i] = '\0';
				}
				else
				{
					break;
				}
			}

			if (strlen(searchpath.path) > 0)
			{
				ei_array_push_back(&config->searchpaths, &searchpath);
			}
		}
	}

	fclose(fp);

	/* echo the config */
	ei_info("nthreads\t\t%d\n", config->nthreads);
	ei_info("memlimit\t\t%d\n", config->memlimit);
	ei_info("distributed\t%s\n", config->distributed ? "on" : "off");

	for (i = 0; i < ei_array_size(&config->servers); ++i)
	{
		eiHostDesc	*host_desc;

		host_desc = (eiHostDesc *)ei_array_get(&config->servers, i);

		ei_info("server\t\t%s:%d\n", host_desc->host_name, (eiInt)host_desc->port_number);
	}

	for (i = 0; i < ei_array_size(&config->searchpaths); ++i)
	{
		eiSearchPath	*searchpath;

		searchpath = (eiSearchPath *)ei_array_get(&config->searchpaths, i);

		ei_info("searchpath\t\t%s\n", searchpath->path);
	}

	return eiTRUE;
}
