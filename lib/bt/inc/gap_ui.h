/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
	gap_ui.h
Abstract:
	The module defines the GAP constants and function declarations exported
	to the application.
---------------------------------------------------------------------------*/
#ifndef GAP_UI_H
#define GAP_UI_H

#include "global.h"
#include "hci_ui.h"
#include "l2cap_ui.h"

#if (defined(CONFIG_GAP) && !defined(CONFIG_GAP_ASYNC_API_SUPPORTED) && !defined(CONFIG_GAP_SYNC_API_SUPPORTED))
#define CONFIG_GAP_ASYNC_API_SUPPORTED      1       /* By defualt, asynchronous API is exported */
#define CONFIG_GAP_SYNC_API_SUPPORTED     1       /* For internal test, synchronous API is exported */
#endif

#define GAP_NEW_STRU(stru, var)	var = (stru *)NEW(sizeof(stru))

/******************************************************************************
 *																			  *
 *					GAP_ExecuteCommand function manual					      *
 *																			  *
 ******************************************************************************
 Table below describes each command index with its corresponding input and
 output paramter.

 Each parameter shall be a pointer of the specified type. A NULL value means
 this parameter is ignored. 

 Volume 2, Part E of Bluetooth Core Specification has detailed definiation for 
 parameters of each command and event.

 ******************************************************************************/

/*****************************************************************************************************************************************************************************
    opindex							                        in_par							                                        out_par
 *****************************************************************************************************************************************************************************/

/*---------------------------------------------------------------------------------*/
/*                              Link Control - 0x0400                              */
/*---------------------------------------------------------------------------------*/
/* HCI_OPS_INQUIRY,                                     struct HCI_InquiryStru,                                         struct HCI_Inquiry_CompleteEvStru */
/* HCI_OPS_INQUIRY_CANCEL,                              -                                                               struct HCI_Inquiry_CancelCompStru */
/* HCI_OPS_PERIODIC_INQUIRY_MODE,                       struct HCI_Periodic_Inquiry_ModeStru,                           struct HCI_Periodic_Inquiry_ModeCompStru */
/* HCI_OPS_EXIT_PERIODIC_INQUIRY_MODE,                  -                                                               struct HCI_Exit_Periodic_Inquiry_ModeCompStru */
/* HCI_OPS_CREATE_CONNECTION,                           struct HCI_Create_ConnectionStru,                               struct HCI_Connection_CompleteEvStru */
/* HCI_OPS_DISCONNECT,                                  struct HCI_DisconnectStru,                                      struct HCI_Disconnection_CompleteEvStru */
/* HCI_OPS_ADD_SCO_CONNECTION,                          struct HCI_Add_SCO_ConnectionStru,                              struct HCI_Connection_CompleteEvStru */
/* HCI_OPS_CREATE_CONNECTION_CANCEL,                    struct HCI_Create_Connection_CancelStru,                        struct HCI_Create_Connection_CancelCompStru */
/* HCI_OPS_ACCEPT_CONNECTION_REQUEST,                   struct HCI_Accept_Connection_RequestStru,                       struct HCI_Connection_CompleteEvStru */
/* HCI_OPS_REJECT_CONNECTION_REQUEST,                   struct HCI_Reject_Connection_RequestStru,                       struct HCI_Connection_CompleteEvStru */
/* HCI_OPS_LINK_KEY_REQUEST_REPLY,                      struct HCI_Link_Key_Request_ReplyStru,                          struct HCI_Link_Key_Request_ReplyCompStru */
/* HCI_OPS_LINK_KEY_REQUEST_NEGATIVE_REPLY,             struct HCI_Link_Key_Request_Negative_ReplyStru,                 struct HCI_Link_Key_Request_Negative_ReplyCompStru */
/* HCI_OPS_PIN_CODE_REQUEST_REPLY,                      struct HCI_PIN_Code_Request_ReplyStru,                          struct HCI_PIN_Code_Request_ReplyCompStru */
/* HCI_OPS_PIN_CODE_REQUEST_NEGATIVE_REPLY,             struct HCI_PIN_Code_Request_Negative_ReplyStru,                 struct HCI_PIN_Code_Request_Negative_ReplyCompStru */
/* HCI_OPS_CHANGE_CONNECTION_PACKET_TYPE,               struct HCI_Change_Connection_Packet_TypeStru,                   struct HCI_Connection_Packet_Type_ChangedEvStru */
/* HCI_OPS_AUTHENTICATION_REQUESTED,                    struct HCI_Authentication_RequestedStru,                        struct HCI_Authentication_CompleteEvStru */
/* HCI_OPS_SET_CONNECTION_ENCRYPTION,                   struct HCI_Set_Connection_EncryptionStru,                       struct HCI_Encryption_ChangeEvStru */
/* HCI_OPS_CHANGE_CONNECTION_LINK_KEY,                  struct HCI_Change_Connection_Link_KeyStru,                      struct HCI_Change_Connection_Link_Key_CompleteEvStru */
/* HCI_OPS_MASTER_LINK_KEY,                             struct HCI_Master_Link_KeyStru,                                 struct HCI_Master_Link_Key_CompleteEvStru */
/* HCI_OPS_REMOTE_NAME_REQUEST,                         struct HCI_Remote_Name_RequestStru,                             struct HCI_Remote_Name_Request_CompleteEvStru */
/* HCI_OPS_REMOTE_NAME_REQUEST_CANCEL,                  struct HCI_Remote_Name_Request_CancelStru,                      struct HCI_Remote_Name_Request_CancelCompStru */
/* HCI_OPS_READ_REMOTE_SUPPORTED_FEATURES,              struct HCI_Read_Remote_Supported_FeaturesStru,                  struct HCI_Read_Remote_Supported_Features_CompleteEvStru */
/* HCI_OPS_READ_REMOTE_EXTENDED_FEATURES,               struct HCI_Read_Remote_Extended_FeaturesStru,                   struct HCI_Read_Remote_Extended_Features_CompleteEvStru */
/* HCI_OPS_READ_REMOTE_VERSION_INFORMATION,             struct HCI_Read_Remote_Version_InformationStru,                 struct HCI_Read_Remote_Version_Information_CompleteEvStru */
/* HCI_OPS_READ_CLOCK_OFFSET,                           struct HCI_Read_Clock_OffsetStru,                               struct HCI_Read_Clock_Offset_CompleteEvStru */
/* HCI_OPS_READ_LMP_HANDLE,                             struct HCI_Read_LMP_HandleStru,                                 struct HCI_Read_LMP_HandleCompStru */
/* HCI_OPS_SETUP_SYNCHRONOUS_CONNECTION,                struct HCI_Setup_Synchronous_ConnectionStru,                    struct HCI_Synchronous_Connection_CompleteEvStru */
/* HCI_OPS_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST,       struct HCI_Accept_Synchronous_Connection_RequestStru,           struct HCI_Synchronous_Connection_CompleteEvStru */
/* HCI_OPS_REJECT_SYNCHRONOUS_CONNECTION_REQUEST,       struct HCI_Reject_Synchronous_Connection_RequestStru,           struct HCI_Synchronous_Connection_CompleteEvStru */
/* HCI_OPS_IO_CAPABILITY_REQUEST_REPLY,                 struct HCI_IO_Capability_Request_ReplyStru,                     struct HCI_IO_Capability_Request_ReplyCompStru */
/* HCI_OPS_USER_CONFIRMATION_REQUEST_REPLY,             struct HCI_User_Confirmation_Request_ReplyStru,                 struct HCI_User_Confirmation_Request_ReplyCompStru */
/* HCI_OPS_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY,    struct HCI_User_Confirmation_Request_Negative_ReplyStru,        struct HCI_User_Confirmation_Request_Negative_ReplyCompStru */
/* HCI_OPS_USER_PASSKEY_REQUEST_REPLY,                  struct HCI_User_Passkey_Request_ReplyStru,                      struct HCI_User_Passkey_Request_ReplyCompStru */
/* HCI_OPS_USER_PASSKEY_REQUEST_NEGATIVE_REPLY,         struct HCI_User_Passkey_Request_Negative_ReplyStru,             struct HCI_User_Passkey_Request_Negative_ReplyCompStru */
/* HCI_OPS_REMOTE_OOB_DATA_REQUEST_REPLY,               struct HCI_Remote_OOB_Data_Request_ReplyStru,                   struct HCI_Remote_OOB_Data_Request_ReplyCompStru */
/* HCI_OPS_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY,      struct HCI_Remote_OOB_Data_Request_Negative_ReplyStru,          struct HCI_Remote_OOB_Data_Request_Negative_ReplyCompStru */
/* HCI_OPS_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY,        struct HCI_IO_Capability_Request_Negative_ReplyStru,            struct HCI_IO_Capability_Request_Negative_ReplyCompStru */

/*---------------------------------------------------------------------------------*/
/*                               Link Policy - 0x0800                              */
/*---------------------------------------------------------------------------------*/
/* HCI_OPS_HOLD_MODE,                                   struct HCI_Hold_ModeStru,                                       struct HCI_Mode_ChangeEvStru */
/* HCI_OPS_SNIFF_MODE,                                  struct HCI_Sniff_ModeStru,                                      struct HCI_Mode_ChangeEvStru */
/* HCI_OPS_EXIT_SNIFF_MODE,                             struct HCI_Exit_Sniff_ModeStru,                                 struct HCI_Mode_ChangeEvStru */
/* HCI_OPS_PARK_STATE,                                  struct HCI_Park_StateStru,                                      struct HCI_Mode_ChangeEvStru */
/* HCI_OPS_EXIT_PARK_STATE,                             struct HCI_Exit_Park_StateStru,                                 struct HCI_Mode_ChangeEvStru */
/* HCI_OPS_QOS_SETUP,                                   struct HCI_QoS_SetupStru,                                       struct HCI_QoS_Setup_CompleteEvStru */
/* HCI_OPS_ROLE_DISCOVERY,                              struct HCI_Role_DiscoveryStru,                                  struct HCI_Role_DiscoveryCompStru */
/* HCI_OPS_SWITCH_ROLE,                                 struct HCI_Switch_RoleStru,                                     struct HCI_Role_ChangeEvStru */
/* HCI_OPS_READ_LINK_POLICY_SETTINGS,                   struct HCI_Read_Link_Policy_SettingsStru,                       struct HCI_Read_Link_Policy_SettingsCompStru */
/* HCI_OPS_WRITE_LINK_POLICY_SETTINGS,                  struct HCI_Write_Link_Policy_SettingsStru,                      struct HCI_Write_Link_Policy_SettingsCompStru */
/* HCI_OPS_READ_DEFAULT_LINK_POLICY_SETTINGS,           -                                                               struct HCI_Read_Default_Link_Policy_SettingsCompStru */
/* HCI_OPS_WRITE_DEFAULT_LINK_POLICY_SETTINGS,          struct HCI_Write_Default_Link_Policy_SettingsStru,              struct HCI_Write_Default_Link_Policy_SettingsCompStru */
/* HCI_OPS_FLOW_SPECIFICATION,                          struct HCI_Flow_SpecificationStru,                              struct HCI_Flow_Specification_CompleteEvStru */
/* HCI_OPS_SNIFF_SUBRATING,                             struct HCI_Sniff_SubratingStru,                                 struct HCI_Sniff_SubratingCompStru */

