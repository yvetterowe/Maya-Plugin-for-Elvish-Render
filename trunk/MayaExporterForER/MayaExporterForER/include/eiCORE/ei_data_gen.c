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

#include <eiCORE/ei_data_gen.h>
#include <eiCORE/ei_data_array.h>
#include <eiCORE/ei_data_table.h>
#include <eiCORE/ei_assert.h>
#include <eiCORE/ei_vector.h>
#include <eiCORE/ei_vector2.h>
#include <eiCORE/ei_vector4.h>
#include <eiCORE/ei_matrix.h>
#include <eiCORE/ei_bound.h>
#include <eiCORE/ei_rect.h>

#define eiTEST_TILE_SIZE		4

static void byteswap_short(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_short((eiShort *)data);
}

static void byteswap_int(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_int((eiInt *)data);
}

static void byteswap_long(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_long((eiLong *)data);
}

static void byteswap_scalar(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_scalar((eiScalar *)data);
}

static void byteswap_geoscalar(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_geoscalar((eiGeoScalar *)data);
}

static void byteswap_vector(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_vector((eiVector *)data);
}

static void byteswap_vector2(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_vector2((eiVector2 *)data);
}

static void byteswap_vector4(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_vector4((eiVector4 *)data);
}

static void byteswap_matrix(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_matrix((eiMatrix *)data);
}

static void byteswap_bound(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_bound((eiBound *)data);
}

static void byteswap_rect(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_rect((eiRect *)data);
}

static void byteswap_rect4i(eiDatabase *db, void *data, const eiUint size)
{
	ei_byteswap_rect4i((eiRect4i *)data);
}

static void byteswap_intarray(eiDatabase *db, void *data, const eiUint size)
{
	eiUint	i;
	eiUint	num_items;
	eiInt	*items;
	
	num_items = size / sizeof(eiInt);
	items = (eiInt *)data;

	for (i = 0; i < num_items; ++i)
	{
		ei_byteswap_int(items + i);
	}
}

static eiBool cast_byte(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	return eiFALSE;
}

static eiBool cast_short(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	return eiFALSE;
}

static eiBool cast_int(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	switch (src_type)
	{
	case EI_DATA_TYPE_INT:
		*((eiInt *)dst) = *((eiInt *)src);
		return eiTRUE;
	case EI_DATA_TYPE_BOOL:
		*((eiInt *)dst) = (eiInt)(*((eiBool *)src));
		return eiTRUE;
	case EI_DATA_TYPE_SCALAR:
		*((eiInt *)dst) = (eiInt)(*((eiScalar *)src));
		return eiTRUE;
	default:
		return eiFALSE;
	}
	return eiFALSE;
}

static eiBool cast_bool(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	switch (src_type)
	{
	case EI_DATA_TYPE_INT:
		*((eiBool *)dst) = (*((eiInt *)src) != 0) ? eiTRUE : eiFALSE;
		return eiTRUE;
	case EI_DATA_TYPE_BOOL:
		*((eiBool *)dst) = *((eiBool *)src);
		return eiTRUE;
	case EI_DATA_TYPE_SCALAR:
		*((eiBool *)dst) = (*((eiScalar *)src) != 0.0f) ? eiTRUE : eiFALSE;
		return eiTRUE;
	default:
		return eiFALSE;
	}
	return eiFALSE;
}

static eiBool cast_long(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	return eiFALSE;
}

static eiBool cast_scalar(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	switch (src_type)
	{
	case EI_DATA_TYPE_INT:
		*((eiScalar *)dst) = (eiScalar)(*((eiInt *)src));
		return eiTRUE;
	case EI_DATA_TYPE_BOOL:
		*((eiScalar *)dst) = (eiScalar)(*((eiBool *)src));
		return eiTRUE;
	case EI_DATA_TYPE_SCALAR:
		*((eiScalar *)dst) = *((eiScalar *)src);
		return eiTRUE;
	default:
		return eiFALSE;
	}
	return eiFALSE;
}

static eiBool cast_geoscalar(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	return eiFALSE;
}

static eiBool cast_vector(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	switch (src_type)
	{
	case EI_DATA_TYPE_INT:
		setvf((eiVector *)dst, (eiScalar)(*((eiInt *)src)));
		return eiTRUE;
	case EI_DATA_TYPE_BOOL:
		setvf((eiVector *)dst, (*((eiInt *)src)) ? 1.0f : 0.0f);
		return eiTRUE;
	case EI_DATA_TYPE_SCALAR:
		setvf((eiVector *)dst, *((eiScalar *)src));
		return eiTRUE;
	case EI_DATA_TYPE_VECTOR:
		*((eiVector *)dst) = *((eiVector *)src);
		return eiTRUE;
	default:
		return eiFALSE;
	}
	return eiFALSE;
}

