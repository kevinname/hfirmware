/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    sdpuistru.h
Abstract:
	This files exports SDP API data types.
---------------------------------------------------------------------------*/
#ifndef _WPSDP_UI_STRU_H_
#define _WPSDP_UI_STRU_H_

/* Data type definition */

struct SDP_UUID {
	UCHAR	var[16];
	UCHAR	mask;
};

struct DataElement {
	DWORD	size;
	UCHAR	descriptor;
	UCHAR	data[1];
};	

struct ServiceAttribute {
	WORD	attribute_id;
	WORD	align_byte;
	struct DataElement	attribute_value;
};

struct ServiceAttributeID {
	UCHAR	mask;
	UCHAR	reserved;
	WORD	id;
	WORD	end_id;
};

struct ServiceRegStru {
	DWORD	handle;
	struct ServiceAttribute *attr;
};

struct SDP_UIReqParamStru {
	WORD req_id;
	UCHAR *in_param;
};

struct SDP_UICfmParamStru {
	WORD req_id;
	WORD result;
	UCHAR *out_param;
};

struct SDP_SearchReqStru {
	WORD trans_handle;
	WORD mask;
	/*Stop Rule*/
	WORD	max_count;
	WORD	duration;
	/*Input Parameters*/
	DWORD	handle;
	WORD	pattern_count;
	WORD	id_count;
	struct SDP_UUID	*service_search_pattern;
	struct ServiceAttributeID	*attribute_id_list;
};

struct SDP_SearchCfmStru {
	DWORD	count;
	UCHAR *var;
};

#endif
