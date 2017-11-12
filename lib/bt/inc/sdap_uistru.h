/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
	sdap_uistru.h
Abstract:
	Export UI data structures.
-------------------------------------------------------------------------------------------------*/
#ifndef _BT_SDAP_UISTRU_H
#define _BT_SDAP_UISTRU_H

/*++++++++++++++++++++ SDAP UI Structure Definition +++++++++++++++++++*/
struct SDAP_UUIDStru
{
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    UCHAR Data4[8];
};

struct SDAP_UInt64Stru
{
	DWORD	higher_4bytes;
	DWORD	lower_4bytes;
};

struct SDAP_DataEleStru {
	DWORD	size;
	UCHAR	descriptor;
	UCHAR	data[1];
};

struct SDAP_SvcAttrStru {
	WORD	attr_id;
	WORD	align_byte;
	struct SDAP_DataEleStru	attr_val;
};

struct SDAP_BrowseInfoStru {
  WORD				trans_hdl;				/* transaction handle representing the browsing transaction */
  WORD				mask;					/* flag used to identify the type of group_id, */
  struct SDAP_UUIDStru	group_id;			/* any UUID value used to identify the browse group 
   											   that is, the size of the UUID value*/
  WORD				duration;				/* duration of the ServiceSearch transaction */
};

struct SDAP_SvcAttrIDStru {
  UCHAR				mask;
  UCHAR				reserved;
  WORD				id;
  WORD				end_id;
};

struct SDAP_SearchInfoStru {
  WORD				trans_hdl;				
  WORD				mask;
  struct SDAP_UUIDStru	pattern;
  WORD				duration;
  WORD				id_count;
  struct SDAP_SvcAttrIDStru	attr_id_list[1];
};

/*++++++++++++++ Service Information Structure +++++++++++++++*/
struct SDAP_GeneralInfoStru {
	WORD           size;				/* sizeof SDAP_GeneralInfoStru, include additional bytes allocated dynamically */
	WORD			reserved1;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			reserved2[1];		/* reserved for alignment */
};

struct SDAP_CTPInfoStru {
	WORD 			size;			    /* sizeof SDAP_CTPInfoStru */
	WORD 			mask;				/* optional attribute mask */
	DWORD 			svc_hdl;			/* service handle */
	UCHAR			ext_network;		/* extern network */
};

struct SDAP_ICPInfoStru {
	WORD 			size;			    /* sizeof SDAP_ICPInfoStru */
	WORD 			reserved;			/* reserved for alignment */
	DWORD 			svc_hdl;			/* service handle */
};

struct SDAP_SPPInfoStru {
	WORD 			size;			    /* sizeof SDAP_SPPInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
};

#ifdef CONFIG_IAP2
struct SDAP_IAPInfoStru {
	WORD 			size;			    /* sizeof SDAP_IAPInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
};
#endif

struct SDAP_HEPHSInfoStru {
	WORD 			size;			    /* sizeof SDAP_HEPHSInfoStru */
	WORD			mask;				/* optional attribute mask */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
};

struct SDAP_HEPAGInfoStru {
	WORD 			size;			    /* sizeof SDAP_HEPAGInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
};

struct SDAP_HFPHFInfoStru {
	WORD			size;			    /* sizeof SDAP_HFPHFInfoStru */
	WORD			mask;				/* optional attribute mask */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
	UCHAR			reserved;			/* reserved for alignment */
	WORD			features;			/* supported features */
	WORD            profile_ver;        /* HFP HF Profile version */
};

struct SDAP_HFPAGInfoStru {
	WORD			size;			    /* sizeof SDAP_HFPAGInfoStru */
	WORD			mask;				/* optional attribute mask */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
	UCHAR			ag_network;			/* network attribute */
	WORD			features;			/* supported features */
	WORD            profile_ver;        /* HFP AG Profile version */
};

struct SDAP_DUNInfoStru {
	WORD			size;			    /* sizeof SDAP_DUNInfoStru */
	WORD			mask;				/* optional attribute mask */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
};

struct SDAP_FaxInfoStru {
	WORD 			size;			    /* sizeof SDAP_FaxInfoStru */
	WORD			mask;				/* optional attribute mask */
	DWORD			svc_hdl;			/* service handle */
	UCHAR 			svr_chnl;			/* service channel */
};