/*---------------------------------------------------------------------------------*/
/*                          Controller & Baseband - 0x0c00                         */
/*---------------------------------------------------------------------------------*/
/* HCI_OPS_SET_EVENT_MASK,                              struct HCI_Set_Event_MaskStru,                                  struct HCI_Set_Event_MaskCompStru */
/* HCI_OPS_RESET,                                       -                                                               struct HCI_ResetCompStru */
/* HCI_OPS_SET_EVENT_FILTER,                            struct HCI_Set_Event_FilterStru,                                struct HCI_Set_Event_FilterCompStru */
/* HCI_OPS_FLUSH,                                       struct HCI_FlushStru,                                           struct HCI_FlushCompStru */
/* HCI_OPS_READ_PIN_TYPE,                               -                                                               struct HCI_Read_PIN_TypeCompStru */
/* HCI_OPS_WRITE_PIN_TYPE,                              struct HCI_Write_PIN_TypeStru,                                  struct HCI_Write_PIN_TypeCompStru */
/* HCI_OPS_CREATE_NEW_UNIT_KEY,                         -                                                               struct HCI_Create_New_Unit_KeyCompStru */
/* HCI_OPS_READ_STORED_LINK_KEY,                        struct HCI_Read_Stored_Link_KeyStru,                            struct HCI_Read_Stored_Link_KeyCompStru */
/* HCI_OPS_WRITE_STORED_LINK_KEY,                       struct HCI_Write_Stored_Link_KeyStru,                           struct HCI_Write_Stored_Link_KeyCompStru */
/* HCI_OPS_DELETE_STORED_LINK_KEY,                      struct HCI_Delete_Stored_Link_KeyStru,                          struct HCI_Delete_Stored_Link_KeyCompStru */
/* HCI_OPS_WRITE_LOCAL_NAME,                            struct HCI_Write_Local_NameStru,                                struct HCI_Write_Local_NameCompStru */
/* HCI_OPS_READ_LOCAL_NAME,                             -                                                               struct HCI_Read_Local_NameCompStru */
/* HCI_OPS_READ_CONNECTION_ACCEPT_TIMEOUT,              -                                                               struct HCI_Read_Connection_Accept_TimeoutCompStru */
/* HCI_OPS_WRITE_CONNECTION_ACCEPT_TIMEOUT,             struct HCI_Write_Connection_Accept_TimeoutStru,                 struct HCI_Write_Connection_Accept_TimeoutCompStru */
/* HCI_OPS_READ_PAGE_TIMEOUT,                           -                                                               struct HCI_Read_Page_TimeoutCompStru */
/* HCI_OPS_WRITE_PAGE_TIMEOUT,                          struct HCI_Write_Page_TimeoutStru,                              struct HCI_Write_Page_TimeoutCompStru */
/* HCI_OPS_READ_SCAN_ENABLE,                            -                                                               struct HCI_Read_Scan_EnableCompStru */
/* HCI_OPS_WRITE_SCAN_ENABLE,                           struct HCI_Write_Scan_EnableStru,                               struct HCI_Write_Scan_EnableCompStru */
/* HCI_OPS_READ_PAGE_SCAN_ACTIVITY,                     -                                                               struct HCI_Read_Page_Scan_ActivityCompStru */
/* HCI_OPS_WRITE_PAGE_SCAN_ACTIVITY,                    struct HCI_Write_Page_Scan_ActivityStru,                        struct HCI_Write_Page_Scan_ActivityCompStru */
/* HCI_OPS_READ_INQUIRY_SCAN_ACTIVITY,                  -                                                               struct HCI_Read_Inquiry_Scan_ActivityCompStru */
/* HCI_OPS_WRITE_INQUIRY_SCAN_ACTIVITY,                 struct HCI_Write_Inquiry_Scan_ActivityStru,                     struct HCI_Write_Inquiry_Scan_ActivityCompStru */
/* HCI_OPS_READ_AUTHENTICATION_ENABLE,                  -                                                               struct HCI_Read_Authentication_EnableCompStru */
/* HCI_OPS_WRITE_AUTHENTICATION_ENABLE,                 struct HCI_Write_Authentication_EnableStru,                     struct HCI_Write_Authentication_EnableCompStru */
/* HCI_OPS_READ_ENCRYPTION_MODE,                        -                                                               struct HCI_Read_Encryption_ModeCompStru */
/* HCI_OPS_WRITE_ENCRYPTION_MODE,                       struct HCI_Write_Encryption_ModeStru,                           struct HCI_Write_Encryption_ModeCompStru */
/* HCI_OPS_READ_CLASS_OF_DEVICE,                        -                                                               struct HCI_Read_Class_of_DeviceCompStru */
/* HCI_OPS_WRITE_CLASS_OF_DEVICE,                       struct HCI_Write_Class_of_DeviceStru,                           struct HCI_Write_Class_of_DeviceCompStru */
/* HCI_OPS_READ_VOICE_SETTING,                          -                                                               struct HCI_Read_Voice_SettingCompStru */
/* HCI_OPS_WRITE_VOICE_SETTING,                         struct HCI_Write_Voice_SettingStru,                             struct HCI_Write_Voice_SettingCompStru */
/* HCI_OPS_READ_AUTOMATIC_FLUSH_TIMEOUT,                struct HCI_Read_Automatic_Flush_TimeoutStru,                    struct HCI_Read_Automatic_Flush_TimeoutCompStru */
/* HCI_OPS_WRITE_AUTOMATIC_FLUSH_TIMEOUT,               struct HCI_Write_Automatic_Flush_TimeoutStru,                   struct HCI_Write_Automatic_Flush_TimeoutCompStru */
/* HCI_OPS_READ_NUM_BROADCAST_RETRANSMISSIONS,          -                                                               struct HCI_Read_Num_Broadcast_RetransmissionsCompStru */
/* HCI_OPS_WRITE_NUM_BROADCAST_RETRANSMISSIONS,         struct HCI_Write_Num_Broadcast_RetransmissionsStru,             struct HCI_Write_Num_Broadcast_RetransmissionsCompStru */
/* HCI_OPS_READ_HOLD_MODE_ACTIVITY,                     -                                                               struct HCI_Read_Hold_Mode_ActivityCompStru */
/* HCI_OPS_WRITE_HOLD_MODE_ACTIVITY,                    struct HCI_Write_Hold_Mode_ActivityStru,                        struct HCI_Write_Hold_Mode_ActivityCompStru */
/* HCI_OPS_READ_TRANSMIT_POWER_LEVEL,                   struct HCI_Read_Transmit_Power_LevelStru,                       struct HCI_Read_Transmit_Power_LevelCompStru */
/* HCI_OPS_READ_SYNCHRONOUS_FLOW_CONTROL_ENABLE,        -                                                               struct HCI_Read_Synchronous_Flow_Control_EnableCompStru */
/* HCI_OPS_WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE,       struct HCI_Write_Synchronous_Flow_Control_EnableStru,           struct HCI_Write_Synchronous_Flow_Control_EnableCompStru */
/* HCI_OPS_SET_CONTROLLER_TO_HOST_FLOW_CONTROL,         struct HCI_Set_Controller_To_Host_Flow_ControlStru,             struct HCI_Set_Controller_To_Host_Flow_ControlCompStru */
/* HCI_OPS_HOST_BUFFER_SIZE,                            struct HCI_Host_Buffer_SizeStru,                                struct HCI_Host_Buffer_SizeCompStru */
/* HCI_OPS_HOST_NUMBER_OF_COMPLETED_PACKETS,            struct HCI_Host_Number_Of_Completed_PacketsStru,                - */
/* HCI_OPS_READ_LINK_SUPERVISION_TIMEOUT,               struct HCI_Read_Link_Supervision_TimeoutStru,                   struct HCI_Read_Link_Supervision_TimeoutCompStru */
/* HCI_OPS_WRITE_LINK_SUPERVISION_TIMEOUT,              struct HCI_Write_Link_Supervision_TimeoutStru,                  struct HCI_Write_Link_Supervision_TimeoutCompStru */
/* HCI_OPS_READ_NUMBER_OF_SUPPORTED_IAC,                -                                                               struct HCI_Read_Number_Of_Supported_IACCompStru */
/* HCI_OPS_READ_CURRENT_IAC_LAP,                        -                                                               struct HCI_Read_Current_IAC_LAPCompStru */
/* HCI_OPS_WRITE_CURRENT_IAC_LAP,                       struct HCI_Write_Current_IAC_LAPStru,                           struct HCI_Write_Current_IAC_LAPCompStru */
/* HCI_OPS_READ_PAGE_SCAN_PERIOD_MODE,                  -                                                               struct HCI_Read_Page_Scan_Period_ModeCompStru */
/* HCI_OPS_WRITE_PAGE_SCAN_PERIOD_MODE,                 struct HCI_Write_Page_Scan_Period_ModeStru,                     struct HCI_Write_Page_Scan_Period_ModeCompStru */
/* HCI_OPS_READ_PAGE_SCAN_MODE,                         -                                                               struct HCI_Read_Page_Scan_ModeCompStru */
/* HCI_OPS_WRITE_PAGE_SCAN_MODE,                        struct HCI_Write_Page_Scan_ModeStru,                            struct HCI_Write_Page_Scan_ModeCompStru */
/* HCI_OPS_SET_AFH_HOST_CHANNEL_CLASSIFICATION,         struct HCI_Set_AFH_Host_Channel_ClassificationStru,             struct HCI_Set_AFH_Host_Channel_ClassificationCompStru */
/* HCI_OPS_READ_INQUIRY_SCAN_TYPE,                      -                                                               struct HCI_Read_Inquiry_Scan_TypeCompStru */
/* HCI_OPS_WRITE_INQUIRY_SCAN_TYPE,                     struct HCI_Write_Inquiry_Scan_TypeStru,                         struct HCI_Write_Inquiry_Scan_TypeCompStru */
/* HCI_OPS_READ_INQUIRY_MODE,                           -                                                               struct HCI_Read_Inquiry_ModeCompStru */
/* HCI_OPS_WRITE_INQUIRY_MODE,                          struct HCI_Write_Inquiry_ModeStru,                              struct HCI_Write_Inquiry_ModeCompStru */
/* HCI_OPS_READ_PAGE_SCAN_TYPE,                         -                                                               struct HCI_Read_Page_Scan_TypeCompStru */
/* HCI_OPS_WRITE_PAGE_SCAN_TYPE,                        struct HCI_Write_Page_Scan_TypeStru,                            struct HCI_Write_Page_Scan_TypeCompStru */
/* HCI_OPS_READ_AFH_CHANNEL_ASSESSMENT_MODE,            -                                                               struct HCI_Read_AFH_Channel_Assessment_ModeCompStru */
/* HCI_OPS_WRITE_AFH_CHANNEL_ASSESSMENT_MODE,           struct HCI_Write_AFH_Channel_Assessment_ModeStru,               struct HCI_Write_AFH_Channel_Assessment_ModeCompStru */
/* HCI_OPS_READ_EXTENDED_INQUIRY_RESPONSE,              -                                                               struct HCI_Read_Extended_Inquiry_ResponseCompStru */
/* HCI_OPS_WRITE_EXTENDED_INQUIRY_RESPONSE,             struct HCI_Write_Extended_Inquiry_ResponseStru,                 struct HCI_Write_Extended_Inquiry_ResponseCompStru */
/* HCI_OPS_REFRESH_ENCRYPTION_KEY,                      struct HCI_Refresh_Encryption_KeyStru,                          struct HCI_Encryption_Key_Refresh_CompleteEvStru */
/* HCI_OPS_READ_SIMPLE_PAIRING_MODE,                    -                                                               struct HCI_Read_Simple_Pairing_ModeCompStru */
/* HCI_OPS_WRITE_SIMPLE_PAIRING_MODE,                   struct HCI_Write_Simple_Pairing_ModeStru,                       struct HCI_Write_Simple_Pairing_ModeCompStru */
/* HCI_OPS_READ_LOCAL_OOB_DATA,                         -                                                               struct HCI_Read_Local_OOB_DataCompStru */
/* HCI_OPS_READ_INQUIRY_RESPONSE_TRANSMIT_POWER_LEVEL,  -                                                               struct HCI_Read_Inquiry_Response_Transmit_Power_LevelCompStru */
/* HCI_OPS_WRITE_INQUIRY_TRANSMIT_POWER_LEVEL,          struct HCI_Write_Inquiry_Transmit_Power_LevelStru,              struct HCI_Write_Inquiry_Transmit_Power_LevelCompStru */
/* HCI_OPS_READ_DEFAULT_ERRONEOUS_DATA_REPORTING,       -                                                               struct HCI_Read_Default_Erroneous_Data_ReportingCompStru */
/* HCI_OPS_WRITE_DEFAULT_ERRONEOUS_DATA_REPORTING,      struct HCI_Write_Default_Erroneous_Data_ReportingStru,          struct HCI_Write_Default_Erroneous_Data_ReportingCompStru */
/* HCI_OPS_ENHANCED_FLUSH,                              struct HCI_Enhanced_FlushStru,                                  struct HCI_Enhanced_Flush_CompleteEvStru */
/* HCI_OPS_SEND_KEYPRESS_NOTIFICATION,                  struct HCI_Send_Keypress_NotificationStru,                      struct HCI_Send_Keypress_NotificationCompStru */
/* HCI_OPS_READ_LOGICAL_LINK_ACCEPT_TIMEOUT,            -                                                               struct HCI_Read_Logical_Link_Accept_TimeoutCompStru */
/* HCI_OPS_WRITE_LOGICAL_LINK_ACCEPT_TIMEOUT,           struct HCI_Write_Logical_Link_Accept_TimeoutStru,               struct HCI_Write_Logical_Link_Accept_TimeoutCompStru */
/* HCI_OPS_SET_EVENT_MASK_PAGE_2,                       struct HCI_Set_Event_Mask_Page_2Stru,                           struct HCI_Set_Event_Mask_Page_2CompStru */
/* HCI_OPS_READ_LOCATION_DATA,                          -                                                               struct HCI_Read_Location_DataCompStru */
/* HCI_OPS_WRITE_LOCATION_DATA,                         struct HCI_Write_Location_DataStru,                             struct HCI_Write_Location_DataCompStru */
/* HCI_OPS_READ_FLOW_CONTROL_MODE,                      -                                                               struct HCI_Read_Flow_Control_ModeCompStru */
/* HCI_OPS_WRITE_FLOW_CONTROL_MODE,                     struct HCI_Write_Flow_Control_ModeStru,                         struct HCI_Write_Flow_Control_ModeCompStru */
/* HCI_OPS_READ_ENHANCED_TRANSMIT_POWER_LEVEL,          struct HCI_Read_Enhanced_Transmit_Power_LevelStru,              struct HCI_Read_Enhanced_Transmit_Power_LevelCompStru */
/* HCI_OPS_READ_LE_HOST_SUPPORT,                        -                                                               struct HCI_Read_LE_Host_SupportCompStru */
/* HCI_OPS_WRITE_LE_HOST_SUPPORT,                       struct HCI_Write_LE_Host_SupportStru,                           struct HCI_Write_LE_Host_SupportCompStru */

