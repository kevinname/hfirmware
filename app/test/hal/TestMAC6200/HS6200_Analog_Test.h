#ifndef HS6200_ANALOG_TEST_H
#define HS6200_ANALOG_TEST_H


#define TEST_EN_ENABLE       0x01
#define TEST_EN_DISABLE      0x00  
extern U8 Test_En_Flag;
extern U8 TEST_PKDET_Reg_Val[4];

#define TEST_PAT_PSDO_RDM_NUM_FLG_START  0x01
#define TEST_PAT_PSDO_RDM_NUM_FLG_STOP   0x00
extern U8 Test_Pattern_Psdo_Rdm_Num_Flag[2];

//------------------------------------Analog Test ----------------------------------
extern U8 HS6200_Activate(U8 DevNum);
extern U8 HS6200_Bank0_Activate(U8 DevNum);
extern U8 HS6200_Bank1_Activate(U8 DevNum);

//------------------1.Test Point Configuration-------------------------
extern void HS6200_Test_Disable(U8 DevNum);
extern void HS6200_Test_Point_Config_Lna_Bias(U8 DevNum);
extern void HS6200_Test_Point_Config_Mix_CM_Vol(U8 DevNum);
extern void HS6200_Test_Point_Config_TIA_I_CM_Vol(U8 DevNum);
extern void HS6200_Test_Point_Config_TIA_Q_CM_Vol(U8 DevNum);
extern void HS6200_Test_Point_Config_TIA_Output(U8 DevNum);
extern void HS6200_Test_Point_Config_1V2_LDO(U8 DevNum);
extern void HS6200_Test_Point_Config_1V3_LDO(U8 DevNum);
extern void HS6200_Test_Point_Config_Div2_CM_Vol(U8 DevNum);
extern void HS6200_Test_Point_Config_LO_Ctrl_Vol(U8 DevNum);
extern void HS6200_Test_Point_Config_Fbclk_LO(U8 DevNum);
extern void HS6200_Test_Point_Config_Crystal_Output(U8 DevNum);
extern void HS6200_Test_Point_Config_Filter_Output(U8 DevNum);