struct SDAP_LAPInfoStru {
	WORD			size;			    /* sizeof SDAP_LAPInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* service channel */
};

struct SDAP_OPPInfoStru {
	WORD 			size;			    /* sizeof SDAP_OPPInfoStru, include additional bytes for fmt_list */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	WORD            psm;                /* PSM of OBEX over L2CAP */
	UCHAR			svr_chnl;			/* service channel */
	UCHAR			fmt_num;			/* number of supported formats */
	UCHAR			fmt_list[1];		/* list of supported formats */
};

struct SDAP_FTPInfoStru {
	WORD			size;			    /* sizeof SDAP_FTPInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	WORD            psm;                /* PSM of OBEX over L2CAP */
	UCHAR			svr_chnl;			/* server channel */
};

struct SDAP_SyncInfoStru {
	WORD			size;			    /* sizeof SDAP_SyncInfoStru, include additional bytes for stores_list */
	WORD 			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* server channel */
	UCHAR			stores_num;			/* number of supported data stores */
	UCHAR 			stores_list[1];		/* list of supported data stores */
};

struct SDAP_SyncCmdInfoStru {
	WORD			size;			    /* sizeof SDAP_SyncCmdInfoStru. */
	WORD 			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* server channel */
};

struct SDAP_PANInfoStru {
	WORD			size;				/* sizeof SDAP_NAPInfoStru, include additional bytes for type_list */
	WORD 			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	WORD			svc_type;			/* type of PAN service in UUID16, it can be GN, NAP or PANU */
	WORD			psm;				/* PSM value in ProtocolDescriptorList attribute */
	WORD 			secu_desc;			/* security description */
	WORD 			net_access_type;	/* net access type, only valid for NAP service */
	DWORD			max_access_rate;	/* maximum possible network access data rate, only valid for NAP service */
	WORD			type_num;			/* number of supported network packet type */
	WORD			type_list[1];		/* list of supported network packet type */
};

struct SDAP_AVRCPInfoStru {
	WORD			size;				/* size of SDAP_AVCTInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	WORD			profile_ver;		/* ProfileVersion attribute value*/
	WORD			svc_type;			/* type of AVRCP service, it can be CT or TG */
	WORD			sup_cg;				/* Supported Categories */
};

struct SDAP_A2DPInfoStru {
	WORD			size;				/* size of SDAP_A2DPInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	WORD			profile_ver;		/* ProfileVersion attribute value*/
	WORD			svc_type;			/* type of A2DP service, it can be SRC or SNK */
	WORD			features;			/* supported features */
};

struct SDAP_HCRPInfoStru {
	WORD			size;				/* sizeof SDAP_HCRPInfoStru */
	WORD			mask;				/* optional attribute mask */
	DWORD			svc_hdl;			/* service handle */
	WORD 			svc_type;			/* the UUID16 value of the most specific class in ServiceClassIDList Attribute */
	WORD 			conn_type;			/* connection type, it can be Control Channel or Notification Channel. */
	WORD			psm_main;			/* psm value for ProtocolDescriptorList attribute */
	WORD			psm_data;			/* psm value for AdditionalProtocolDescriptorList attribute, only valid for 
										   when .*/
	DWORD			svc_id;				/* ServiceID attribute, only valid when conn_type is set to Control Channel. */
	WORD 			size_of_1284id;		/* size of the 1284id attribute, only valid when conn_type is set to Control Channel. */
	UCHAR			str_1284id[1];		/* 1284id attribute value, only valid when conn_type is set to Control Channel. */
};

struct SDAP_HIDInfoStru {
	WORD 			size;				/* size of SDAP_HIDInfoStru, include additional bytes for stores_list */
	WORD 			mask;				/* optional or mandatory Bool type attribute mask */
	DWORD 			svc_hdl;			/* service handle */
	WORD			release_num;		/* HID device release number */
	WORD			parser_ver;			/* HID parser version */
	UCHAR			sub_cls;			/* HID device subclass */
	UCHAR			country_code;		/* HID country code */
	WORD			super_to;			/* HID supervision timeout */
	WORD			profile_ver;		/* HID ProfileVersion attribute value*/
        WORD                    max_latency;            /* HID HOST Max Latency */
        WORD                    min_timeout;            /* HID HOST Min Timeout */
	WORD			desc_list_size;		/* total size of the descriptor list. It also marks the start point 
										   of the report list in the successive memory. */
	UCHAR			list[1];			/* list of HID class descriptor. */
};