/*---------------------------------------------------------------------------------*/
/*                           Informational Param - 0x1000                          */
/*---------------------------------------------------------------------------------*/
/* HCI_OPS_READ_LOCAL_VERSION_INFORMATION,              -                                                               struct HCI_Read_Local_Version_InformationCompStru */
/* HCI_OPS_READ_LOCAL_SUPPORTED_COMMANDS,               -                                                               struct HCI_Read_Local_Supported_CommandsCompStru */
/* HCI_OPS_READ_LOCAL_SUPPORTED_FEATURES,               -                                                               struct HCI_Read_Local_Supported_FeaturesCompStru */
/* HCI_OPS_READ_LOCAL_EXTENDED_FEATURES,                struct HCI_Read_Local_Extended_FeaturesStru,                    struct HCI_Read_Local_Extended_FeaturesCompStru */
/* HCI_OPS_READ_BUFFER_SIZE,                            -                                                               struct HCI_Read_Buffer_SizeCompStru */
/* HCI_OPS_READ_COUNTRY_CODE,                           -                                                               struct HCI_Read_Country_CodeCompStru */
/* HCI_OPS_READ_BD_ADDR,                                -                                                               struct HCI_Read_BD_ADDRCompStru */
/* HCI_OPS_READ_DATA_BLOCK_SIZE,                        -                                                               struct HCI_Read_Data_Block_SizeCompStru */

/*---------------------------------------------------------------------------------*/
/*                              Status Param - 0x1400                              */
/*---------------------------------------------------------------------------------*/
/* HCI_OPS_READ_FAILED_CONTACT_COUNTER,                 struct HCI_Read_Failed_Contact_CounterStru,                     struct HCI_Read_Failed_Contact_CounterCompStru */
/* HCI_OPS_RESET_FAILED_CONTACT_COUNTER,                struct HCI_Reset_Failed_Contact_CounterStru,                    struct HCI_Reset_Failed_Contact_CounterCompStru */
/* HCI_OPS_READ_LINK_QUALITY,                           struct HCI_Read_Link_QualityStru,                               struct HCI_Read_Link_QualityCompStru */
/* HCI_OPS_READ_RSSI,                                   struct HCI_Read_RSSIStru,                                       struct HCI_Read_RSSICompStru */
/* HCI_OPS_READ_AFH_CHANNEL_MAP,                        struct HCI_Read_AFH_Channel_MapStru,                            struct HCI_Read_AFH_Channel_MapCompStru */
/* HCI_OPS_READ_CLOCK,                                  struct HCI_Read_ClockStru,                                      struct HCI_Read_ClockCompStru */
/* HCI_OPS_READ_ENCRYPTION_KEY_SIZE,                    struct HCI_Read_Encryption_Key_SizeStru,                        struct HCI_Read_Encryption_Key_SizeCompStru */

/*---------------------------------------------------------------------------------*/
/*                                 Testing - 0x1800                                */
/*---------------------------------------------------------------------------------*/
/* HCI_OPS_READ_LOOPBACK_MODE,                          -                                                               struct HCI_Read_Loopback_ModeCompStru */
/* HCI_OPS_WRITE_LOOPBACK_MODE,                         struct HCI_Write_Loopback_ModeStru,                             struct HCI_Write_Loopback_ModeCompStru */
/* HCI_OPS_ENABLE_DEVICE_UNDER_TEST_MODE,               -                                                               struct HCI_Enable_Device_Under_Test_ModeCompStru */
/* HCI_OPS_WRITE_SIMPLE_PAIRING_DEBUG_MODE,             struct HCI_Write_Simple_Pairing_Debug_ModeStru,                 struct HCI_Write_Simple_Pairing_Debug_ModeCompStru */

/*---------------------------------------------------------------------------------*/
/*                              LE Controller - 0x2000                             */
/*---------------------------------------------------------------------------------*/
/* HCI_OPS_LE_SET_EVENT_MASK,                           struct HCI_LE_Set_Event_MaskStru,                               struct HCI_LE_Set_Event_MaskCompStru */
/* HCI_OPS_LE_READ_BUFFER_SIZE,                         -                                                               struct HCI_LE_Read_Buffer_SizeCompStru */
/* HCI_OPS_LE_READ_LOCAL_SUPPORTED_FEATURES,            -                                                               struct HCI_LE_Read_Local_Supported_FeaturesCompStru */
/* HCI_OPS_LE_SET_RANDOM_ADDRESS,                       struct HCI_LE_Set_Random_AddressStru,                           struct HCI_LE_Set_Random_AddressCompStru */
/* HCI_OPS_LE_SET_ADVERTISING_PARAMETERS,               struct HCI_LE_Set_Advertising_ParametersStru,                   struct HCI_LE_Set_Advertising_ParametersCompStru */
/* HCI_OPS_LE_READ_ADVERTISING_CHANNEL_TX_POWER,        -                                                               struct HCI_LE_Read_Advertising_Channel_Tx_PowerCompStru */
/* HCI_OPS_LE_SET_ADVERTISING_DATA,                     struct HCI_LE_Set_Advertising_DataStru,                         struct HCI_LE_Set_Advertising_DataCompStru */
/* HCI_OPS_LE_SET_SCAN_RESPONSE_DATA,                   struct HCI_LE_Set_Scan_Response_DataStru,                       struct HCI_LE_Set_Scan_Response_DataCompStru */
/* HCI_OPS_LE_SET_ADVERTISE_ENABLE,                     struct HCI_LE_Set_Advertise_EnableStru,                         struct HCI_LE_Set_Advertise_EnableCompStru */
/* HCI_OPS_LE_SET_SCAN_PARAMETERS,                      struct HCI_LE_Set_Scan_ParametersStru,                          struct HCI_LE_Set_Scan_ParametersCompStru */
/* HCI_OPS_LE_SET_SCAN_ENABLE,                          struct HCI_LE_Set_Scan_EnableStru,                              struct HCI_LE_Set_Scan_EnableCompStru */
/* HCI_OPS_LE_CREATE_CONNECTION,                        struct HCI_LE_Create_ConnectionStru,                            struct HCI_LE_Connection_CompleteEvStru */
/* HCI_OPS_LE_CREATE_CONNECTION_CANCEL,                 -                                                               struct HCI_LE_Create_Connection_CancelCompStru */
/* HCI_OPS_LE_READ_WHITE_LIST_SIZE,                     -                                                               struct HCI_LE_Read_White_List_SizeCompStru */
/* HCI_OPS_LE_CLEAR_WHITE_LIST,                         -                                                               struct HCI_LE_Clear_White_ListCompStru */
/* HCI_OPS_LE_ADD_DEVICE_TO_WHITE_LIST,                 struct HCI_LE_Add_Device_To_White_ListStru,                     struct HCI_LE_Add_Device_To_White_ListCompStru */
/* HCI_OPS_LE_REMOVE_DEVICE_FROM_WHITE_LIST,            struct HCI_LE_Remove_Device_From_White_ListStru,                struct HCI_LE_Remove_Device_From_White_ListCompStru */
/* HCI_OPS_LE_CONNECTION_UPDATE,                        struct HCI_LE_Connection_UpdateStru,                            struct HCI_LE_Connection_Update_CompleteEvStru */
/* HCI_OPS_LE_SET_HOST_CHANNEL_CLASSIFICATION,          struct HCI_LE_Set_Host_Channel_ClassificationStru,              struct HCI_LE_Set_Host_Channel_ClassificationCompStru */
/* HCI_OPS_LE_READ_CHANNEL_MAP,                         struct HCI_LE_Read_Channel_MapStru,                             struct HCI_LE_Read_Channel_MapCompStru */
/* HCI_OPS_LE_READ_REMOTE_USED_FEATURES,                struct HCI_LE_Read_Remote_Used_FeaturesStru,                    struct HCI_LE_Read_Remote_Used_Features_CompleteEvStru */
/* HCI_OPS_LE_ENCRYPT,                                  struct HCI_LE_EncryptStru,                                      struct HCI_LE_EncryptCompStru */
/* HCI_OPS_LE_RAND,                                     -                                                               struct HCI_LE_RandCompStru */
/* HCI_OPS_LE_START_ENCRYPTION,                         struct HCI_LE_Start_EncryptionStru,                             struct HCI_Encryption_Key_Refresh_CompleteEvStru */
/* HCI_OPS_LE_LONG_TERM_KEY_REQUEST_REPLY,              struct HCI_LE_Long_Term_Key_Request_ReplyStru,                  struct HCI_LE_Long_Term_Key_Request_ReplyCompStru */
/* HCI_OPS_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY,   struct HCI_LE_Long_Term_Key_Requested_Negative_ReplyStru,       struct HCI_LE_Long_Term_Key_Requested_Negative_ReplyCompStru */
/* HCI_OPS_LE_READ_SUPPORTED_STATES,                    -                                                               struct HCI_LE_Read_Supported_StatesCompStru */
/* HCI_OPS_LE_RECEIVER_TEST,                            struct HCI_LE_Receiver_TestStru,                                struct HCI_LE_Receiver_TestCompStru */
/* HCI_OPS_LE_TRANSMITTER_TEST,                         struct HCI_LE_Transmitter_TestStru,                             struct HCI_LE_Transmitter_TestCompStru */
/* HCI_OPS_LE_TEST_END,                                 -                                                               struct HCI_LE_Test_EndCompStru */