static eiBool cast_vector2(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	return eiFALSE;
}

static eiBool cast_vector4(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	return eiFALSE;
}

static eiBool cast_matrix(eiDatabase *db, eiByte *dst, const eiByte *src, const eiInt src_type)
{
	switch (src_type)
	{
	case EI_DATA_TYPE_INT:
		initm((eiMatrix *)dst, (eiScalar)(*((eiInt *)src)));
		return eiTRUE;
	case EI_DATA_TYPE_BOOL:
		initm((eiMatrix *)dst, (*((eiInt *)src)) ? 1.0f : 0.0f);
		return eiTRUE;
	case EI_DATA_TYPE_SCALAR:
		initm((eiMatrix *)dst, *((eiScalar *)src));
		return eiTRUE;
	case EI_DATA_TYPE_MATRIX:
		*((eiMatrix *)dst) = *((eiMatrix *)src);
		return eiTRUE;
	default:
		return eiFALSE;
	}
	return eiFALSE;
}

static void byteswap_job_test(eiDatabase *db, void *job, const eiUint size)
{
	eiTestJob *pJob = (eiTestJob *)job;

	ei_byteswap_short(&pJob->checkShort);
	ei_byteswap_short(&pJob->checkUshort);
	ei_byteswap_int(&pJob->checkInt);
	ei_byteswap_int(&pJob->checkUint);
	ei_byteswap_long(&pJob->checkLong);
	ei_byteswap_long(&pJob->checkUlong);
	ei_byteswap_scalar(&pJob->checkFloat);
	ei_byteswap_geoscalar(&pJob->checkDouble);
}

static eiBool execute_job_test(eiDatabase *db, eiBaseWorker *pWorker, void *job, void *param)
{
	eiUint i, j;
	eiTestJob *pJob;
	
	pJob = (eiTestJob *)job;

	/* do data integrity checks... */
	if (pJob->checkShort != eiTEST_CHECK_SHORT)
	{
		return eiFALSE;
	}

	if (pJob->checkUshort != eiTEST_CHECK_USHORT)
	{
		return eiFALSE;
	}

	if (pJob->checkInt != eiTEST_CHECK_INT)
	{
		return eiFALSE;
	}

	if (pJob->checkUint != eiTEST_CHECK_UINT)
	{
		return eiFALSE;
	}

	if (pJob->checkLong != eiTEST_CHECK_LONG)
	{
		return eiFALSE;
	}

	if (pJob->checkUlong != eiTEST_CHECK_ULONG)
	{
		return eiFALSE;
	}

	if (pJob->checkFloat != eiTEST_CHECK_FLOAT)
	{
		return eiFALSE;
	}

	if (pJob->checkDouble != eiTEST_CHECK_DOUBLE)
	{
		return eiFALSE;
	}

	/* simulate a small tile rendering... */
	for (j = 0; j < eiTEST_TILE_SIZE; ++j)
	{
		if (!ei_base_worker_is_running(pWorker))
		{
			return eiTRUE;
		}

		for (i = 0; i < eiTEST_TILE_SIZE; ++i)
		{ 
			ei_sleep(1);
		}

		ei_base_worker_step_progress(pWorker, eiTEST_TILE_SIZE);
	}

	return eiTRUE;
}

static eiUint count_job_test(eiDatabase *db, void *job)
{
	eiTestJob *pJob = (eiTestJob *)job;

	return (eiTEST_TILE_SIZE * eiTEST_TILE_SIZE);
}

eiDataGenTable g_DataGenTable = {
	NULL, 
	0, 
};

eiInitGlobals g_InitGlobals = NULL;
eiExitGlobals g_ExitGlobals = NULL;

eiInitTLS g_InitTLS = NULL;
eiExitTLS g_ExitTLS = NULL;

eiSetSceneCallback g_SetSceneCallback = NULL;
eiEndSceneCallback g_EndSceneCallback = NULL;
eiUpdateSceneCallback g_UpdateSceneCallback = NULL;
eiLinkCallback g_LinkCallback = NULL;