struct SDAP_DIInfoStru {
	WORD			size;				/* size of SDAP_DIInfoStru, include additional bytes for str_url_list */
	WORD			mask;				/* optional or mandatory Bool type attribute mask */
	DWORD			svc_hdl;			/* service handle */
	WORD			spec_id;			/* value of SpecificationID attribute */
	WORD			vendor_id;			/* value of VendorID attriubte */
	WORD			product_id;			/* value of ProductID attribute */
	WORD			version;			/* value of Version attribute */
	WORD			vendor_id_src;		/* value of VendorIDSource attribute */
	WORD			list_size;			/* size of the text string list */
	UCHAR			str_url_list[1];	/* List of ClientExecutableURL, DocumentationURL and 
										  ServiceDescription attributes. */
};

struct SDAP_BIPImgInfoStru {
	WORD 			size;				/* size of SDAP_BIPImgInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* server channel */
	UCHAR			capabilities;		/* supported capabilities */
	WORD            psm;                /* PSM of OBEX over L2CAP */
	WORD			features;			/* supported features */
	DWORD			functions;			/* supported functions */
	struct SDAP_UInt64Stru total_data_capacity; /* value of the TotalImagingDataCapacity attribute */
};

struct SDAP_BIPObjInfoStru {
	WORD			size;				/* size of SDAP_BIPObjInfoStru */
	WORD			mask;				/* attribute mask. It presents size descriptor of svc_id currently. */
	DWORD			svc_hdl;			/* service handle */
	WORD			obj_type;			/* a UUID16 representing the type of the object, it can be a referenced object 
										  or an automatic archive. */
	WORD            psm;                /* PSM of OBEX over L2CAP */
	UCHAR			svr_chnl;			/* server channel */
	struct SDAP_UUIDStru svc_id;		/* The value of ServiceId attribute */
	DWORD			functions;			/* supported functions */
};

struct SDAP_SAPInfoStru {
	WORD			size;				/* size of SDAP_SAPInfoStru */
	WORD			mask;				/* attribute mask */
	DWORD			svc_hdl;			/* service handle */
	UCHAR			svr_chnl;			/* server channel */
};

struct SDAP_VDPInfoStru {
	WORD			size;				/* size of SDAP_A2DPInfoStru */
	WORD			reserved;			/* reserved for alignment */
	DWORD			svc_hdl;			/* service handle */
	WORD			svc_type;			/* type of VDP service, it can be SRC or SNK */
};

struct SDAP_BPPPrinterInfoStru {
	WORD 				  size;				/* size of SDAP_BPPRUIInfoStru, include additional bytes for fmt_url_list */
	WORD 				  mask;				/* attribute mask */
	DWORD				  svc_hdl;			/* service handle */
	struct SDAP_UUIDStru  svc_id;			/* The value of ServiceId attribute. */
	struct SDAP_UUIDStru char_repertoires;  /* The value of CharacterRepertoiresSupported attribute */
	WORD				  profile_ver;		/* ProfileVersion attribute value*/
	UCHAR				  job_chnl;			/* Job channel */
	UCHAR				  status_chnl;		/* Status channel */
	WORD				  max_media_width;	/* MaxMediaWidth attribute value */
	WORD				  max_media_len;	/* MaxMediaLength attribute value */
	WORD				  docfmts_size;		/* size of the DocumentFormatsSupported string in str_list */
	WORD				  imgfmts_size;		/* size of the XHTML-PrintImageFormatsSupported string in str_list */
	WORD				  pt1284id_size;	/* size of the 1284ID string in str_list */
	WORD				  ptname_size;		/* size of the PrinterName string in str_list */
	WORD				  ptlocation_size;  /* size of the PrinterLocation string in str_list */
	WORD				  mediatypes_size;  /* size of the MediaTypesSupported string in str_list */
	WORD				  ruifmts_size;	    /* size of the RUIFormatsSupported string in str_list */
	WORD				  pbr_topurl_size;	/* size of the ReferencePrintingTopURL string in str_list */
	WORD				  dps_topurl_size;  /* size of the DirectPrintingTopURL string in str_list */
	WORD				  devname_size;	    /* size of the DeviceName string in str_list */
	UCHAR				  str_list[1]; 		/* List of DocumentFormatsSupported, XHTML-PrintImageFormatsSupported,
											   1284ID, PrinterName, PrinterLocation, MediaTypesSupported,
											   RUIFormatsSupported, ReferencePrintingTopURL, DirectPrintingTopURL
											   and DeviceName attributes. */
};