/********************************************************************************
 *																				*
 *								CONST DEFINATION								*
 *																				*
 ********************************************************************************/
#define GAP_INVALID_CONNECTION_HANDLE       0xFFFF /* Invalid Primary Controller connection handle */
 
/* Additional internal error codes: */
#define HCI_STATUS_HOST_NOT_READY           0x70   /* The Host Stack is not ready to accept any requests from the appliation */
#define HCI_STATUS_CONTROLLER_UNAVAILABLE   0x71   /* Fails to communicate with the specified Controller */
#define HCI_STATUS_LOCAL_HOST_TIMEOUT       0x72   /* The Host Stack fails to receive the complete event from the Controller within the internal timeout */
#define HCI_STATUS_USER_TERMINATED          0x73   /* The operation is terminated by the user, e.g. cancel the inquiry */
#define HCI_STATUS_INVALID_PARAMETERS		0X74   /* Invalid parameter(s) is/are used to call the function */

#define CLS_DEV_LEN				3			/* class of device length */

/* BOOL */
typedef enum _tGAP_Bool {
    GAP_FALSE,                                 /* Value 0 */
    GAP_TRUE                                   /* Value 1 */
} tGAP_Bool;

/* Possible Controller type values */
typedef enum _tGAP_Controller_Type {
    GAP_CONTROLLER_BREDR,                      /* Bluetooth */
    GAP_CONTROLLER_80211,                      /* WIFI */
    GAP_CONTROLLER_ECMA368,                    /* UWB */
/* 0x03 - 0xEF Reserved, 0xF0 - 0xFF Vendor-Specific*/
    GAP_CONTROLLER_FAKEPAL_UDP = 0xEF,         /* Fake PAL based on UDP, for test only */
    GAP_CONTROLLER_VENDOR_1 = 0xF0,
    GAP_CONTROLLER_VENDOR_2,
    GAP_CONTROLLER_VENDOR_3,
    GAP_CONTROLLER_VENDOR_4,
    GAP_CONTROLLER_VENDOR_5,
    GAP_CONTROLLER_VENDOR_6,
    GAP_CONTROLLER_VENDOR_7,
    GAP_CONTROLLER_VENDOR_8,
    GAP_CONTROLLER_VENDOR_9,
    GAP_CONTROLLER_VENDOR_10,
    GAP_CONTROLLER_VENDOR_11,
    GAP_CONTROLLER_VENDOR_12,
    GAP_CONTROLLER_VENDOR_13,
    GAP_CONTROLLER_VENDOR_14,
    GAP_CONTROLLER_VENDOR_15,
} tGAP_Controller_Type;

/* Possible AMP type values */
typedef enum _tGAP_AMP_Type {
    GAP_AMP_80211,                      /* WIFI */
    GAP_AMP_ECMA368,                    /* UWB */
/* 0x03 - 0xEF Reserved, 0xF0 - 0xFF Vendor-Specific*/
    GAP_AMP_FAKEPAL_UDP = 0xEF,         /* Fake PAL based on UDP, for test only */
    GAP_AMP_VENDOR_1 = 0xF0,
    GAP_AMP_VENDOR_2,
    GAP_AMP_VENDOR_3,
    GAP_AMP_VENDOR_4,
    GAP_AMP_VENDOR_5,
    GAP_AMP_VENDOR_6,
    GAP_AMP_VENDOR_7,
    GAP_AMP_VENDOR_8,
    GAP_AMP_VENDOR_9,
    GAP_AMP_VENDOR_10,
    GAP_AMP_VENDOR_11,
    GAP_AMP_VENDOR_12,
    GAP_AMP_VENDOR_13,
    GAP_AMP_VENDOR_14,
    GAP_AMP_VENDOR_15,
} tGAP_AMP_Type;

/* Possible return values of GAP_IndicationCbk */
typedef enum _tGAP_IndCbk_Result {
    GAP_IND_RESULT_NOT_CARE,    /* The application will not respond to this indication, so the Host Stack shall make its own decision */
    GAP_IND_RESULT_PENDING,     /* The application will respond to this indication later by itself */
    GAP_IND_RESULT_ACCEPT,      /* The application requires a possitive reply to this indication, default value shall be used if the reply requires additional parameters */
    GAP_IND_RESULT_REJECT       /* The application requires a negative reply to this indication */
} tGAP_IndCbk_Result;

/* Possible indication type values */
typedef enum _tGAP_Ind_Type {
    /* Inquiry related events */
    GAP_IND_INQUIRY_COMPLETE = HCI_EVI_INQUIRY_COMPLETE, /* Report the Bluetooth Inquiry complete Event to the application, parameter: HCI_Inquiry_CompleteEvStru */
    GAP_IND_INQUIRY_RESULT = HCI_EVI_INQUIRY_RESULT,     /* Report a device discovered to the application, parameter: GAP_RemoteDeviceInfoStru */

    /* Bluetooth Connection related events */
    GAP_IND_CONNECTION_EVENT, /* Report the Connection Complete event to the application, parameter: GAP_ConnectionEventStru */
    GAP_IND_CONNECTION_REQUEST, /* Report the Connection Request event to the application, parameter: HCI_Connection_RequestEvStru */

    /* Legacy Security related events - Authentication */
    GAP_IND_PIN_CODE_REQ = HCI_EVO_PIN_CODE,     /* Request PIN code from the application, parameter: struct HCI_PIN_Code_RequestEvStru */
    GAP_IND_LINK_KEY_REQUEST = HCI_EVI_LINK_KEY_REQUEST, /* Request Link Key from the application, parameter: HCI_Security_Link_Key_Ask4Stru */
    GAP_IND_LINK_KEY_NOTIFICATION = HCI_EVI_LINK_KEY_NOTIFICATION, /* Report the new link key created, parameter: HCI_Security_Link_Key_SaveStru */
    GAP_IND_AUTHENTICATION_FAILURE = HCI_EVI_AUTHENTICATION_COMPLETE, /* Notify the application an authentication failure, parameter: GAP_AuthenticationFailureStru */

    /* Inquiry related events - Name discovery */
    GAP_IND_REMOTE_DEVICE_NAME_UPDATED = HCI_EVI_REMOTE_NAME_REQUEST_COMPLETE, /* Report the latest remote device name read to the application, parameter: NULL terminated UTF-8 encoded string */

    /* Secure Simple Pairing and LE Seurity related events - Authentication */
    GAP_IND_USER_CONFIRMATION_REQUEST = HCI_EVI_USER_CONFIRMATION_REQUEST, /* Report the User Confirmation Request event to the application, parameter: HCI_User_Confirmation_RequestEvStru */
    GAP_IND_USER_PASSKEY_REQUEST = HCI_EVI_USER_PASSKEY_REQUEST, /* Report the User Passkey Request event to the application, parameter: UINT8 bd_addr[6] */
    GAP_IND_SIMPLE_PAIRING_COMPLETE = HCI_EVI_SIMPLE_PAIRING_COMPLETE, /* Report the Simple Pairing Complete event to the application, parameter: HCI_Simple_Pairing_CompleteEvStru */
    GAP_IND_USER_PASSKEY_NOTIFICATION = HCI_EVI_USER_PASSKEY_NOTIFICATION, /* Report the Passkey to the application, parameter: HCI_User_Passkey_NotificationEvStru */

    /* Legacy Security related events - Authorization */
    GAP_IND_AUTHORIZATION_REQUEST = HCI_EVO_AUTHORIZATION_REQUEST, /* Request authorization confirmation from the application, parameter: tGAP_AuthorizationReqStru */
    GAP_IND_AUTHORIZATION_ABORT = HCI_EVO_AUTHORIZATION_ABORT, /* Report the authorization aborted indication to the application, parameter: tGAP_AuthorizationReqStru */

    GAP_IND_ALL = 0xFFFFFFFF   /* Deal with all possible indications, only used in GAP_RegisterIndCbk */
} tGAP_Ind_Type;

/* Possible indication type values */
typedef enum _tGAP_InqEv_Type {
    GAP_INQ_LE_ADV_IND,             /* LE Advertising Report Event - Connectable undirected advertising */
    GAP_INQ_LE_ADV_DIRECT_IND,      /* LE Advertising Report Event - Connectable directed advertising */
    GAP_INQ_LE_ADV_SCAN_IND,        /* LE Advertising Report Event - Scannable undirected advertising */
    GAP_INQ_LE_ADV_NONCONN_IND,     /* LE Advertising Report Event - Non connectable undirected advertising */
    GAP_INQ_LE_SCAN_RSP,            /* LE Advertising Report Event - Scan Response */
    GAP_INQ_BREDR_IND = 0xFF        /* BE/EDR Inquiry Result Event */
} tGAP_InqEv_Type;

/* Possible Requested Link Key type values */
typedef enum _tGAP_Requested_Key_Type {
    GAP_REQUEST_BREDR_LK,    /* BR/EDR link key */
    GAP_REQUEST_GAMP_LK,     /* Generic AMP Link Key */
    GAP_REQUEST_DAMP_LK,     /* Dedicated AMP Link Key */
    GAP_REQUEST_LE_LTK       /* LE Long Term Key */
} tGAP_Requested_Key_Type;

/* Possible IO Capability values */
typedef enum _tGAP_IO_Capability {
    GAP_IO_CAPABILITY_DISPLAY_ONLY,    /* DisplayOnly */
    GAP_IO_CAPABILITY_DISPLAY_YESNO,   /* DisplayYesNo */
    GAP_IO_CAPABILITY_KEYBOARD_ONLY,   /* KeyboardOnly */
    GAP_IO_CAPABILITY_NOINPUT_NOUTPUT, /* NoInputNoOutput */
    GAP_IO_CAPABILITY_KEYBOARD_DISPLAY,/* KeyboardDisplay */
} tGAP_IO_Capability;

/* Possible LE Authentication Requirements */
typedef enum _tGAP_LE_AuthReq {
    GAP_LE_AUTHREQ_MITM_NOTREQUIRED_NOBONGDING, /* MITM protection not required and no bonding requested */
    GAP_LE_AUTHREQ_MITM_NOTREQUIRED_BONGDING,   /* MITM protection not required and bonding requested */
    GAP_LE_AUTHREQ_MITM_REQUIRED_NOBONGDING = 4,/* MITM protection required and no bonding requested */
    GAP_LE_AUTHREQ_MITM_REQUIRED_BONGDING = 5,  /* MITM protection required and bonding requested */
} tGAP_LE_AuthReq;

