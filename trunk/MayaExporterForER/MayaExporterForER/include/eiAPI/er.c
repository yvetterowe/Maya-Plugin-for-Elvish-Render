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

#include <eiAPI/ei.h>

int main(int argc, char *argv[])
{
	printf("*****************************************************************\n");
	printf("*                                                               *\n");
	printf("*                         elvish render                         *\n");
	printf("*                                                               *\n");
	printf("*                        Version : %s                      *\n", EI_VERSION);
#ifdef EI_ARCH_X86
	printf("*                        Build   : 32-bit                       *\n");
#else
	printf("*                        Build   : 64-bit                       *\n");
#endif
	printf("*                                                               *\n");
	printf("*  Copyright (c) 2011 elvish render Team. All rights reserved.  *\n");
	printf("*                                                               *\n");
	printf("*****************************************************************\n");
	printf("\n");

	-- argc, ++ argv;
	if (argc == 1)
	{
		ei_parse(argv[0]);
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
