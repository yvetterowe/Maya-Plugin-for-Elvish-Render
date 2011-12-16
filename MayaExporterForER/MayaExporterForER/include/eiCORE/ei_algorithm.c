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

#include <eiCORE/ei_algorithm.h>
#include <string.h>

static eiFORCEINLINE void adjust_heap(void *a, const eiIntptr i, const eiIntptr n, 
						const eiIntptr item_size, ei_compare compare, 
						eiByte *item)
{
	eiIntptr j = 2 * i + 1;
	eiByte *base = (eiByte *)a;
	memcpy(item, base + item_size * i, item_size); /* item = a[i] */

	while (j < n)
	{
		if (j < (n - 1) && compare(base + item_size * j, base + item_size * (j + 1)) < 0) /* a[j] < a[j + 1] */
		{
			++j;
		}

		if (compare(item, base + item_size * j) >= 0) /* item >= a[j] */
		{
			break;
		}

		memcpy(base + item_size * ((j - 1) / 2), base + item_size * j, item_size); /* a[(j - 1) / 2] = a[j] */
		j = 2 * j + 1;
	}

	memcpy(base + item_size * ((j - 1) / 2), item, item_size); /* a[(j - 1) / 2] = item */
}

static eiFORCEINLINE void make_heap(void *a, const eiIntptr n, 
					  const eiIntptr item_size, ei_compare compare, 
					  eiByte *item)
{
	eiIntptr i;

	for (i = ((n - 2) / 2); i >= 0; --i)
	{
		adjust_heap(a, i, n, item_size, compare, item);
	}
}

void ei_adjust_heap(void *a, const eiIntptr i, const eiIntptr n, const eiIntptr item_size, ei_compare compare)
{
	eiByte *item = (eiByte *)_alloca(item_size);

	adjust_heap(a, i, n, item_size, compare, item);
}

void ei_make_heap(void *a, const eiIntptr n, const eiIntptr item_size, ei_compare compare)
{
	eiByte *item = (eiByte *)_alloca(item_size);

	make_heap(a, n, item_size, compare, item);
}

void ei_heapsort(void *a, const eiIntptr n, const eiIntptr item_size, ei_compare compare)
{
	eiIntptr i;
	eiByte *base = (eiByte *)a;
	eiByte *temp = (eiByte *)_alloca(item_size);

	if (n <= 1)
	{
		return;
	}

	make_heap(a, n, item_size, compare, temp);

	for (i = n - 1; i > 0; --i)
	{
		memcpy(temp, base + item_size * i, item_size); /* temp = a[i] */
		memcpy(base + item_size * i, base, item_size); /* a[i] = a[0] */
		memcpy(base, temp, item_size); /* a[0] = temp */

		adjust_heap(a, 0, i, item_size, compare, temp);
	}
}

eiIntptr ei_binsearch(void *a, const eiIntptr n, void *x, const eiIntptr item_size, ei_compare compare)
{
	eiIntptr low = 0, high = n, mid = 0;
	eiByte *base = (eiByte *)a;
	eiByte *p = NULL;

	if (n <= 0)
	{
		return -1;
	}

	while (low < (high - 1))
	{
		mid = (low + high) / 2;

		p = base + item_size * mid; /* a[mid] */
		if (compare(x, p) < 0)
		{
			high = mid;
		}
		else
		{
			low = mid;
		}
	}

	p = base + item_size * low; /* a[low] */
	if (compare(x, p) == 0)
	{
		return low;
	}
	else
	{
		return -1;
	}
}