/* Possible LE Authentication Requirements */
typedef enum _tGAP_BREDR_AuthReq {
    GAP_BE_AUTHREQ_MITM_NOTREQUIRED_NOBONGDING,         /* MITM protection not required and no bonding requested */
    GAP_BE_AUTHREQ_MITM_REQUIRED_NOBONGDING,            /* MITM protection required and no bonding requested */
    GAP_BE_AUTHREQ_MITM_NOTREQUIRED_DEDICATED_BONGDING, /* MITM protection not required and dedicated bonding requested */
    GAP_BE_AUTHREQ_MITM_REQUIRED_DEDICATED_BONGDING,    /* MITM protection required and dedicated bonding requested */
    GAP_BE_AUTHREQ_MITM_NOTREQUIRED_GENERAL_BONGDING,   /* MITM protection not required and general bonding requested */
    GAP_BE_AUTHREQ_MITM_REQUIRED_GENERAL_BONGDING,      /* MITM protection required and general bonding requested */
} tGAP_BREDR_AuthReq;

/* Possible LE Discovery Mode */
typedef enum _tGAP_LE_DiscoveryMode {
	GAP_LE_NONE_DISCOVERABLE,       /* None discoverable mode - LE_DISCOVERYMODE_NONE */
	GAP_LE_LIMITED_DISCOVERABLE,    /* Limited discoverable mode - LE_DISCOVERYMODE_LIMITED */
	GAP_LE_GENERAL_DISCOVERABLE     /* General discoverable mode - LE_DISCOVERYMODE_GENERAL */
} tGAP_LE_DiscoveryMode;

/* Possible LE Advertising/Scanning Speed */
typedef enum _tGAP_LE_AdvScanSpeed {
	GAP_LE_ADV_SCAN_FAST,       /* Tgap(scan_fast) or Tgap(adv_fast) - LE_FAST */
	GAP_LE_ADV_SCAN_LOW,        /* Tgap(scan_slow) or Tgap(adv_slow) - LE_SLOW */
	GAP_LE_ADV_SCAN_AUTO        /* Select timer automatically - LE_FAST_AUTO_SLOW */
} tGAP_LE_AdvScanSpeed;

/* Possible ACL Link Type */
typedef enum _tGAP_ACLLinkType {
	GAP_LINK_TYPE_LE = 1,       /* LE ACL Link */
	GAP_LINK_TYPE_BREDR         /* BR/EDR ACL Link */
} tGAP_ACLLinkType;

/* Possible values of GAP_DeviceDiscoveryReqStru::flag */ 
#define GAP_DEVICE_DISCOVERY_BREDR		        0x0001      /* Only initiate BR/EDR device discovery - Ignored by LE-only Controller */
#define GAP_DEVICE_DISCOVERY_LE		            0x0002      /* Only initiate LE device discovery - Ignored by BR/EDR-only Controller */
#define GAP_DEVICE_DISCOVERY_BOTH_BREDR_LE      (GAP_DEVICE_DISCOVERY_BREDR|GAP_DEVICE_DISCOVERY_LE)  /* Initiated both BR/EDR and LE device discovery - May initiate only the one supported by the Controller */
#define GAP_DEVICE_DISCOVERY_LIMITED_INQUIRY    0x0010      /* Limited Discovery if set, otherwise General Discovery */
#define GAP_DEVICE_DISCOVERY_DEVICE_CLASS_SET   0x0020      /* If set, use the specified device of class to filter the inquiry result, for BR/EDR inquiry only */
#define GAP_DEVICE_DISCOVERY_DEVICE_ADDRESS_SET 0x0040      /* If set, use the specified device address to filter the inquiry result */
#define GAP_DEVICE_DISCOVERY_LE_WHITELIST_ONLY  0x0100      /* If set, only report the devices in the While list in case of LE inquiry. Ignored in case of BR/EDR inquiry. */
#define GAP_DEVICE_DISCOVERY_PERIODICALLY       0x1000      /* If set, configure the BR/EDR Controller to enter the Periodic Inquiry Mode. It is ignored if GAP_DEVICE_DISCOVERY_LE_ONLY is set. */
#define GAP_DEVICE_DISCOVERY_NAME_REQUIRED      0x2000      /* If set, initiate the name discovery procedure after inquiry complete. It is ignored in the BR/EDR Periodic Inquiry Mode. */

/* Possible Class of Device - Represented as an 32bit unsigned integer */
/* Major Service Classe bit mask */
#define GAP_COD_SERVICE_BITS(c)		    (c & 0xFFE000)  /* The major service classes field */
#define GAP_COD_DEVICE_MAJOR_BITS(c)    (c & 0x001F00)  /* The major device classes field */
#define GAP_COD_DEVICE_MINOR_BITS(c)    (c & 0x0000FC)  /* The minor device classes field */

#define GAP_COD_SERVICE_LDM 		0x002000        /* Limited Discoverable Mode */
#define GAP_COD_SERVICE_POSITION	0x010000        /* Positioning (Location identification) */
#define GAP_COD_SERVICE_NETWORK		0x020000        /* Networking (LAN, Ad hoc, ...) */
#define GAP_COD_SERVICE_RENDERING	0x040000        /* Rending (Printing, Speaker, ...) */
#define GAP_COD_SERVICE_CAPTURING	0x080000        /* Capturing (Scanner, Microphone, ...) */
#define GAP_COD_SERVICE_OBJECT		0x100000        /* Object Transfer (v-Inbox, v-Folder, ...) */
#define GAP_COD_SERVICE_AUDIO		0x200000        /* Audio (Speaker, Microphone, Headset service, ...) */
#define GAP_COD_SERVICE_TELEPHONY	0x400000        /* Telephony (Cordless telephony, Modem, Headset service, ...) */
#define GAP_COD_SERVICE_INFORMATION	0x800000        /* Information (WEB-server, WAP-server, ...) */

/* Major Device Classes bit mask */			                                    
#define GAP_COD_DEVICE_MISCELLANEOUS	0x000000    /* Major Device Class - Miscellaneous */
#define GAP_COD_DEVICE_COMPUTER			0x000100    /* Major Device Class - Computer (desktop, notebook, PDA, organizers, ...) */
#define GAP_COD_DEVICE_PHONE			0x000200    /* Major Device Class - Phone (cellular, cordless, payphone, modem, ...) */
#define GAP_COD_DEVICE_LAP				0x000300    /* Major Device Class - LAN/Network Access Point */
#define GAP_COD_DEVICE_AV			    0x000400    /* Major Device Class - Audio/Video (headset, speaker, stereo, video display, vcr...) */
#define GAP_COD_DEVICE_PERIPHERAL		0x000500    /* Major Device Class - Peripheral (mouse, joystick, keyboards, ...) */
#define GAP_COD_DEVICE_IMAGING          0x000600    /* Major Device Class - Imaging (printing, scanner, camera, display, ...) */
#define GAP_COD_DEVICE_WEARABLE			0x000700    /* Major Device Class - Wearable */
#define GAP_COD_DEVICE_TOY			    0x000800    /* Major Device Class - Toy */
#define GAP_COD_DEVICE_HEALTH		    0x000900    /* Major Device Class - Health */
#define GAP_COD_DEVICE_UNCLASSIFIED		0x001F00    /* Major Device Class - Uncategorized, specific device code not specified */

/* Minor Device Class - Computer major class */
#define GAP_COD_COMPUTER_UNCLASSIFIED   (GAP_COD_DEVICE_COMPUTER | 0x000000) 
#define GAP_COD_COMPUTER_DESKTOP        (GAP_COD_DEVICE_COMPUTER | 0x000004)
#define GAP_COD_COMPUTER_SERVER         (GAP_COD_DEVICE_COMPUTER | 0x000008)
#define GAP_COD_COMPUTER_LAPTOP         (GAP_COD_DEVICE_COMPUTER | 0x00000C)
#define GAP_COD_COMPUTER_HANDHELD       (GAP_COD_DEVICE_COMPUTER | 0x000010)
#define GAP_COD_COMPUTER_PALMSIZED      (GAP_COD_DEVICE_COMPUTER | 0x000014)
#define GAP_COD_COMPUTER_WEARABLE       (GAP_COD_DEVICE_COMPUTER | 0x000018)

/* Minor Device Class - Phone major class*/
#define GAP_COD_PHONE_UNCLASSIFIED   	(GAP_COD_DEVICE_PHONE | 0x000000) 
#define GAP_COD_PHONE_CELLULAR         	(GAP_COD_DEVICE_PHONE | 0x000004)
#define GAP_COD_PHONE_CORDLESS        	(GAP_COD_DEVICE_PHONE | 0x000008)
#define GAP_COD_PHONE_SMARTPHONE     	(GAP_COD_DEVICE_PHONE | 0x00000C)
#define GAP_COD_PHONE_WIREDMODEM    	(GAP_COD_DEVICE_PHONE | 0x000010)
#define GAP_COD_PHONE_COMMONISDNACCESS	(GAP_COD_DEVICE_PHONE | 0x000014)
#define GAP_COD_PHONE_SIMCARDREADER		(GAP_COD_DEVICE_PHONE | 0x000018)

/* Minor Device Class - LAN/Network access point major class*/
#define GAP_COD_LAP_FULLY_AVAILABLE    	(GAP_COD_DEVICE_LAP | 0x000000)
#define GAP_COD_LAP_17_UTILIZED         (GAP_COD_DEVICE_LAP | 0x000020)
#define GAP_COD_LAP_33_UTILIZED         (GAP_COD_DEVICE_LAP | 0x000040)
#define GAP_COD_LAP_50_UTILIZED         (GAP_COD_DEVICE_LAP | 0x000060)
#define GAP_COD_LAP_67_UTILIZED         (GAP_COD_DEVICE_LAP | 0x000080)
#define GAP_COD_LAP_83_UTILIZED         (GAP_COD_DEVICE_LAP | 0x0000A0)
#define GAP_COD_LAP_99_UTILIZED         (GAP_COD_DEVICE_LAP | 0x0000C0)
#define GAP_COD_LAP_UNAVAILABLE         (GAP_COD_DEVICE_LAP | 0x0000E0)

/* Minor Device Class - Audio/Video major class*/
#define GAP_COD_AV_UNCLASSIFIED         (GAP_COD_DEVICE_AV | 0x000000)
#define GAP_COD_AV_HEADSET              (GAP_COD_DEVICE_AV | 0x000004)
#define GAP_COD_AV_HANDSFREE            (GAP_COD_DEVICE_AV | 0x000008)
#define GAP_COD_AV_HEADANDHAND          (GAP_COD_DEVICE_AV | 0x00000C)
#define GAP_COD_AV_MICROPHONE           (GAP_COD_DEVICE_AV | 0x000010) 
#define GAP_COD_AV_LOUD_SPEAKER         (GAP_COD_DEVICE_AV | 0x000014)
#define GAP_COD_AV_HEADPHONES           (GAP_COD_DEVICE_AV | 0x000018)
#define GAP_COD_AV_PORTABLE_AUDIO       (GAP_COD_DEVICE_AV | 0x00001C)
#define GAP_COD_AV_CAR_AUDIO            (GAP_COD_DEVICE_AV | 0x000020)
#define GAP_COD_AV_SETTOPBOX            (GAP_COD_DEVICE_AV | 0x000024)
#define GAP_COD_AV_HIFI_AUDIO           (GAP_COD_DEVICE_AV | 0x000028)
#define GAP_COD_AV_VCR                  (GAP_COD_DEVICE_AV | 0x00002C)
#define GAP_COD_AV_VIDEO_CAMERA         (GAP_COD_DEVICE_AV | 0x000030)
#define GAP_COD_AV_CAMCORDER            (GAP_COD_DEVICE_AV | 0x000034)
#define GAP_COD_AV_VIDEO_MONITOR        (GAP_COD_DEVICE_AV | 0x000038)
#define GAP_COD_AV_DISPLAY_AND_SPEAKER  (GAP_COD_DEVICE_AV | 0x00003C) 
#define GAP_COD_AV_VIDEO_CONFERENCING   (GAP_COD_DEVICE_AV | 0x000040)
#define GAP_COD_AV_GAME_OR_TOY          (GAP_COD_DEVICE_AV | 0x000048)