struct SDAP_BPPObjInfoStru {
	WORD 				  size;				/* size of SDAP_BPPRUIInfoStru, include additional bytes for fmt_url_list */
	WORD 				  mask;				/* attribute mask */
	DWORD				  svc_hdl;			/* service handle */
	struct SDAP_UUIDStru  svc_id;			/* The value of ServiceId attribute. */
	WORD				  profile_ver;		/* ProfileVersion attribute value*/
	UCHAR				  obj_chnl;			/* Object server channel */
	UCHAR				  job_chnl;			/* RUI referenced job channel */
};

struct SDAP_BPPRUIInfoStru {
	WORD 				  size;				/* size of SDAP_BPPRUIInfoStru, include additional bytes for fmt_url_list */
	WORD 				  mask;				/* attribute mask */
	DWORD				  svc_hdl;			/* service handle */
	struct SDAP_UUIDStru  svc_id;			/* The value of ServiceId attribute. */
	WORD				  ruifmts_size;		/* size of the RUIFormatsSupported string in str_list */
	WORD				  rui_topurl_size;	/* size of the PrinterAdminRUITopURL string in str_list */
	UCHAR				  svr_chnl;			/* RUI server channel */
	UCHAR				  str_list[1]; 		/* List of RUIFormatsSupported and PrinterAdminRUITopURL attributes. */
};

struct SDAP_PBAPPCEInfoStru {
	WORD 			size;				/* size of SDAP_PBAPPCEInfoStru */
	WORD			mask;			    /* attribute mask */
	DWORD			svc_hdl;			/* service handle */
	WORD            profile_ver;		/* Profile Version value*/
};

struct SDAP_PBAPPSEInfoStru {
	WORD 			size;				/* size of SDAP_PBAPPSEInfoStru */
	WORD			mask;			    /* attribute mask */
	DWORD			svc_hdl;			/* service handle */
	WORD            profile_ver;		/* Profile Version value*/
	UCHAR			svr_chnl;			/* PSE server channel */
	UCHAR			sup_repst;	        /* SupportedRepositories attribute value */
};

struct SDAP_HDPInfoStru {
	WORD 			size;				/* size of SDAP_HDPInfoStru, include additional bytes for mdep_entry_list */
	WORD			mask;			    /* attribute mask */
	DWORD			svc_hdl;			/* service handle */
	WORD            protocol_ver;       /* MCAP Protocol Version value */
	WORD            profile_ver;		/* HDP Profile Version value*/
	WORD            fst_svc_type;       /* First UUID in the ServiceClassIDList */
	WORD            snd_svc_type;       /* Second UUID in the ServiceClassIDList if presented. Set it to 0 if not required. */
	WORD            psm_ctrl;           /* Control channel PSM */
	WORD            psm_data;           /* Data channel PSM */
	UCHAR           data_exchg_spec;    /* Data Exchange Specification */
	UCHAR           mcap_sup_proc;      /* MCAP Supported Procedure */
	WORD            mdep_entry_num;     /* Number of MDEP entries supported */
	UCHAR           mdep_entry_list[1]; /* List of MDEP entries. Each entry shall specifies its MDEP_ID, MDEP_Data_Type,
	                                       MDEP_Role and MDEP_Description values in the following order:
	||-MDEP_ID (1Byte)-||-MDEP_Data_Type (2Bytes, Little Endian)-||-MDEP_Role (1Byte)-||
	||-MDEP_Description_Length (2Bytes, Little Endian)-||-MDEP_Descrition (Variable Length)-||.*/
};