void ei_init_default_data_gen_table()
{
	/* register atomic types */
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NONE ].byteswap = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NONE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NONE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NONE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NONE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NONE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_NONE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_BYTE ].byteswap = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BYTE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BYTE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BYTE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BYTE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BYTE ].cast = cast_byte;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BYTE ].type_size = sizeof(eiByte);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHORT ].byteswap = byteswap_short;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHORT ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHORT ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHORT ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHORT ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHORT ].cast = cast_short;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SHORT ].type_size = sizeof(eiShort);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_INT ].byteswap = byteswap_int;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INT ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INT ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INT ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INT ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INT ].cast = cast_int;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INT ].type_size = sizeof(eiInt);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOOL ].byteswap = byteswap_int;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOOL ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOOL ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOOL ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOOL ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOOL ].cast = cast_bool;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOOL ].type_size = sizeof(eiBool);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_TAG ].byteswap = byteswap_int;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TAG ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TAG ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TAG ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TAG ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TAG ].cast = cast_int;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TAG ].type_size = sizeof(eiTag);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_INDEX ].byteswap = byteswap_int;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INDEX ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INDEX ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INDEX ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INDEX ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INDEX ].cast = cast_int;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INDEX ].type_size = sizeof(eiIndex);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_LONG ].byteswap = byteswap_long;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LONG ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LONG ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LONG ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LONG ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LONG ].cast = cast_long;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_LONG ].type_size = sizeof(eiLong);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_SCALAR ].byteswap = byteswap_scalar;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SCALAR ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SCALAR ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SCALAR ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SCALAR ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SCALAR ].cast = cast_scalar;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_SCALAR ].type_size = sizeof(eiScalar);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_GEOSCALAR ].byteswap = byteswap_geoscalar;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_GEOSCALAR ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_GEOSCALAR ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_GEOSCALAR ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_GEOSCALAR ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_GEOSCALAR ].cast = cast_geoscalar;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_GEOSCALAR ].type_size = sizeof(eiGeoScalar);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR ].byteswap = byteswap_vector;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR ].cast = cast_vector;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR ].type_size = sizeof(eiVector);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR2 ].byteswap = byteswap_vector2;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR2 ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR2 ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR2 ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR2 ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR2 ].cast = cast_vector2;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR2 ].type_size = sizeof(eiVector2);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR4 ].byteswap = byteswap_vector4;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR4 ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR4 ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR4 ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR4 ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR4 ].cast = cast_vector4;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_VECTOR4 ].type_size = sizeof(eiVector4);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_MATRIX ].byteswap = byteswap_matrix;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MATRIX ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MATRIX ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MATRIX ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MATRIX ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MATRIX ].cast = cast_matrix;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_MATRIX ].type_size = sizeof(eiMatrix);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOUND ].byteswap = byteswap_bound;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOUND ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOUND ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOUND ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOUND ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOUND ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BOUND ].type_size = sizeof(eiBound);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT ].byteswap = byteswap_rect;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT ].type_size = sizeof(eiRect);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT4I ].byteswap = byteswap_rect4i;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT4I ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT4I ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT4I ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT4I ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT4I ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_RECT4I ].type_size = sizeof(eiRect4i);

	g_DataGenTable.data_gens[ EI_DATA_TYPE_INTARRAY ].byteswap = byteswap_intarray;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INTARRAY ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INTARRAY ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INTARRAY ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INTARRAY ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INTARRAY ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_INTARRAY ].type_size = 0;

	/* register compound types */
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TEST ].byteswap = byteswap_job_test;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TEST ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TEST ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TEST ].execute_job = execute_job_test;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TEST ].count_job = count_job_test;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TEST ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_JOB_TEST ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_ARRAY ].byteswap = byteswap_data_array;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_ARRAY ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_ARRAY ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_ARRAY ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_ARRAY ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_ARRAY ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_ARRAY ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].byteswap = byteswap_data_table;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].type_size = 0;

	g_DataGenTable.data_gens[ EI_DATA_TYPE_BLOCK ].byteswap = byteswap_data_block;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BLOCK ].generate_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BLOCK ].clear_data = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BLOCK ].execute_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BLOCK ].count_job = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_BLOCK ].cast = NULL;
	g_DataGenTable.data_gens[ EI_DATA_TYPE_TABLE ].type_size = 0;
}

void ei_run_server()
{
	ei_job_run_server(
		&g_DataGenTable, 
		g_InitGlobals, g_ExitGlobals, 
		g_InitTLS, g_ExitTLS, 
		g_SetSceneCallback, g_EndSceneCallback, g_UpdateSceneCallback, 
		g_LinkCallback);
}