/* Minor Device Class - Peripheral major class*/
#define GAP_COD_PERIPHERAL_UNCLASSIFIED  	(GAP_COD_DEVICE_PERIPHERAL | 0x000000) 
#define GAP_COD_PERIPHERAL_JOYSTICK       	(GAP_COD_DEVICE_PERIPHERAL | 0x000004)
#define GAP_COD_PERIPHERAL_GAMEPAD       	(GAP_COD_DEVICE_PERIPHERAL | 0x000008)
#define GAP_COD_PERIPHERAL_REMCONTROL    	(GAP_COD_DEVICE_PERIPHERAL | 0x00000C)
#define GAP_COD_PERIPHERAL_SENSE          	(GAP_COD_DEVICE_PERIPHERAL | 0x000010)
#define GAP_COD_PERIPHERAL_TABLET			(GAP_COD_DEVICE_PERIPHERAL | 0x000014)
#define GAP_COD_PERIPHERAL_SIMCARDREADER	(GAP_COD_DEVICE_PERIPHERAL | 0x000018)
#define GAP_COD_PERIPHERAL_KEYBOARD      	(GAP_COD_DEVICE_PERIPHERAL | 0x000040)    
#define GAP_COD_PERIPHERAL_POINT           	(GAP_COD_DEVICE_PERIPHERAL | 0x000080)
#define GAP_COD_PERIPHERAL_KEYORPOINT    	(GAP_COD_DEVICE_PERIPHERAL | 0x0000C0)

/* Minor Device Class - Imaging major class*/
#define GAP_COD_IMAGING_DISPLAY         (GAP_COD_DEVICE_IMAGING | 0x000010)
#define GAP_COD_IMAGING_CAMERA          (GAP_COD_DEVICE_IMAGING | 0x000020)
#define GAP_COD_IMAGING_SCANNER         (GAP_COD_DEVICE_IMAGING | 0x000040)
#define GAP_COD_IMAGING_PRINTER         (GAP_COD_DEVICE_IMAGING | 0x000080)

/* Minor Device Class - Wearable major class*/
#define GAP_COD_WERABLE_WATCH			(GAP_COD_DEVICE_WEARABLE | 0x000004)
#define GAP_COD_WERABLE_PAGER			(GAP_COD_DEVICE_WEARABLE | 0x000008)
#define GAP_COD_WERABLE_JACKET			(GAP_COD_DEVICE_WEARABLE | 0x00000C)
#define GAP_COD_WERABLE_HELMET			(GAP_COD_DEVICE_WEARABLE | 0x000010)
#define GAP_COD_WERABLE_GLASSES			(GAP_COD_DEVICE_WEARABLE | 0x000014)

/* Minor Device Class - Toy major class*/
#define GAP_COD_TOY_ROBOT				(GAP_COD_DEVICE_TOY | 0x000004)
#define GAP_COD_TOY_VEHICLE				(GAP_COD_DEVICE_TOY | 0x000008)
#define GAP_COD_TOY_DOLL				(GAP_COD_DEVICE_TOY | 0x00000C)
#define GAP_COD_TOY_CONROLLER			(GAP_COD_DEVICE_TOY | 0x000010)
#define GAP_COD_TOY_GAME    			(GAP_COD_DEVICE_TOY | 0x000014)

/* Minor Device Class - Health major class*/
#define GAP_COD_HEALTH_BLOOD_PRESURE	(GAP_COD_DEVICE_HEALTH | 0x000004)
#define GAP_COD_HEALTH_THERMOMETER		(GAP_COD_DEVICE_HEALTH | 0x000008)
#define GAP_COD_HEALTH_WEIGHING_SCALE	(GAP_COD_DEVICE_HEALTH | 0x00000C)
#define GAP_COD_HEALTH_GLUCOSE_METER	(GAP_COD_DEVICE_HEALTH | 0x000010)
#define GAP_COD_HEALTH_PULSE_OXIMETER	(GAP_COD_DEVICE_HEALTH | 0x000014)
#define GAP_COD_HEALTH_RATE_MONITOR 	(GAP_COD_DEVICE_HEALTH | 0x000018)
#define GAP_COD_HEALTH_DATA_DISPLAY 	(GAP_COD_DEVICE_HEALTH | 0x00001C)

/* Possible LE Distribution flags */
#define GAP_LE_DISTRIBUTION_ENCKEY      0x01
#define GAP_LE_DISTRIBUTION_IDKEY       0x02
#define GAP_LE_DISTRIBUTION_SIGN        0x04

/* Possible Visual Mode*/
#define GAP_VM_DISABLE			        0x0000 
#define GAP_VM_LIMIT_DISCOV		        0x0002		
#define GAP_VM_GENERAL_DISCOV       	0x0003		
#define GAP_VM_PAGE				        0x0004		
#define GAP_VM_PAIR				        0x0008		
#define GAP_VM_DISCOV			        GAP_VM_GENERAL_DISCOV 
#define GAP_VM_DEFAULT			        (GAP_VM_GENERAL_DISCOV | GAP_VM_PAGE | GAP_VM_PAIR) 

/********************************************************************************
 *																				*
 *								TYPE DEFINATION								    *
 *																				*
 ********************************************************************************/
struct GAP_RegisterTLCfmStru {
    UINT8                 amp_id;  /* AMP ID assigned by the Host Stack to identify this transport layer. */
    tGAP_Controller_Type  type;    /* Type of the Controller connected by this transport layer. */
};

struct GAP_ExecuteCommandCfmStru {
    UINT32 opindex;         /* Specifies the command confirmed, e.g. HCI_OPS_INQUIRY. It also specifies the content of the event_parameter. */
    UINT32 size;            /* Specifies the size, in bytes, of the event_parameter buffer. */
    void   *event_parameter;/* Pointer to a command specific data structure contains the
                               parameters of the complete event, e.g. HCI_Inquiry_CompleteEvStru. */
};

struct GAP_DeviceDiscoveryCfmStru {
    UINT32                          number_response;    /* Number of devices reported in the device discovery procedure. The duplicate responses are filtered.  */
    struct GAP_RemoteDeviceInfoStru *device_array[1];   /* List of devices reported.  */
};

struct GAP_DeviceDiscoveryReqStru {
    UINT32  flag;                   /* Bit mask specifies the device discovery requirement, e.g. GAP_DEVICE_DISCOVERY_LIMITED_INQUIRY.
                                       GAP_DEVICE_DISCOVERY_LE_ONLY and GAP_DEVICE_DISCOVERY_PERIODICALLY should not be used at the same
                                       time, or the GAP_DEVICE_DISCOVERY_PERIODICALLY is ignored.
                                       GAP_DEVICE_DISCOVERY_PERIODICALLY and GAP_DEVICE_DISCOVERY_NAME_REQUIRED should not be used at the same
                                       time, or the GAP_DEVICE_DISCOVERY_NAME_REQUIRED is ignored if GAP_DEVICE_DISCOVERY_PERIODICALLY takes
                                       effect.*/
	UINT32  class_of_device;        /* Class of Device interested, only the lower 3bytes are used, little endian. 
	                                   The application can specify one or all of the major class, minor class and service class field.
	                                   Ignored if GAP_DEVICE_DISCOVERY_DEVICE_CLASS_SET is not set in the flag. */
	UINT8   addr[BD_ADDR_LEN];      /* Device address interested. It can be either a public address or a random address.
	                                   Ignored if GAP_DEVICE_DISCOVERY_DEVICE_ADDRESS_SET is not set in the flag. */
	UINT8   inquiry_len;            /* Maximum amount of time duration for each Inquiry operation. Range: 0x01 - 0x30 (*1.28s).
	                                   If both the BR/EDR and LE inquiries are executed, the total duration shall be inquiry_len*2.
	                                   The name discovery procedure is not controlled by this duration. */
	UINT8   num_responses;          /* Maximum number of responses from each inquiry before this inquiry is halted. 0 means unlimited number of responses.
	                                   If num_responses is not 0 and both the BR/EDR and LE inquiries are executed, the maximum responses available shall
	                                   be num_responses*2. */
	UINT16  max_period_len;         /* Maximum amout of time specified between consecutive inquirires.  Range: 0x0003 - 0xFFFF (*1.28s).
	                                   It is ignored if GAP_DEVICE_DISCOVERY_PERIODICALLY takes no effect */
	UINT16  min_period_len;         /* Minimum amout of time specified between consecutive inquirires.  Range: 0x0002 - 0xFFFE (*1.28s).
	                                   It is ignored if GAP_DEVICE_DISCOVERY_PERIODICALLY takes no effect */

    /* max_period_len > min_period_len > inquiry_len if BR/EDR Periodic Mode is required */
};

struct GAP_PairReqStru { /* Request parameter of GAP_PairDevice */
	struct HCI_AddressStru addr;	/* Addr info of the remote device */
};

struct GAP_RemoteDeviceInfoStru {
    UINT32            size;              /* Size of this structure, including the full eir buffer */
	struct HCI_AddressStru	addr;		 /* Address structure */
	UINT16            clock_offset;      /* Clock offset, shall be ignored if device_type is GAP_DEVICE_LE_ONLY */ 
	tHCI_Page_Scan_Repetition_Mode page_scan_rmode;   /* Page Scan Repetition Mode, shall be ignored if device_type is GAP_DEVICE_LE_ONLY */
	UINT32            class_of_device;   /* Class of Device, shall be ignored if device_type is GAP_DEVICE_LE_ONLY */
	UINT16            eir_size;          /* Size of the Extended Inquiry Response Data or LE Advertising Report Data */
	UINT8             rssi;              /* RSSI, Range -127 <= rssi <= 20; 127 means RSSI is not available */
	UINT8             eir[1];            /* Extended Inquiry Response Data or LE Advertising Report Data. The content may be different
	                                        from the one got from the device discovery procedure. The Host Stack may modify it, e.g. add
	                                        the complete name got from name discovery procedure, add a service list got from service
	                                        discovery procedure. The Host Stack will construct an EIR packet for a device not supporting EIR. */
};

struct GAP_AuthenticationFailureStru {
	struct HCI_AddressStru   addr; /* address of th remote device */
	UINT8   error;             /* Standard HCI error code reflects the failure reason */
};

struct GAP_PhysicalLinkStatusStru {
	UINT8      addr[BD_ADDR_LEN]; /* Bluetooth address of the device with which the physical link is created or disconnected */
	tGAP_Bool  status;            /* Physical Link Status; GAP_TRUE - Connected. GAP_FALSE - disconnected */
};