struct SDAP_HDPMdepListStru {
    UCHAR           id;            /* MDEP ID value */
    UCHAR           role;          /* MDEP Role value */
    WORD            data_type;     /* MDEP Data Type value */
    UCHAR           *description;  /* MDEP Description, null-terminated UTF-8 string */
};

struct SDAP_HDPInfoHlpStru {
	DWORD			svc_hdl;			/* service handle */
	WORD            protocol_ver;       /* MCAP Protocol Version value */
	WORD            profile_ver;		/* Profile Version value*/
	WORD            fst_svc_type;       /* First UUID in the ServiceClassIDList */
	WORD            snd_svc_type;       /* Second UUID in the ServiceClassIDList if presented. Set it to 0 if not required. */
	WORD            psm_ctrl;           /* Control channel PSM */
	WORD            psm_data;           /* Data channel PSM */
	UCHAR           data_exchg_spec;    /* Data Exchange Specification */
	UCHAR           mcap_sup_proc;      /* MCAP Supported Procedure */
	WORD            mdep_entry_num;     /* Number of MDEP entries supported */
	struct SDAP_HDPMdepListStru *mdep_entry_list; /* List of MDEP entries */
};

struct SDAP_MAPMASInfoStru {
	WORD	        size;               /* size of SDAP_MAPMASInfoStru */
	WORD	        mask;               /* attribute mask */
	DWORD	        svc_hdl;			/* service handle */
	WORD	        profile_ver;		/* Profile Version value*/
	UCHAR	        svr_chnl;			/* MSE server channel */
	UCHAR	        mas_inst_id;        /* MAS Instance ID */
	UCHAR	        sup_msg_types;      /* Supported Message Types */ 
};

struct SDAP_MAPMNSInfoStru {
	WORD	        size;               /* size of SDAP_MAPMASInfoStru */
	WORD	        mask;               /* attribute mask */
	DWORD	        svc_hdl;			/* service handle */
	WORD	        profile_ver;		/* Profile Version value*/
	UCHAR	        svr_chnl;			/* MSE server channel */
};

struct SDAP_GATTInfoStru {
    WORD            size;               /* size of SDAP_GATTInfoStru */
    WORD            mask;               /* attribute mask. For alignment only, set to 0. */
    DWORD           svc_hdl;			/* service handle */
    WORD            start_handle;		/* GATT Start Handle */
    WORD            end_handle;		    /* GATT End Handle */
};

struct SDAP_MPSInfoStru {
   DWORD                      svc_hdl;          /* service handle */
   WORD                       mask;             /* mask, support MSMD set 1, */
   WORD                       ver;              /* mps version */
   struct SDAP_UInt64Stru     mpsd_scenarios;   /* mpsd secnarios */
   struct SDAP_UInt64Stru     mpmd_scenarios;   /* mpmd secnarios */
   WORD                       depend;           /* profile and protocol dependencies */
};

#define SDAP_HEPInfoStru			SDAP_HEPHSInfoStru
#define SDAP_HFPInfoStru			SDAP_HFPHFInfoStru

#define SDAP_GNInfoStru				SDAP_PANInfoStru
#define SDAP_NAPInfoStru			SDAP_PANInfoStru
#define SDAP_PANUInfoStru			SDAP_PANInfoStru

#define SDAP_AVCTInfoStru			SDAP_AVRCPInfoStru
#define SDAP_AVTGInfoStru			SDAP_AVRCPInfoStru

#define SDAP_A2DPSrcInfoStru		SDAP_A2DPInfoStru
#define SDAP_A2DPSnkInfoStru		SDAP_A2DPInfoStru

#define SDAP_HCRPCtrlInfoStru		SDAP_HCRPInfoStru
#define SDAP_HCRPNotifyInfoStru		SDAP_HCRPInfoStru

#define SDAP_BIPRefObjInfoStru		SDAP_BIPObjInfoStru
#define SDAP_BIPArchInfoStru		SDAP_BIPObjInfoStru

#define SDAP_HDPSrcInfoStru         SDAP_HDPInfoStru
#define SDAP_HDPSnkInfoStru         SDAP_HDPInfoStru

#endif