//HS6200B1新增 wangs 2013-12-25
extern void HS6200B1_Test_Point_Config_Fbclk_b(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Crystal_clock(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Old_pkdet_ip(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Old_pkdet_qp(U8 DevNum);
extern void HS6200B1_Test_Point_Config_New_pkdet_1(U8 DevNum);
extern void HS6200B1_Test_Point_Config_New_pkdet_0(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Fiter_ip(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Fiter_in(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Fiter_qp(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Fiter_qn(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Mixer_ip(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Mixer_in(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Mixer_qp(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Mixer_qn(U8 DevNum);
extern void HS6200B1_Test_Point_Config_VCO_LDO(U8 DevNum);
extern void HS6200B1_Test_Point_Config_LO_BUF_LDO(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Bnadgap(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Vctrl(U8 DevNum);
extern void HS6200B1_Test_Point_Config_LNA_bias_B1(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Mixer_bias(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Mixer_LDO(U8 DevNum);
extern void HS6200B1_Test_Point_Config_LNA_LDO(U8 DevNum);
extern void HS6200B1_Test_Point_Config_Filter_LDO(U8 DevNum);
extern void HS6200B1_Test_Point_Config_XTAL_BUF_LDO(U8 DevNum);
//END wangs 2013-12-25


//-----------------2.Frequency Configuration---------------------------
extern void HS6200_Set_Freq(U8 DevNum,U8*Freq,U8 Freq_Length);
extern void HS6200_Freq_Cfg_SDM_On(U8 DevNum);
extern void HS6200_Freq_Cfg_SDM_Off(U8 DevNum);
extern void HS6200_Freq_Cfg_SDM_Dither_On(U8 DevNum);
extern void HS6200_Freq_Cfg_SDM_Dither_Off(U8 DevNum);
extern void HS6200_Freq_Cfg_SDM_Dither_Stage(U8 DevNum,U8 Stage);
extern void HS6200_Freq_Cfg_Freq_Offset_160KHz(U8 DevNum);
extern void HS6200_Freq_Cfg_Freq_Offset_250KHz(U8 DevNum);
extern void HS6200_Freq_Cfg_Freq_Offset_320KHz(U8 DevNum);
extern void HS6200_Data_Rate_250K(U8 DevNum);
extern void HS6200_Data_Rate_500K(U8 DevNum);
extern void HS6200_Data_Rate_1M(U8 DevNum);
extern void HS6200_Data_Rate_2M(U8 DevNum);
extern void HS6200_Data_Rate(U8 DevNum,U8 Data_Rate);
extern void HS6200_IF_Freq(U8 DevNum,U8* Freq,U8 Freq_Length);	
extern void HS6200_AB_Cnt(U8 DevNum,U8 A_Cnt,U8 B_Cnt);
extern void HS6200_PLL_Test_En_On(U8 DevNum);
extern void HS6200_PLL_Test_En_Off(U8 DevNum);	


//-------------------3.Rx Gain Configration----------------------------
extern void HS6200_Agc_Gain_Mn_On(U8 DevNum);
extern void HS6200_Agc_Gain_Mn_Off(U8 DevNum);
extern void HS6200_LNA_Gain_4dB(U8 DevNum);
extern void HS6200_LNA_Gain_16dB(U8 DevNum);
extern void HS6200_LNA_Gain_28dB(U8 DevNum);
extern void HS6200_LNA_Gain_40dB(U8 DevNum);

//HS6200B1新增  wangs 2013-12-25
extern void HS6200B1_LNA_Gain_4dB(U8 DevNum);
extern void HS6200B1_LNA_Gain_16dB(U8 DevNum);
extern void HS6200B1_LNA_Gain_28dB(U8 DevNum);
extern void HS6200B1_LNA_Gain_40dB(U8 DevNum);
//END  wangs 2013-12-25

extern void  HS6200_Gain_Filter_6dB(U8 DevNum);
extern void  HS6200_Gain_Filter_8dB(U8 DevNum);
extern void  HS6200_Gain_Filter_10dB(U8 DevNum);
extern void  HS6200_Gain_Filter_12dB(U8 DevNum);
extern void  HS6200_Gain_Filter_14dB(U8 DevNum);
extern void  HS6200_Gain_Filter_16dB(U8 DevNum);
extern void  HS6200_Gain_Filter_18dB(U8 DevNum);
extern void  HS6200_Gain_Filter_20dB(U8 DevNum);
extern void  HS6200_Gain_Filter_22dB(U8 DevNum);
extern void  HS6200_Gain_Filter_24dB(U8 DevNum);
extern void  HS6200_Gain_Filter_26dB(U8 DevNum);
extern void  HS6200_Gain_Filter_28dB(U8 DevNum);
extern void  HS6200_Gain_Filter_30dB(U8 DevNum);
extern void  HS6200_Gain_Filter_32dB(U8 DevNum);
extern void  HS6200_Gain_Filter_34dB(U8 DevNum);
extern void  HS6200_Gain_Filter_36dB(U8 DevNum);
extern void  HS6200_Gain_Filter_38dB(U8 DevNum);
extern void  HS6200_Gain_Filter_40dB(U8 DevNum);
extern void  HS6200_Gain_Filter_42dB(U8 DevNum);
extern void  HS6200_Gain_Filter_44dB(U8 DevNum);
extern void  HS6200_Gain_Filter_46dB(U8 DevNum);

//------------------------4.peak detector threshold----------------------
extern void HS6200_Peak_Detector_Power_50mV(U8 DevNum);
extern void HS6200_Peak_Detector_Power_100mV(U8 DevNum);
extern void HS6200_Peak_Detector_Power_150mV(U8 DevNum);
extern void HS6200_Peak_Detector_Power_200mV(U8 DevNum);
extern void HS6200_Peak_Detector_Power_250mV(U8 DevNum);
extern void HS6200_Peak_Detector_Power_300mV(U8 DevNum);
extern void HS6200_Peak_Detector_Power_350mV(U8 DevNum);
extern void HS6200_Peak_Detector_Power_400mV(U8 DevNum);

//HS6200B1 New peak detector (VH VL)   wangs 2013-12-26
extern void HS6200B1_Old_Peak_Detector_Activate(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Activate(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_VH_VL_Configuration(U8 DevNum, U8 Value_VH_VL);
extern void HS6200B1_New_Peak_Detector_Power_VH0(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Power_VH1(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Power_VH2(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Power_VH3(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Power_VH4(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Power_VL0(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Power_VL1(U8 DevNum);
extern void HS6200B1_New_Peak_Detector_Power_VL2(U8 DevNum);
//END wangs 2013-12-26


//-------------------------5.PA power config-----------------------------
extern U8 HS6200_Read_PA_Power(U8 DevNum);
extern void HS6200_PA_Power_n18dBm(U8 DevNum);
extern void HS6200_PA_Power_n12dBm(U8 DevNum);
extern void HS6200_PA_Power_n6dBm(U8 DevNum);
extern void HS6200_PA_Power_0dBm(U8 DevNum);
extern void HS6200_PA_Power_5dBm(U8 DevNum);
extern  void HS6200_PA_power(U8 DevNum,U8 PA_Power);


//HS6200B1  wangs 2013-12-27
extern void HS6200B1_PA_Power_Voltage_1V8(U8 DevNum);
extern void HS6200B1_PA_Power_Voltage_3V(U8 DevNum);
extern void HS6200B1_PA_Power_n18dBm(U8 DevNum);
extern void HS6200B1_PA_Power_n12dBm(U8 DevNum);
extern void HS6200B1_PA_Power_n6dBm(U8 DevNum);
extern void HS6200B1_PA_Power_0dBm(U8 DevNum);
extern void HS6200B1_PA_Power_5dBm(U8 DevNum);
//END wangs 2013-12-27


//----------------------------6.calibration manual mode-------------------
//VCO and frequency calibration
extern void HS6200_VCO_Ctuning_Calb_Mn_On(U8 DevNum);
extern void HS6200_VCO_Ctuning_Calb_Mn_Off(U8 DevNum);
extern U8 HS6200_Read_VCO_Ctuning(U8 DevNum);
extern void HS6200_Write_VCO_Ctuning(U8 DevNum,U8 Ctuning_Num);

extern void HS6200_VCO_Ftuning_Calb_Mn_On(U8 DevNum);
extern void HS6200_VCO_Ftuning_Calb_Mn_Off(U8 DevNum);
extern U8 HS6200_Read_VCO_Ftuning(U8 DevNum);
extern void HS6200_Write_VCO_Ftuning(U8 DevNum,U8 Ftuning_Num);
//VCO LDO calibration
extern void HS6200_VCO_LDO_Calb_Mn_On(U8 DevNum);
extern void HS6200_VCO_LDO_Calb_Mn_Off(U8 DevNum);
extern U8 HS6200_Read_VCO_LDO_Calb(U8 DevNum);
extern void HS6200_Write_VCO_LDO_Calb(U8 DevNum,U8 Calb_Value);
//DAC gain calibration
extern void HS6200_DAC_Range_Mn_On(U8 DevNum);
extern void HS6200_DAC_Range_Mn_Off(U8 DevNum);
extern U8 HS6200_Read_DAC_Gain(U8 DevNum);
extern void HS6200_Write_DAC_Gain(U8 DevNum,U8 Gain);
//DC　offset calibration
extern void HS6200_DOC_DAC_Mn_On(U8 DevNum);
extern void HS6200_DOC_DAC_Mn_Off(U8 DevNum);
extern U8 HS6200_Read_DC_Offset_I(U8 DevNum);	
extern void HS6200_Write_DC_Offset_I(U8 DevNum,U8 DC_Ofst_I);
extern U8 HS6200_Read_DC_Offset_Q(U8 DevNum);
extern void HS6200_Write_DC_Offset_Q(U8 DevNum,U8 DC_Ofst_Q);

extern void HS6200_Calibration(U8 DevNum);
extern void HS6200_Carrier_Calb(U8 DevNum,U8 Carrier);

//--------------------------7.continuous carrier transmit-------------------
extern void HS6200_Cont_Wave_Start(U8 DevNum);
extern void HS6200_Cont_Wave_Stop(U8 DevNum);
extern void HS6200_PLL_DAC_On(U8 DevNum);
extern void HS6200_PLL_DAC_Off(U8 DevNum);
extern void HS6200_Bp_Cp_Diox_On(U8 DevNum);
extern void HS6200_Bp_Cp_Diox_Off(U8 DevNum);
extern void HS6200_Vc_Det_On(U8 DevNum);
extern void HS6200_Vc_Det_Off(U8 DevNum);

extern void HS6200_PLL_Icp_Sel_80uA(U8 DevNum);
extern void HS6200_PLL_Icp_Sel_120uA(U8 DevNum);
extern void HS6200_PLL_Icp_Sel_160uA(U8 DevNum);
extern void HS6200_PLL_Icp_Sel_200uA(U8 DevNum);
extern void HS6200_PLL_Vdiv2_Sel_550mV(U8 DevNum);
extern void HS6200_PLL_Vdiv2_Sel_600mV(U8 DevNum);
extern void HS6200_PLL_Vdiv2_Sel_650mV(U8 DevNum);
extern void HS6200_PLL_Vdiv2_Sel_700mV(U8 DevNum);


//-----------------------8.continuous test pattern transmit----------------------
extern void HS6200_Cont_Test_Pat_Tx_Start(U8 DevNum);
extern void HS6200_Cont_Test_Pat_Tx_Stop(U8 DevNum); 
extern void HS6200_Cont_Test_Pat(U8 DevNum,U8 Test_Pat);
extern void HS6200_Test_Pattern_Pseudo_Random_Num(U8 DevNum);

extern void HS6200_Soft_Rst(U8 DevNum);


#endif  /*HS6200_ANALOG_TEST_H*/


 /*-------------------------------End Of File--------------------------------*/