struct GAP_LinkKeyRequestIndStru {
    tGAP_Requested_Key_Type req_type;         /* Type of the Link Key being requested */
    tGAP_AMP_Type           amp_type;         /* AMP type of the Dedicated AMP Link Key. It is used only when the key_type is GAP_REQUEST_DAMP_LK. */
	tHCI_Peer_Address_Type  addr_type;        /* Type of the device address */
    UINT8                   addr[BD_ADDR_LEN];/* Address of the device which a stored link key is being requested. */ 
    tHCI_Key_Type           key_type;         /* Type of the BR/EDR Link Key returned. It is used only when the key_type is GAP_REQUEST_BREDR_LK. */
    UINT8                   key[16];          /* The link key returned. */ 
};

struct GAP_IOCapabilityRequstIndStru {
    tGAP_Bool            le_pair;          /* GAP_TRUE - IO capabilities for LE Pairing. GAP_FALSE - IO capabilities for BR/EDR Pairing. */
	tHCI_Peer_Address_Type    addr_type;        /* Type of the device address */
    UINT8                addr[BD_ADDR_LEN];/* Address of the device which a stored link key is being requested. */ 
    tGAP_IO_Capability   io_cap;           /* IO Capability of the application. */ 
    tGAP_Bool            oob_data_flag;    /* GAP_TRUE - OOB authentication data present. GAP_FALSE - OOB authentication data not present */ 
    union {
    tGAP_LE_AuthReq      le_auth_req;      /* LE Authentication requirements, used only when le_pair is GAP_TRUE */ 
    tGAP_BREDR_AuthReq   be_auth_req;      /* BR/EDR Authentication requirements, used only when le_pair is GAP_FALSE */
    };
    UINT8                le_max_encrypt_key_size;   /* Maximum encryption key size, used only when le_pair is GAP_TRUE */
    UINT8                le_init_key_distribution;  /* Initiator Key Distribution flags, e.g. GAP_LE_DISTRIBUTION_ENCKEY, used only when le_pair is GAP_TRUE */
    UINT8                le_resp_key_distribution;  /* Responder Key Distribution flags, e.g. GAP_LE_DISTRIBUTION_ENCKEY, used only when le_pair is GAP_TRUE */
};

typedef struct SMAuthorReqStru tGAP_AuthorizationReqStru;

struct GAP_ConnectionEventStru {
	struct HCI_AddressStru addr;		/* Address of the device to which the connection event related */
	UINT16                 acl_hdl;		/* The connection handle of the ACL link */
	tGAP_Bool              connected;	/* GAP_TRUE - Connected. GAP_FALSE - Disconnected */
    UINT8                  reason;      /* Reason for connection failure or disconnection, e.g. HCI_STATUS_PAGE_TIMEOUT */
    tGAP_ACLLinkType       link_type;	/* Type of the ACL link created. */
};

struct GAP_HCIEventPacketStru {
	UINT8       event_code;		    /* HCI event code, e.g. 0x0E - Command Complete Event. */
	UINT8       parameter_length;   /* Length of the parameter field, in octets */
	UINT8       parameter[1];       /* Event specific parameters, it is copied from HCI event packet received from controller */
};

struct GAP_CreateConnectionCfmStru {
	HANDLE                  tl_handle;  /* Application defined data structure used to identify the
                                           transport layer instance creating the connection. */
	struct HCI_AddressStru  addr;       /* Remote Address */
	UINT16                  acl_hdl;	/* The connection handle of the ACL link */
    tGAP_ACLLinkType        link_type;	/* Type of the ACL link created. */
};

/*
WiFI -----
	Possible channels: 2412 + k * 5Mhz(k = 0...12), etc, 2412...2472 and 2484
	Possible bandwidth: 22 and 40
*/
struct GAP_RadioChannelInfoStru
{
	UINT16  central_frequency;	/* Central frequency, in MHz, of this channel, e.g 2412(MHz). */
	UINT16  band_width;		    /* Bandwidth, in MHz, of this channel, e.g. 22(MHz), 40(MHz). */
};

/* 
Description:
    Confirmation to the asynchronous function call.
Parameters:
    context     [in] Application defined data structure used to identify the request.
    result      [in] The result of the requested procedure. It may be:
                    HCI_STATUS_OK - bonding is successful.
                    HCI_STATUS_HOST_NOT_READY - The Host Stack is not initialized.
                    HCI_STATUS_LOCAL_HOST_TIMEOUT - The supervision timer expires
                                    before receiving the expected complete event.
                    HCI_STATUS_CONTROLLER_UNAVAILABLE - The Controller is not ready
                                    to execute any command.
                    Other HCI error code returned by the Controller, e.g.
                    HCI_STATUS_AUTHENTICATION_FAILURE.
    cfm_par     [in] Aditional information of the confirmation. Refer to the following
                    specific confirmation callback definition. It is set to NULL if
                    not specified additionally.
*/
typedef void (GAP_ConfirmationCbk)(void *context, UINT16 result, void *cfm_par);

/* 
Description:
    Confirmation to the GAP_RegisterTransportLayerA.
Parameters:
    tl_handle   [in] Application defined data structure used to identify the
                transport layer instance registered.
    result      [in] The result of the registration, it may be:
                    HCI_STATUS_OK - The Host Stack setup a communication pipe with
                                    the Transport Layer driver successfully.
                    HCI_STATUS_HOST_NOT_READY - The Host Stack is not initialized.
                    HCI_STATUS_CONTROLLER_UNAVAILABLE - The Host Stack can't setup the
                                    communication pipe with the Transport Layer driver.
    cfm_par     [in] Aditional information of the confirmation. If the result is
                not HCI_STATUS_OK, the cfm_par shall be ignored.
*/
typedef void (GAP_RegisterTLCfmCbk)(HANDLE tl_handle, UINT16 result, struct GAP_RegisterTLCfmStru *cfm_par);

/* 
Description:
    Confirmation to the GAP_UnregisterTransportLayerA.
Parameters:
    tl_handle   [in] Application defined data structure used to identify the
                transport layer instance unregistered.
    result      [in] The result of the unregistration, it may be:
                    HCI_STATUS_OK - Success.
                    HCI_STATUS_HOST_NOT_READY - The Host Stack is not initialized.
    cfm_par     [in] Aditional information of the confirmation. It is reserved
                for future extension and shall be ignored.
*/
typedef void (GAP_UnregisterTLCfmCbk)(HANDLE tl_handle, UINT16 result, void *cfm_par);

/* 
Description:
    Confirmation to the GAP_ResetHardwareA.
Parameters:
    context     [in] Application defined data structure used to identify the request.
    result      [in] The result of the last command executed. It may be:
                    HCI_STATUS_OK - All the mandatory commands are executed
                                    successfully.
                    HCI_STATUS_HOST_NOT_READY - The Host Stack is not initialized.
                    HCI_STATUS_LOCAL_HOST_TIMEOUT - The supervision timer expires
                                    before receiving the expected complete event.
                    HCI_STATUS_CONTROLLER_UNAVAILABLE - The Controller is not ready
                                    to execute any command.
                    Other HCI error code returned by the Controller, e.g.
                    HCI_STATUS_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE.
    cfm_par     [in] Aditional information of the confirmation. It is reserved
                for future extension and shall be ignored.
*/
typedef void (GAP_ResetHardwareCfmCbk)(void *context, UINT16 result, void *cfm_par);

/* 
Description:
    Confirmation to the GAP_ExecuteCommandA.
Parameters:
    context     [in] Application defined data structure used to identify the request.
    result      [in] The result of the command execution. It may be:
                     HCI_STATUS_OK - The expected HCI complete event is received.
                                     The application shall check the possible 
                                     status member of the cfm_par->event_parameter
                                     for the actual result.
                     HCI_STATUS_UNKNOWN_HCI_COMMAND - The command is not supported
                                     by the Controller.
                     HCI_STATUS_LOCAL_HOST_TIMEOUT - The supervision timer expires
                                     before receiving the expected complete event.
                     HCI_STATUS_HOST_NOT_READY - The Host Stack is not initialized.
                     HCI_STATUS_CONTROLLER_UNAVAILABLE - The Controller is not ready
                                     to work.
    cfm_par     [in] Aditional information of the confirmation. If the result is
                not HCI_STATUS_OK, the content within cfm_par->event_parameter is
                unspecified and shall be ignored by the application.

*/
typedef void (GAP_ExecuteCommandCfmCbk)(void *context, UINT16 result, struct GAP_ExecuteCommandCfmStru *cfm_par);

/* 
Description:
    Confirmation to the GAP_DeviceDiscovery.
Parameters:
    context     [in] Application defined data structure used to identify the request.
    result      [in] The result of the command execution. It may be:
                     HCI_STATUS_OK - The device discovery procedure is finished
                                     successfully, or the Controller enters the
                                     Periodic Mode.
                     HCI_STATUS_LOCAL_HOST_TIMEOUT - The supervision timer expires
                                     before receiving the expected complete event,
                                     e.g. Inquiry Complet Event.
                     HCI_STATUS_CONTROLLER_UNAVAILABLE - The Controller is not ready
                                     to work.
                     HCI_STATUS_USER_TERMINATED - The device discovery procedure
                                     is canceled by the user.
                     Other HCI error code returned by the Controller, e.g.
                     HCI_STATUS_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE.
    cfm_par     [in] Aditional information of the confirmation. It is set to NULL
                if no devices reported during the Inquiry procedure.

*/
typedef void (GAP_DeviceDiscoveryCfmCbk)(void *context, UINT16 result, struct GAP_DeviceDiscoveryCfmStru *cfm_par);

/* 
Description:
    Confirmation to the GAP_VendorCommandA.
Parameters:
    context     [in] Application defined data structure used to identify the request.
    result      [in] The result of the command execution. It may be:
                     HCI_STATUS_OK - The device discovery procedure is finished
                                     successfully, or the Controller enters the
                                     Periodic Mode.
                     HCI_STATUS_LOCAL_HOST_TIMEOUT - The supervision timer expires
                                     before receiving the expected complete event,
                                     e.g. Inquiry Complet Event.
                     HCI_STATUS_CONTROLLER_UNAVAILABLE - The Controller is not ready
                                     to work.
                     HCI_STATUS_USER_TERMINATED - The device discovery procedure
                                     is canceled by the user.
                     Other HCI error code returned by the Controller, e.g.
                     HCI_STATUS_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE.
    cfm_par     [in] Aditional information of the confirmation. It is set to NULL
                if result is not HCI_STATUS_OK or no event is created due to this
                command.

*/
typedef void (GAP_VendorCommandCfmCbk)(void *context, UINT16 result, struct GAP_HCIEventPacketStru *cfm_par);

/* 
Description:
    Confirmation to the GAP_CreateConnectionA.
Parameters:
    context     [in] Application defined data structure used to identify the request.
    result      [in] The result of the command execution. It may be:
                     HCI_STATUS_OK - The device discovery procedure is finished
                                     successfully, or the Controller enters the
                                     Periodic Mode.
                     HCI_STATUS_LOCAL_HOST_TIMEOUT - The supervision timer expires
                                     before receiving the expected complete event,
                                     e.g. Inquiry Complet Event.
                     HCI_STATUS_CONTROLLER_UNAVAILABLE - The Controller is not ready
                                     to work.
                     HCI_STATUS_USER_TERMINATED - The device discovery procedure
                                     is canceled by the user.
                     Other HCI error code returned by the Controller, e.g.
                     HCI_STATUS_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE.
    cfm_par     [in] Aditional information of the confirmation.

*/
typedef void (GAP_CreateConnectionCfmCbk)(void *context, UINT16 result, struct GAP_CreateConnectionCfmStru *cfm_par);

/* 
Description:
    Callback function used to report an indication to the application.
Parameters:
    tl_handle  [in] Specifies the controller related to this indication, e.g. the
               Controller creates the HCI Connect Request Event.
    context    [in] Application defined data structure used to identify the callback.
    ind_type   [in/out] Specifies the indication type.
    parameter  [in] Additional indication parameter. Refer to the following specific
               indication callback definition.
    size       [in] Specifies the size, in bytes, of the parameter buffer.
Return:
    If not specified additionally, the return value is ignored by the Host Stack.
    Refer to the following specific indication callback definition.

    The Host Stack allows multiple callback functions exist to handle the same
    indication. The first registered one has the highest priority to determine
    the result, the last one has the lowest priority. The Host Stack executes
    these functions one by one as long as GAP_IND_RESULT_NOT_CARE is returned or
    the indication does not require a response.
*/
typedef tGAP_IndCbk_Result (GAP_IndicationCbk)(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type,
                                               void *parameter, UINT32 size);

/* 
Description:
    Callback function used to report an inquiry result to the application. It is
    called each time when the Host Stack receives one of the Inquiry_Result_Event,
    Inquiry_Result_With_RSSI_Event, Extended_Inquiry_Result_Event and
    LE_Advertising_Report_Event from one of the Controller.
Parameters:
    tl_handle  [in] Specifies the controller created the event.
    context    [in] Application defined data structure used to identify the callback.
    ind_type   [in] It shall be GAP_IND_INQUIRY_RESULT.
    parameter  [in] Information of one device reported by this event.
    size       [in] Specifies the size, in bytes, of the parameter buffer.
Return:
    Ignored and always set to GAP_IND_RESULT_NOT_CARE.
*/
typedef tGAP_IndCbk_Result (GAP_InquiryResultIndCbk)(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type,
                            struct GAP_RemoteDeviceInfoStru *parameter, UINT32 size);

/* 
Description:
    Callback function used to notify the application that the Inquiry procedure
    is complete. It is called each time when the Host Stack receives Inquiry_Complete_Event.
Parameters:
    tl_handle  [in] Specifies the controller exits the Inquiry mode.
    context    [in] Application defined data structure used to identify the callback.
    ind_type   [in] It shall be GAP_IND_INQUIRY_COMPLETE.
    parameter  [in] The status member indicates if the Inquiry completed successfully
               or if the Inquiry was not completed. It can be one of:
                     HCI_STATUS_OK - The Inquiry is completed successfully.
                     Other HCI error code returned by the Controller, e.g.
                     HCI_STATUS_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE.
    size       [in] Specifies the size, in bytes, of the parameter buffer.
Return:
    Ignored and always set to GAP_IND_RESULT_NOT_CARE.
*/
typedef tGAP_IndCbk_Result (GAP_InquiryCompleteIndCbk)(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type,
                            struct HCI_Inquiry_CompleteEvStru *parameter, UINT32 size);

/* 
Description:
    Callback function used to report the latest remote device name retrieved. It
    is called each time when the Host Stack receives Remote_Name_Request_Complete_Event.
Parameters:
    tl_handle  [in] Specifies the controller created the event.
    context    [in] Application defined data structure used to identify the callback.
    ind_type   [in] It shall be GAP_IND_REMOTE_DEVICE_NAME_UPDATED.
    parameter  [in] remote_name is coded in UTF-8. It may not be NULL terminated.
    size       [in] Specifies the size, in bytes, of the parameter buffer.
Return:
    Ignored and always set to GAP_IND_RESULT_NOT_CARE.
*/
typedef tGAP_IndCbk_Result (GAP_RemoteNameUpdateIndCbk)(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type,
                            struct HCI_Remote_Name_Request_CompleteEvStru *parameter, UINT32 size);

/* 
Description:
    Callback function used to request a link key from the application. It
    is called each time when the Host Stack requires a Link Key for BR/EDR
    authentication, AMP Create Physical Link or LE encryption.
Parameters:
    tl_handle  [in] Specifies the controller created the event.
    context    Application defined data structure used to identify the callback.
    ind_type   [in] It shall be GAP_IND_LINK_KEY_REQUEST.
    parameter  [in/out] On input, specifies the what kind of Link Key is requested.
               On output, returns the requested Link Key if available.
    size       [in] Specifies the size, in bytes, of the parameter buffer.
Return:
    It shall be one of:
    GAP_IND_RESULT_PENDING - The application shall reply to the request by calling
                           GAP_LinkKeyReply directly.
    GAP_IND_RESULT_ACCEPT - The application has the link key and copies it to the
                           parameter->key. The parameter->key_type shall also be
                           set to the right one if a BR/EDR link key is required.
    GAP_IND_RESULT_REJECT - The application forces to reject the Link Key Request.
    GAP_IND_RESULT_NOT_CARE - Determined by the Host Stack. If all callback functions
                          return GAP_IND_RESULT_NOT_CARE, the Host Stack rejects the
                          link Key request directly.

*/
typedef tGAP_IndCbk_Result (GAP_LinkKeyRequestIndCbk)(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type,
                            struct GAP_LinkKeyRequestIndStru *parameter, UINT32 size);


/* LE Data Structures */
struct GAP_AdvParametersStru {
    tHCI_Advertising_Type type;         /* Advertising type. */
    tGAP_LE_DiscoveryMode discoery_mode;/* Discovery Mode over LE physical channel. */
    tGAP_LE_AdvScanSpeed speed;         /* Determines the advertising timer parameters. */
    tHCI_Advertising_Filter_Policy adv_filter_policy; /* Advertising Filter Policy */
};

/* Create Connection request structure */
struct GAP_CreateConnectionReqStru {
	struct HCI_AddressStru addr;		/* address of the remote device */
	tGAP_ACLLinkType  link_type;		/* Type of the ACL link expected.*/
};


/********************************************************************************
 *																				*
 *								FUNCATION DECLARATION							*
 *																				*
 ********************************************************************************/
void GAP_Init(void);
void GAP_Done(void);

void GAP_RegisterIndCbk(void *context, tGAP_Ind_Type ind_type, GAP_IndicationCbk *ind_cbk);
void GAP_UnregisterIndCbk(void *context, GAP_IndicationCbk *ind_cbk);

#ifdef CONFIG_GAP_ASYNC_API_SUPPORTED
void GAP_RegisterTransportLayerA(HANDLE tl_handle, TransportLayerStru *func, GAP_RegisterTLCfmCbk *cfm_cbk);
void GAP_UnregisterTransportLayerA(HANDLE tl_handle, GAP_UnregisterTLCfmCbk *cfm_cbk);
void GAP_ResetHardwareA(HANDLE tl_handle, void *context, GAP_ResetHardwareCfmCbk *cfm_cbk);
void GAP_ExecuteCommandA(HANDLE tl_handle, void *context, UINT32 opindex, void *in_par, UINT32 in_par_size, GAP_ExecuteCommandCfmCbk *cfm_cbk);
void GAP_PairDeviceA(HANDLE tl_handle, void *context, struct GAP_PairReqStru *req, GAP_ConfirmationCbk *cfm_cbk);
void GAP_EncryptLinkA(HANDLE tl_handle, void *context, struct HCI_AddressStru *bd, GAP_ConfirmationCbk *cfm_cbk);
void GAP_VendorCommandA(HANDLE tl_handle, void *context, UINT8 evi, UINT16 ocf, void *in_par, UINT8 in_par_size, GAP_VendorCommandCfmCbk *cfm_cbk);
void GAP_CreateConnectionA(HANDLE tl_handle, void *context, struct GAP_CreateConnectionReqStru *conn_req, GAP_CreateConnectionCfmCbk *cfm_cbk);
void GAP_SetLocalNameA(HANDLE tl_handle, void *context, UINT8 *name, UINT16 len, GAP_ConfirmationCbk *cfm_cbk);
void GAP_SetVisualModeA(HANDLE tl_handle, void *context, UINT16 mode, GAP_ConfirmationCbk *cfm_cbk);
void GAP_SetLocalDeviceClassA(HANDLE tl_handle, void *context, UINT8 cod[CLS_DEV_LEN], GAP_ConfirmationCbk *cfm_cbk);
#endif

#ifdef CONFIG_GAP_SYNC_API_SUPPORTED
UINT16 GAP_RegisterTransportLayer(HANDLE tl_handle, TransportLayerStru *func, struct GAP_RegisterTLCfmStru *out_par);
UINT16 GAP_UnregisterTransportLayer(HANDLE tl_handle);
UINT16 GAP_ResetHardware(HANDLE tl_handle);
UINT16 GAP_ExecuteCommand(HANDLE tl_handle, UINT32 opindex, void *in_par, UINT32 in_par_size, void *out_par);
UINT16 GAP_PairDevice(HANDLE tl_handle, struct GAP_PairReqStru *req);
UINT16 GAP_EncryptLink(HANDLE tl_handle, struct HCI_AddressStru *bd);
UINT16 GAP_VendorCommand(HANDLE tl_handle, UINT8 evi, UINT16 ocf, void *in_par, UINT8 in_par_size, void *out_par);
UINT16 GAP_CreateConnection(HANDLE tl_handle, struct GAP_CreateConnectionReqStru *conn_req);
UINT16 GAP_SetLocalName(HANDLE tl_handle, UINT8 *name, UINT16 len);
UINT16 GAP_GetLocalName(HANDLE tl_handle, UINT8 *name, UINT16 *len);
UINT16 GAP_SetVisualMode(HANDLE tl_handle, UINT16 mode);
UINT16 GAP_GetLocalDeviceAddress(HANDLE tl_handle, UINT8 *bd_addr);
UINT16 GAP_SetLocalDeviceClass(HANDLE tl_handle, UINT8 cod[CLS_DEV_LEN]);
UINT16 GAP_GetLocalDeviceClass(HANDLE tl_handle, UINT8 cod[CLS_DEV_LEN]);
#endif

UINT16 GAP_Disconnect(HANDLE tl_handle, UINT16 acl_hdl);
UINT16 GAP_SetFixedPinCode(HANDLE tl_handle, UINT8* pin_code, UINT8 size);
UINT16 GAP_GetFixedPinCode(HANDLE tl_handle, UINT8* pin_code, UINT8 *size);
UINT16 GAP_GetConnHandle(HANDLE tl_handle, struct HCI_AddressStru *bd, UINT16 *conn_hdl);
UINT16 GAP_GetLinkMode(HANDLE tl_handle, struct HCI_AddressStru *bd, tHCI_Current_Mode *mode);
UINT16 GAP_DeviceDiscovery(HANDLE tl_handle, void *context, struct GAP_DeviceDiscoveryReqStru *req, GAP_DeviceDiscoveryCfmCbk *cfm_cbk);
UINT16 GAP_CancelDeviceDiscovery(HANDLE tl_handle, void *context);
UINT8  GAP_LinkKeyReply(HANDLE tl_handle, struct HCI_Security_Link_Key_Request_ReplyStru *par);
UINT16 GAP_SetIOCapability(UINT8 io_cap);
UINT16 GAP_SetChannelClassification(HANDLE tl_handle, struct GAP_RadioChannelInfoStru *channels, UINT16 number);

tGAP_Bool GAP_IsLinkEncrypted(HANDLE tl_handle, struct HCI_AddressStru *bd);
#ifdef CONFIG_BLE
UINT16 GAP_LESetAdvSetParam(UINT8* param);
UINT16 GAP_LESetAdvSetEnable(UINT8 enable, UINT16 timer);
UINT16 GAP_LESetAdvData(UINT8 len, UINT8* data);
UINT16 GAP_LESetScanResponseData(UINT16 len, UINT8* data);
UINT16 GAP_LEConnUpdateParam(struct L2CAP_ConnectionParameterUpdateStru *data);
#endif
#endif
