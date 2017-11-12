/*
 * HS6200 Analog test, 参考《HS6200_Software_Request_of_Analog_v0.2.docx》文件 
 *
 * Yao  07/05/2013
 * HunterSun Electronics Co., Ltd.
 */

#include "nRF24L01_X.h"
#include "HS6200_test.h"
#include "HS6200_test_sys.h"

#include "HS6200_Reg.h"
#include "HS6200Test_Application_Protocol.h"
#include "stdlib.h"
#include "HS6200_Debug.h"
#include "HS6200_Analog_Test.h"
#include "C8051F_USB.h"

U8 Test_Pattern_Psdo_Rdm_Num_Flag[2]={
          TEST_PAT_PSDO_RDM_NUM_FLG_STOP,
          TEST_PAT_PSDO_RDM_NUM_FLG_STOP
	};
//TEST_PKDET中的TEST_EN标志位，用于判断TEST_EN位是否置1，TEST_EN置 1，HS6200的IRQ,MISO引脚会被复用.
U8 Test_En_Flag=TEST_EN_DISABLE;     
U8 TEST_PKDET_Reg_Val[4]={0x2A,0x10,0x00,0x21};   //HS6200 BANK1的TEST_PKDET寄存器的复位值 
U8 IE_Restor;	            //在Test point configuration下 IE的状态.

U8 HS6200_Activate(U8 DevNum)
{
  U8 status;
  nRF24L01_X_csn_low_disable_int(DevNum);                       // CSN low, init SPI transaction
  status = nRF24L01_X_spi_rw(DevNum, nRF24L01_ACTIVATE);    // send command
  nRF24L01_X_spi_rw(DevNum, 0x53);
  nRF24L01_X_csn_high_restore_int(DevNum);                      // CSN high again	
  return status;
}

U8 HS6200_Bank1_Activate(U8 DevNum)	//切换到BANK1 状态下
{
    U8 status;
	status=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_STATUS);
	if( Dev_Flag[DevNum]==HS6200_DEV )  //HS6200 切换bank	    
	{
		if( (status & STATUS_BANK1)==STATUS_BANK0 )	   //in Bank0 status
		{
    		nRF24L01_X_csn_low_disable_int(DevNum);                       // CSN low, init SPI transaction
        	status = nRF24L01_X_spi_rw(DevNum, nRF24L01_ACTIVATE);    // send command
        	nRF24L01_X_spi_rw(DevNum, 0x53);
    		nRF24L01_X_csn_high_restore_int(DevNum);                      // CSN high again
		}
	} 
    return (status);                                              // return nRF24L01 status byte
}			    
U8 HS6200_Bank0_Activate(U8 DevNum)	 //切换到BANK0 状态下 
{
    U8 status;
	status=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_STATUS);
	if( Dev_Flag[DevNum]==HS6200_DEV )     //HS6200 切换bank
	{
		if( (status & STATUS_BANK1)==STATUS_BANK1 )	   //in Bank1 status
		{
	   		nRF24L01_X_csn_low_disable_int(DevNum);                       // CSN low, init SPI transaction
	        status = nRF24L01_X_spi_rw(DevNum, nRF24L01_ACTIVATE);    // send command
	        nRF24L01_X_spi_rw(DevNum, 0x53);
	    	nRF24L01_X_csn_high_restore_int(DevNum);                      // CSN high again
		}
	}
    return (status);                                              // return status byte
} 

/*-----------------------------------------------Analog Test------------------------------------------------------*/

//----------------------------1.Test Point Configuration------------------------------
//HS6200的BANK1中的TEST_PKDET寄存器中的TEST_EN位置 1 时，此时 HS6200 处于测试模式，
//芯片的MISO和IRQ pin处于其他状态，此时不能对芯片进行读操作，可以进行写操作.
void HS6200_Test_Disable(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor= IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);	 //bank1	
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[3] &=~HS6200_BANK1_TEST_PACKET_TEST_EN;	
	Test_En_Flag=TEST_EN_DISABLE; 
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val,4);
	//IE=IE_Restor;
}
/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010          000                  00
 */
void HS6200_Test_Point_Config_Lna_Bias(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;	
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);	
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |= HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00;
	TEST_PKDET_Reg_Val[3] |= HS6200_BANK1_TEST_PACKET_TEST_EN;		                        //TEST_EN=1
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val, 4);	
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010             000                  11
 */
void HS6200_Test_Point_Config_Mix_CM_Vol(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |= HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_11;
	TEST_PKDET_Reg_Val[3] |= HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);	
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010             000                  01
 */
void HS6200_Test_Point_Config_TIA_I_CM_Vol(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |= HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01;
	TEST_PKDET_Reg_Val[3] |= HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}
/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010            000                  10
 */
void HS6200_Test_Point_Config_TIA_Q_CM_Vol(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;	
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |= HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_10;
	TEST_PKDET_Reg_Val[3] |= HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   100            011                   xx
 */
void HS6200_Test_Point_Config_TIA_Output(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |= (HS6200_BANK1_TEST_PACKET_TEST_MODE_100+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_xx);
	TEST_PKDET_Reg_Val[3] |= HS6200_BANK1_TEST_PACKET_TEST_EN;	
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010             001                  01
 */
void HS6200_Test_Point_Config_1V2_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4); 
	} 
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010             001                  10
 */
void HS6200_Test_Point_Config_1V3_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_10);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010            001                  00
 */
void HS6200_Test_Point_Config_Div2_CM_Vol(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);	
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00);
   	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   010            001                  11
 */
void HS6200_Test_Point_Config_LO_Ctrl_Vol(U8 DevNum)
{
 	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}	
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200_BANK1_TEST_PACKET_TEST_MODE_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_11);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *    001            000                  0x
 */
void HS6200_Test_Point_Config_Fbclk_LO(U8 DevNum)
{
  	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200_BANK1_TEST_PACKET_TEST_MODE_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_0x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   001            000                  1x
 */
void HS6200_Test_Point_Config_Crystal_Output(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}	
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200_BANK1_TEST_PACKET_TEST_MODE_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_1x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

/* Test_Mode   Test_Point_Sel_1    Test_Point_Sel_0
 *   100             000                  xx
 */
void HS6200_Test_Point_Config_Filter_Output(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200_BANK1_TEST_PACKET_TEST_MODE_100+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_xx);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}


/*HS6200B1新增模拟测试内容  wangs 2013-12-25*/
void HS6200B1_Test_Point_Config_Fbclk_b(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_01+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_0x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Crystal_clock(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_01+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_1x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Old_pkdet_ip(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_01+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_0x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Old_pkdet_qp(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_01+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_1x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_New_pkdet_1(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_01+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_0x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_New_pkdet_0(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_01+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_1x);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Fiter_ip(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Fiter_in(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Fiter_qp(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_10);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Fiter_qn(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_000+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_11);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Mixer_ip(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Mixer_in(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Mixer_qp(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_10);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Mixer_qn(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_001+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_11);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_VCO_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_LO_BUF_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Bnadgap(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_10);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Vctrl(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_010+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_11);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_LNA_bias_B1(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_011+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Mixer_bias(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_011+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Mixer_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_011+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_10);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_LNA_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_011+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_11);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_Filter_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_100+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_00);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}

void HS6200B1_Test_Point_Config_XTAL_BUF_LDO(U8 DevNum)
{
	if(TEST_EN_DISABLE==Test_En_Flag)
	{
//		IE_Restor=IE;
//		if(DevNum==DEV_0)nRF24L01_0_EI=0;
//		else nRF24L01_1_EI=0;
		HS6200_Bank1_Activate(DevNum);
		nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,TEST_PKDET_Reg_Val,4);
	}
	TEST_PKDET_Reg_Val[2] &=~(HS6200B1_BANK1_TEST_PACKET_TEST_MODE+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0);
	TEST_PKDET_Reg_Val[2] |=(HS6200B1_BANK1_TEST_PACKET_TEST_MODE_10+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL1_100+HS6200_BANK1_TEST_PACKET_TEST_POINT_SEL0_01);
	TEST_PKDET_Reg_Val[3] |=HS6200_BANK1_TEST_PACKET_TEST_EN;
	Test_En_Flag=TEST_EN_ENABLE;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, TEST_PKDET_Reg_Val, 4);
}



/*END wangs 2013-12-25*/


//--------------------------------2.frequency configuration-------------------------------
void HS6200_Set_Freq(U8 DevNum,U8*Freq,U8 Freq_Length)
{
	U8 pTemp[5];
	U8 Reg_Val[4];
	U8 i;
	for(i=0x00;i<Freq_Length;i++)pTemp[i]=Freq[i];
	HS6200_Bank1_Activate(DevNum);
    nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CHAN,Reg_Val,4);
	/*
	 *	     Freq[0]        Freq[1]           Freq[2]       Freq[3]   Freq[4]
	 * 	integer MSByte	 integer LSByte	   fraction MSByte   fraction LSByte
	 *                      
	 *  CHAN Register  
	 *	CHAN[8:0] interger 
	 *  CHAN[29:9] fraction
	 *  CHAN[31:30] reserved  
	 */
	Reg_Val[0]=pTemp[1];		   //integer[7:0]
	Reg_Val[1]=((pTemp[4]& 0x7F)<<1) + ( pTemp[0] & 0x01);       //frac[7:0]+int[8] 
	Reg_Val[2]=((pTemp[3]& 0x3F)<<1) + ( (pTemp[4] & 0x80 )>>7); //frac[15:8]
	Reg_Val[3]=((pTemp[2]& 0x1F)<<1) + ( (pTemp[3] & 0x80)>>7);	 //frac[21:16]
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CHAN,Reg_Val,4);	
}


void HS6200_Freq_Cfg_SDM_On(U8 DevNum)
{
   	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
    Temp[3]|=HS6200_BANK1_SDM_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);
}
void HS6200_Freq_Cfg_SDM_Off(U8 DevNum)
{
   	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
    Temp[3]&=~HS6200_BANK1_SDM_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}
void HS6200_Freq_Cfg_SDM_Dither_On(U8 DevNum)
{
   	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
    Temp[0] |=HS6200_BANK1_SDM_DITH_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}
void HS6200_Freq_Cfg_SDM_Dither_Off(U8 DevNum)
{
   	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
    Temp[0] &=~HS6200_BANK1_SDM_DITH_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}

void HS6200_Freq_Cfg_SDM_Dither_Stage(U8 DevNum,U8 Stage)
{
   	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[0] &=~HS6200_BANK1_SDM_DITH_IN;
	if(Stage==0x01)	Temp[0] |=HS6200_BANK1_SDM_DITH_STAGE_1;
	else Temp[0] |=HS6200_BANK1_SDM_DITH_STAGE_2;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}

void HS6200_Freq_Cfg_Freq_Offset_160KHz(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FDEV,HS6200_BANK1_FDEV_160KHZ);	//FDEV
	
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);	   //PLL_FOFFSET_SEL
	Temp[3] &=~HS6200_BANK1_PLL_FOFST_SEL;
	Temp[3] |=HS6200_BANK1_PLL_FOFST_SEL_160KHZ;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);
}
void HS6200_Freq_Cfg_Freq_Offset_250KHz(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FDEV,HS6200_BANK1_FDEV_250KHZ);	//FDEV
	
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);	   //PLL_FOFFSET_SEL
	Temp[3] &=~HS6200_BANK1_PLL_FOFST_SEL;
	Temp[3] |=HS6200_BANK1_PLL_FOFST_SEL_250KHZ;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);
}
void HS6200_Freq_Cfg_Freq_Offset_320KHz(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FDEV,HS6200_BANK1_FDEV_320KHZ);	//FDEV
	
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);	   //PLL_FOFFSET_SEL
	Temp[3] &=~HS6200_BANK1_PLL_FOFST_SEL;
	Temp[3] |=HS6200_BANK1_PLL_FOFST_SEL_320KHZ;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);
}
//数据传输速率设置
void HS6200_Data_Rate_250K(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_SETUP);	
	Temp &=~HS6200_BANK0_DR;
	Temp |=HS6200_BANK0_DR_250K;
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_Data_Rate_500K(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_SETUP);	
	Temp &=~HS6200_BANK0_DR;
	Temp |=HS6200_BANK0_DR_500K;
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_Data_Rate_1M(U8 DevNum)
{
 	U8 Temp;
	HS6200_Bank0_Activate(DevNum);
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_SETUP);	
	Temp &=~HS6200_BANK0_DR;
	Temp |=HS6200_BANK0_DR_1M;
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_Data_Rate_2M(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_SETUP);	
	Temp &=~HS6200_BANK0_DR;
	Temp |=HS6200_BANK0_DR_2M;
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_Data_Rate(U8 DevNum,U8 Data_Rate)
{
	switch(Data_Rate)
	{
		case 0x00:
			HS6200_Data_Rate_250K(DevNum);   //HS6200 无250K Data Rate.
		break;
		case 0x01:
			HS6200_Data_Rate_500K(DevNum);    //nRF24L01 无 500K Data Rate.
		break;
		case 0x02:
			HS6200_Data_Rate_1M(DevNum);
		break;
		case 0x03:
			HS6200_Data_Rate_2M(DevNum);
		break;
		default:
		break;
	}	
}


//设置中频
void HS6200_IF_Freq(U8 DevNum,U8* Freq,U8 Freq_Length)	 
{
	U8 Temp[3];
	/*
	 * Freq[0]  Freq[1]  Freq[2]
	 * MSByte 	 ...     LSByte
	 */
	if(Freq_Length==0x03)   //3Bytes
	{
		Temp[0]=Freq[2];
		Temp[1]=Freq[1];
		Temp[2]=Freq[0];
	}
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_IF_FREQ, Temp, 3);	
}
//AB Cnt write enable
void HS6200_AB_Cnt(U8 DevNum,U8 A_Cnt,U8 B_Cnt)
{
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_A_CNT_REG,A_Cnt);
 	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_B_CNT_REG,B_Cnt);

}
void HS6200_PLL_Test_En_On(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[1]|=HS6200_BANK1_PLL_TEST_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);
}
void HS6200_PLL_Test_En_Off(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[1]&=~HS6200_BANK1_PLL_TEST_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);
}

//--------------------------------3. Rx gain configuation----------------------------
void HS6200_Agc_Gain_Mn_On(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[3]|=HS6200_BANK1_RX_GAIN_CONFIG_AGC_GAIN_MN;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN,Temp,4);	
}
void HS6200_Agc_Gain_Mn_Off(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[3]&=~HS6200_BANK1_RX_GAIN_CONFIG_AGC_GAIN_MN;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN,Temp,4);
}

void HS6200_LNA_Gain_4dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_4dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);		
}
void HS6200_LNA_Gain_16dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_16dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);		
}

void HS6200_LNA_Gain_28dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_28dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);		
}
void HS6200_LNA_Gain_40dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200_BANK1_RX_GAIN_CONFIG_LNA_GAIN_40dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp,4);		
}

/*HS6200B1 更改模拟测试内容2.RX gain configuration   wangs 2013-12-25*/
void HS6200B1_LNA_Gain_4dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_4dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);		
}
void HS6200B1_LNA_Gain_16dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_16dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);		
}

void HS6200B1_LNA_Gain_28dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_28dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);		
}
void HS6200B1_LNA_Gain_40dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN;
	Temp[1] |= HS6200B1_BANK1_RX_GAIN_CONFIG_LNA_GAIN_40dB;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp,4);		
}

/*END wangs 2013-12-25*/

void HS6200_Gain_Filter_6dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT6;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_8dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT6;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_10dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT6;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_12dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT5;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}

void HS6200_Gain_Filter_14dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
    Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT5;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_16dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT5;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_18dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT4;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}		
void HS6200_Gain_Filter_20dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT4;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}		
void HS6200_Gain_Filter_22dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT4;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_24dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT3;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_26dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT3;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_28dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT3;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}			
void HS6200_Gain_Filter_30dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT2;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}		
void HS6200_Gain_Filter_32dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT2;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}	
void HS6200_Gain_Filter_34dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT2;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_36dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT1;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}	
void HS6200_Gain_Filter_38dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT1;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_40dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT1;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}		
void HS6200_Gain_Filter_42dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[3]|=HS6200_BANK1_RX_GAIN_CONFIG_AGC_GAIN_MN;
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT0;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_100;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}
void HS6200_Gain_Filter_44dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT0;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_010;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}			
void HS6200_Gain_Filter_46dB(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_GAIN,Temp,4);
	Temp[1] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2;
	Temp[0] &=~HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1;
	Temp[0]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_1_BIT0;
	Temp[1]|=HS6200_BANK1_RX_GAIN_CONFIG_GAIN_FILTER_2_001;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_GAIN, Temp, 4);
}		
//---------------------------4.peak detector power test---------------------------
void HS6200_Peak_Detector_Power_50mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT7;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}
void HS6200_Peak_Detector_Power_100mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT6;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}
void HS6200_Peak_Detector_Power_150mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT5;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}
void HS6200_Peak_Detector_Power_200mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT4;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}
void HS6200_Peak_Detector_Power_250mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT3;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}
void HS6200_Peak_Detector_Power_300mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT2;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}
void HS6200_Peak_Detector_Power_350mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT1;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}
void HS6200_Peak_Detector_Power_400mV(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[1] = HS6200_BNAK1_PKDET_VREF_BIT0;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET, Temp, 4);	
}


//HS6200B1新增 New peak detector configuration      wangs 2013-12-26
void HS6200B1_Old_Peak_Detector_Activate(U8 DevNum)                        //Bank1   Add:13    agc_sel(BIT23):0
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
	Temp[2] &=~HS6200_BNAK1_PKDET_ACTIVATE_EN;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Activate(U8 DevNum)                        //Bank1   Add:13    agc_sel(BIT23):1
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
	Temp[2] |= HS6200_BNAK1_PKDET_ACTIVATE_EN;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_VH_VL_Configuration(U8 DevNum, U8 Value_VH_VL)
{
    U8 Value_Vh;
    U8 Value_Vl;
    Value_Vh = (Value_VH_VL >> 4);                //VH occupies the high half byte
    Value_Vl = (Value_VH_VL & 0x0f);              //VL occupies the low half byte
    
    switch(Value_Vh)                //VH
    {
        case AT_PDT_VH0:                    //  000————2/16*mixer_cm
            HS6200B1_New_Peak_Detector_Power_VH0(DevNum);
//            USB_ACK_2_Host(DevNum,0,NULL,0);
        break;
        case AT_PDT_VH1:                    //  001————3/16*mixer_cm
            HS6200B1_New_Peak_Detector_Power_VH1(DevNum);
//            USB_ACK_2_Host(DevNum,0,NULL,0);
        break;
        case AT_PDT_VH2:                    //  010————4/16*mixer_cm
            HS6200B1_New_Peak_Detector_Power_VH2(DevNum);
//            USB_ACK_2_Host(DevNum,0,NULL,0); 
        break;
        case AT_PDT_VH3:                    //  011————5/16*mixer_cm
            HS6200B1_New_Peak_Detector_Power_VH3(DevNum);
//            USB_ACK_2_Host(DevNum,0,NULL,0);
        break;
        case AT_PDT_VH4:                    //  100————6/16*mixer_cm
            HS6200B1_New_Peak_Detector_Power_VH4(DevNum);
//            USB_ACK_2_Host(DevNum,0,NULL,0);
        break;
        default:
            #ifdef HS6200_DEBUG
                #if(DEBUG_OUT_ERR==LED_OUT_ERR)
                    LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
                #else
                    USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
                #endif
            #endif  /*HS6200_DEBUG*/
        break;
    }
    switch(Value_Vl)                //VL
    {
        case AT_PDT_VL0:                    //00————1/10*VH
            HS6200B1_New_Peak_Detector_Power_VL0(DevNum);
            USB_ACK_2_Host(DevNum,0,NULL,0);
        break;
        case AT_PDT_VL1:                    //01————1/8*VH
            HS6200B1_New_Peak_Detector_Power_VL1(DevNum);
            USB_ACK_2_Host(DevNum,0,NULL,0);
        break;
        case AT_PDT_VL2:                    //10————1/6*VH
            HS6200B1_New_Peak_Detector_Power_VL2(DevNum);
            USB_ACK_2_Host(DevNum,0,NULL,0);
        break;
        default:
            #ifdef HS6200_DEBUG
                #if(DEBUG_OUT_ERR==LED_OUT_ERR)
                    LED_Out_Err_Info(ERR_USB_PACKET_AT_CMD);
                #else
                    USB_Out_Err_Info(DevNum,ERR_USB_PACKET_AT_CMD);
                #endif
            #endif  /*HS6200_DEBUG*/
        break;
    }
}

void HS6200B1_New_Peak_Detector_Power_VH0(U8 DevNum)                        //Bank1   Add:13    pkdet_vrefc(BIT28|27|26):000
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREFC;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Power_VH1(U8 DevNum)                        //Bank1   Add:13    pkdet_vrefc(BIT28|27|26):001
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREFC;
    Temp[3] |=HS6200_BNAK1_NEW_PKDET_VREFC_BIT2;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Power_VH2(U8 DevNum)                        //Bank1   Add:13    pkdet_vrefc(BIT28|27|26):010
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREFC;
    Temp[3] |=HS6200_BNAK1_NEW_PKDET_VREFC_BIT3;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Power_VH3(U8 DevNum)                        //Bank1   Add:13    pkdet_vrefc(BIT28|27|26):011
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREFC;
    Temp[3] |=(HS6200_BNAK1_NEW_PKDET_VREFC_BIT3+HS6200_BNAK1_NEW_PKDET_VREFC_BIT2);
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Power_VH4(U8 DevNum)                        //Bank1   Add:13    pkdet_vrefc(BIT28|27|26):100
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREFC;
    Temp[3] |=HS6200_BNAK1_NEW_PKDET_VREFC_BIT4;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Power_VL0(U8 DevNum)                       //Bank1   Add:13    pkdet_vref2c(BIT25|24):00
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREF2C;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Power_VL1(U8 DevNum)                       //Bank1   Add:13    pkdet_vref2c(BIT25|24):01
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREF2C;
    Temp[3] |=HS6200_BNAK1_NEW_PKDET_VREF2C_BIT0;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

void HS6200B1_New_Peak_Detector_Power_VL2(U8 DevNum)                       //Bank1   Add:13    pkdet_vref2c(BIT25|24):10
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1,Temp,4);
    Temp[3] &=~HS6200_BNAK1_NEW_PKDET_VREF2C;
    Temp[3] |=HS6200_BNAK1_NEW_PKDET_VREF2C_BIT1;
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_FAGC_CTRL_1, Temp, 4);	
}

//END wangs 2013-12-26



//---------------------------------5.HS6200 PA power--------------------------------
U8 HS6200_Read_PA_Power(U8 DevNum)
{
	U8 Temp;
	U8 ret=0x00;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp&=HS6200_BANK0_PA_POWER;
	switch(Temp)
	{
		case HS6200_BANK0_PA_POWER_n18dBm:			/*000*/
			ret=0x01;
		break;
		case HS6200_BANK0_PA_POWER_n12dBm:		    /*010*/
			ret=0x02;
		break;
		case HS6200_BANK0_PA_POWER_n6dBm:			/*100*/
			ret=0x03;
		break;
		case HS6200_BANK0_PA_POWER_0dBm:		    /*110*/
			ret=0x04;
		break;
		case HS6200_BANK0_PA_POWER_5dBm:		    /*xx1*/
			ret=0x05;
		break;
		default:
			ret=0x05;
		break;
	}
	return ret;		
}



void HS6200_PA_Power_n18dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200_BANK0_PA_POWER;
	Temp|=HS6200_BANK0_PA_POWER_n18dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_PA_Power_n12dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200_BANK0_PA_POWER;
	Temp|=HS6200_BANK0_PA_POWER_n12dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_PA_Power_n6dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200_BANK0_PA_POWER;
	Temp|=HS6200_BANK0_PA_POWER_n6dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_PA_Power_0dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200_BANK0_PA_POWER;
	Temp|=HS6200_BANK0_PA_POWER_0dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_PA_Power_5dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200_BANK0_PA_POWER;
	Temp|=HS6200_BANK0_PA_POWER_5dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}

//HS6200B1 wangs 2013-12-27
void HS6200B1_PA_Power_Voltage_1V8(U8 DevNum)
{
    U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_RF_IVGEN,Temp,4);
    Temp[0] &=~HS6200B1_BNAK1_PA_POWER_VOLTAGE;                             //1.8V power ,pa_voltage(Bit7)=0
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_RF_IVGEN, Temp, 4);	
}

void HS6200B1_PA_Power_Voltage_3V(U8 DevNum)
{
    U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_RF_IVGEN,Temp,4);
    Temp[0] |=HS6200B1_BNAK1_PA_POWER_VOLTAGE;                             //3V power ,pa_voltage(Bit7)=1
	nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_RF_IVGEN, Temp, 4);	
}

void HS6200B1_PA_Power_n18dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200B1_BANK0_PA_POWER;
	Temp|=HS6200B1_BANK0_PA_POWER_n18dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}

void HS6200B1_PA_Power_n12dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200B1_BANK0_PA_POWER;
	Temp|=HS6200B1_BANK0_PA_POWER_n12dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}

void HS6200B1_PA_Power_n6dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200B1_BANK0_PA_POWER;
	Temp|=HS6200B1_BANK0_PA_POWER_n6dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}

void HS6200B1_PA_Power_0dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200B1_BANK0_PA_POWER;
	Temp|=HS6200B1_BANK0_PA_POWER_0dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}

void HS6200B1_PA_Power_5dBm(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank0_Activate(DevNum);	
	Temp=nRF24L01_X_read_reg(DevNum, HS6200_BANK0_RF_SETUP);
	Temp &=~HS6200B1_BANK0_PA_POWER;
	Temp|=HS6200B1_BANK0_PA_POWER_5dBm;	
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}


//END   wangs   2013-12-27

 void HS6200_PA_power(U8 DevNum,U8 PA_Power)
{

	switch(PA_Power)
	{
		case 0x00: 
			HS6200_PA_Power_n18dBm(DevNum);
		break;
		case 0x01: 
			HS6200_PA_Power_n12dBm(DevNum);
		break;
		case 0x02: 
			HS6200_PA_Power_n6dBm(DevNum);
		break;
		case 0x03: 
			HS6200_PA_Power_0dBm(DevNum);
		break;
		case 0x04: 
			HS6200_PA_Power_5dBm(DevNum);
		break;
		default:
		break;									 
	}
}

//6.calibration manual mode
//VCO calbiration
void HS6200_VCO_Ctuning_Calb_Mn_On(U8 DevNum)
{	
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[0]|=HS6200_BANK1_CTUNING_MN_ON;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);		
}
void HS6200_VCO_Ctuning_Calb_Mn_Off(U8 DevNum)
{	
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[0]&=~HS6200_BANK1_CTUNING_MN_ON;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);		
}
U8 HS6200_Read_VCO_Ctuning(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank1_Activate(DevNum);
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK1_CTUNING);
	return Temp;	
}
void HS6200_Write_VCO_Ctuning(U8 DevNum,U8 Ctuning_Num)
{
   HS6200_Bank1_Activate(DevNum);
   nRF24L01_X_write_reg(DevNum,HS6200_BANK1_CTUNING,Ctuning_Num);
}

void HS6200_VCO_Ftuning_Calb_Mn_On(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[0]|=HS6200_BANK1_FTUNING_MN_ON;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}
void HS6200_VCO_Ftuning_Calb_Mn_Off(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[0]&=~HS6200_BANK1_FTUNING_MN_ON;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}

U8 HS6200_Read_VCO_Ftuning(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank1_Activate(DevNum);
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK1_FTUNING);
	return Temp;
}
void HS6200_Write_VCO_Ftuning(U8 DevNum,U8 Ftuning_Num)
{
   	HS6200_Bank1_Activate(DevNum);
    nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FTUNING,Ftuning_Num);
}

//VCO LDO calibration
void HS6200_VCO_LDO_Calb_Mn_On(U8 DevNum)
{
	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);	
	Temp[0]|=HS6200_BANK1_VCO_LDO_CAL_MN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL, Temp, 2);	
}
void HS6200_VCO_LDO_Calb_Mn_Off(U8 DevNum)
{
	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);	
	Temp[0]&=~HS6200_BANK1_VCO_LDO_CAL_MN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL, Temp, 2);	
}

U8 HS6200_Read_VCO_LDO_Calb(U8 DevNum)
{
	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);	
	Temp[1]&=HS6200_BANK1_VCO_LDO_CAL_REG;
	return Temp[1];	
}
void HS6200_Write_VCO_LDO_Calb(U8 DevNum,U8 Calb_Value)
{
	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);	
	Temp[1]&=~HS6200_BANK1_VCO_LDO_CAL_REG;
	Temp[1] |=(Calb_Value & HS6200_BANK1_VCO_LDO_CAL_REG);
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL,Temp,2);	
}

//DAC gain calibration
void HS6200_DAC_Range_Mn_On(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[0]|=HS6200_BANK1_DAC_GAIN_CAL_MN;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}

void HS6200_DAC_Range_Mn_Off(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[0]&=~HS6200_BANK1_DAC_GAIN_CAL_MN;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}

U8 HS6200_Read_DAC_Gain(U8 DevNum)
{
	U8 Temp;
	HS6200_Bank1_Activate(DevNum);
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK1_DAC_RANGE);
	return Temp;
}
void HS6200_Write_DAC_Gain(U8 DevNum,U8 Gain)
{
	 HS6200_Bank1_Activate(DevNum);
	 nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DAC_RANGE,Gain);
}

//DC offset calibration
void HS6200_DOC_DAC_Mn_On(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[2]|=HS6200_BANK1_DOC_DAC_MN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
}

void HS6200_DOC_DAC_Mn_Off(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[2]&=~HS6200_BANK1_DOC_DAC_MN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);		
}



U8 HS6200_Read_DC_Offset_I(U8 DevNum)
{
	 U8 Temp;
	 HS6200_Bank1_Activate(DevNum);
	 Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK1_DOC_DACI);
	 return Temp;
}
void HS6200_Write_DC_Offset_I(U8 DevNum,U8 DC_Ofst_I)
{
	 HS6200_Bank1_Activate(DevNum);
	 nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DOC_DACI,DC_Ofst_I);
}

U8 HS6200_Read_DC_Offset_Q(U8 DevNum)
{
	 U8 Temp;
	 HS6200_Bank1_Activate(DevNum);
	 Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK1_DOC_DACQ);
	 return Temp;
}
void HS6200_Write_DC_Offset_Q(U8 DevNum,U8 DC_Ofst_Q)
{
	 HS6200_Bank1_Activate(DevNum);
	 nRF24L01_X_write_reg(DevNum,HS6200_BANK1_DOC_DACI,DC_Ofst_Q);
}

void HS6200_Calibration(U8 DevNum)
{
	U8 Temp[4];
	U8 Rx_Config=0;  //TX RX config
	
	HS6200_Bank0_Activate(DevNum);
	if( nRF24L01_X_is_prx(DevNum) ) Rx_Config=1;   //PRX
	else Rx_Config=0;
	nRF24L01_X_set_ptx(DevNum);  //设置成PTX才能进行校准
	
	HS6200_Bank1_Activate(DevNum);	  //Bank1   	
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	
	Temp[3]|=HS6200_BANK1_CAL_EN;    //cal_en=1
	Temp[2]&=~HS6200_BANK1_DAC_CALMODE_REG;   //DAC_Calmode_reg=1	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);   //calibration

	nRF24L01_X_ce_high_pulse(DevNum);	               //ce pulse 10us    
        chThdSleepMilliseconds(100);	

	Temp[3] &=~ HS6200_BANK1_CAL_EN;
	Temp[2] |= HS6200_BANK1_DAC_CALMODE_REG;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, Temp, 4);	
  HS6200_Bank0_Activate(DevNum);
	if(Rx_Config)nRF24L01_X_set_prx(DevNum);
}

//HS6200B1_xxxx_Calibration()适用于HS6200B1 chip的calibration. 
//上电之后的校准
//请参考<HS6200B1 Software Guideline for Keyboard and Mouser V0.1>.
void HS6200B1_Powerup_Calibration(U8 DevNum)
{   
	  U8 temp[5];
    U8 Rx_Config_Flag=0;  //TX RX config 
	    
    HS6200_Bank0_Activate(DevNum);
    if( nRF24L01_X_is_prx(DevNum) ) Rx_Config_Flag=0x01;   //PRX
    else Rx_Config_Flag=0x00;  //PTX
    nRF24L01_X_set_ptx(DevNum);   //设置成PTX 进行校准   
    
    HS6200_Bank1_Activate(DevNum);  //慢速校准 1Mbps
    temp[0]=0x40;
    temp[1]=0x00;
    temp[2]=0x00;    //DAC_CALMODE_REG=0为快速校准,若DAC_CALMODE_REG=1, 为慢速校准
    temp[3]=0xe4;
    nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0,temp,4);

    temp[0]=0x20;
    temp[1]=0x08;
    temp[2]=0x10;        //afc_w_sel=0b01, AFC校准时间为180us
    temp[3]=0x40;
    temp[4]=0x50;
    nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,temp,5);

    HS6200_Bank0_Activate(DevNum);
    nRF24L01_X_write_reg(DevNum, HS6200_BANK0_RF_CH, 0x28);

	  HS6200_Bank1_Activate(DevNum);
	  nRF24L01_X_write_reg(DevNum, HS6200_BANK1_FDEV, 0x14);    //对应160KHz频偏， 若采用2MHz速率，则为0x29,频偏为320KHz

	  temp[0]=0x9F;
	  temp[1]=0x64;
	  temp[2]=0x00;
	  temp[3]=0x01;
	  nRF24L01_X_write_reg_multibytes(DevNum,HS6200_BANK1_RF_IVGEN,temp, 4);
	  
	  nRF24L01_X_ce_high_pulse(DevNum);	               //ce pulse 10us, 开始进行校准
		do
		{
	  	chThdSleepMilliseconds(40);   //延时40ms
	  	nRF24L01_X_read_reg_multibytes(DevNum,HS6200B1_BANK1_STATE, temp, 2); 	  	
		}
	 	while((temp[1]&0x0F)!=0x00);   //CAL_ST_CS=0x00校准完成. 

	  temp[0]=0x40;
	  temp[1]=0x00;
	  temp[2]=0x20;   //DAC_CALMODE_REG=0,校准完成后,此位要设为1
	  temp[3]=0xe0;   //CAL_EN=0
	  nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0, temp, 4);	
		
}



//Carrier calibration
void HS6200_Carrier_Calb(U8 DevNum,U8 Carrier)
{
	HS6200_Bank1_Activate(DevNum);
    nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FDEV,Carrier);
	HS6200_Calibration(DevNum);
}

//7.continuous carrier transmit
void HS6200_Cont_Wave_Start(U8 DevNum)
{
	U8 Reg_Val;

	HS6200_Bank1_Activate(DevNum);	  //Bank1
	nRF24L01_X_write_reg(DevNum,HS6200_BANK1_FDEV,0x00);  //  频偏置零

  HS6200_Calibration(DevNum);
	
	HS6200_Bank0_Activate(DevNum);	    //Bank0
	nRF24L01_X_flush_tx(DevNum);		//flush tx fifo
	
	Reg_Val=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_SETUP);	  //CONT_WAVE=1
	Reg_Val |=	HS6200_BANK0_CONT_WAVE;
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Reg_Val);
	    
	Reg_Val=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_CONFIG);   //PTX 
	Reg_Val &=~HS6200_BANK0_PRX;			 
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_CONFIG,Reg_Val);
	nRF24L01_X_ce_high(DevNum);		    //pull up ce
} 

void HS6200_Cont_Wave_Stop(U8 DevNum)
{
	U8 Temp;
	nRF24L01_X_ce_low(DevNum);	        //pull dewn ce
	HS6200_Bank0_Activate(DevNum);	    //Bank0
	Temp=nRF24L01_X_read_reg(DevNum,HS6200_BANK0_RF_SETUP);
	Temp&=~HS6200_BANK0_CONT_WAVE;					 //CONT_WAVE=0
	nRF24L01_X_write_reg(DevNum,HS6200_BANK0_RF_SETUP,Temp);
}
void HS6200_PLL_DAC_On(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[3]|=HS6200_BANK1_PLL_DAC_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0,Temp,4); 
}
void HS6200_PLL_DAC_Off(U8 DevNum)
{
	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL0,Temp,4);
	Temp[3]&=~HS6200_BANK1_PLL_DAC_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL0,Temp,4); 	
} 

//Bypass_Cp_diox on
void HS6200_Bp_Cp_Diox_On(U8 DevNum)
{
 	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);
	Temp[1]&=~HS6200_BANK1_BP_CP_DIOX;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL,Temp,2); 
}
void HS6200_Bp_Cp_Diox_Off(U8 DevNum)
{
 	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);
	Temp[1] |= HS6200_BANK1_BP_CP_DIOX;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL,Temp,2); 
}
//VC_DET on
void HS6200_Vc_Det_On(U8 DevNum)
{
 	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);
	Temp[1] &=~ HS6200_BANK1_VC_DET;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL,Temp,2); 
}
void HS6200_Vc_Det_Off(U8 DevNum)
{
 	U8 Temp[2];  //2Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_CAL_CTL,Temp,2);
	Temp[1] |= HS6200_BANK1_VC_DET;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_CAL_CTL,Temp,2); 
}

void HS6200_PLL_Icp_Sel_80uA(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[3]&=~HS6200_BANK1_PLL_ICP_SEL;
	Temp[3]|=HS6200_BANK1_PLL_ICP_SEL_80;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}

void HS6200_PLL_Icp_Sel_120uA(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[3]&=~HS6200_BANK1_PLL_ICP_SEL;
	Temp[3]|=HS6200_BANK1_PLL_ICP_SEL_120;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}

void HS6200_PLL_Icp_Sel_160uA(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[3]&=~HS6200_BANK1_PLL_ICP_SEL;
	Temp[3]|=HS6200_BANK1_PLL_ICP_SEL_160;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}

void HS6200_PLL_Icp_Sel_200uA(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[3]&=~HS6200_BANK1_PLL_ICP_SEL;
	Temp[3]|=HS6200_BANK1_PLL_ICP_SEL_200;	
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}

void HS6200_PLL_Vdiv2_Sel_550mV(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[0]&=~HS6200_BANK1_PLL_VDIV2_SEL;
	Temp[0]|=HS6200_BANK1_PLL_VDIV2_SEL_550mV;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}
void HS6200_PLL_Vdiv2_Sel_600mV(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[0]&=~HS6200_BANK1_PLL_VDIV2_SEL;
	Temp[0]|=HS6200_BANK1_PLL_VDIV2_SEL_600mV;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}
void HS6200_PLL_Vdiv2_Sel_650mV(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[0]&=~HS6200_BANK1_PLL_VDIV2_SEL;
	Temp[0]|=HS6200_BANK1_PLL_VDIV2_SEL_650mV;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}

void HS6200_PLL_Vdiv2_Sel_700mV(U8 DevNum)
{
 	U8 Temp[4];  //4Byte width 
	HS6200_Bank1_Activate(DevNum);
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_TEST_PKDET,Temp,4);
	Temp[0]&=~HS6200_BANK1_PLL_VDIV2_SEL;
	Temp[0]|=HS6200_BANK1_PLL_VDIV2_SEL_700mV;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_TEST_PKDET,Temp,4);
}

//-----------------------08.Continuous test partern transmit--------------------------
//TEST_PAT_EN=1后，不对CE进行操作。stop test pattern 后，需根据需要对CE拉高拉低。
void HS6200_Cont_Test_Pat_Tx_Start(U8 DevNum)
{ 
	U8 Temp[4];
//	HS6200_Calibration(DevNum);
	HS6200_Bank1_Activate(DevNum);	  //Bank1
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_CTRL,Temp,4);
	Temp[2] |= HS6200_BANK1_TEST_PAT_EN;					 //Test_Pat_en
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_AGC_CTRL,Temp,4);
	nRF24L01_X_ce_high(DevNum);
} 

void HS6200_Cont_Test_Pat_Tx_Stop(U8 DevNum)   // stop patten_en=0   ce pull low
{
	U8 Temp[4];	 
	HS6200_Bank1_Activate(DevNum);	  //Bank1
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_CTRL,Temp,4);
	Temp[2]&=~HS6200_BANK1_TEST_PAT_EN;					 //Test_Pat_en=0
	nRF24L01_X_write_reg_multibytes_no_ce(DevNum, HS6200_BANK1_AGC_CTRL,Temp,4);
	//nRF24L01_X_ce_low(DevNum);	
}
void HS6200_Cont_Test_Pat(U8 DevNum,U8 Test_Pat)
{
	U8 Temp[4];
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_CTRL,Temp,4);
	Temp[3]=Test_Pat;   //write test patten         //Test_Pat_en 不是控制Test_Pat是否能写的，所以可以一起写
	nRF24L01_X_write_reg_multibytes_no_ce(DevNum, HS6200_BANK1_AGC_CTRL,Temp,4);  //写Test_Pat，不对ce进行任何操作
}

//伪随机数

void HS6200_Test_Pattern_Pseudo_Random_Num(U8 DevNum)
{
	U8 Temp[4];	 
	Test_Pattern_Psdo_Rdm_Num_Flag[DevNum]=TEST_PAT_PSDO_RDM_NUM_FLG_START;	  //启动pseudo random number test
   	HS6200_Bank1_Activate(DevNum);	  //Bank1
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_AGC_CTRL,Temp,4);
	if(DevNum==DEV_0)
	{
		while(Test_Pattern_Psdo_Rdm_Num_Flag[DevNum]!=TEST_PAT_PSDO_RDM_NUM_FLG_STOP) //一直处于pseudo random number test 
		{
//			EA=1;					 //开启中断				                     //则此时会进入USB中断 ，判断是否stop pesudo random test
//			Temp[3]=rand()%256;		 //得到pseudo random num 
//			EA=0; 					 //关闭中断,等待下次开启中断 在判断
			nRF24L01_X_write_reg_multibytes_no_ce(DevNum, HS6200_BANK1_AGC_CTRL,Temp,4);  //写Test_Pat，不对ce进行任何操作
		}
	}
} 

void HS6200_Soft_Rst(U8 DevNum)
{
	U8 Temp[4];
	HS6200_Bank1_Activate(DevNum);	  //Bank1
	nRF24L01_X_read_reg_multibytes(DevNum,HS6200_BANK1_PLL_CTL1,Temp,4);
	Temp[3]|=HS6200_BANK1_SOFT_RST_EN;
	nRF24L01_X_write_reg_multibytes(DevNum, HS6200_BANK1_PLL_CTL1,Temp,4);   //soft reset后寄存器复位, 寄存器会自动处于BANK0状态.
}




/*-----------------------------------End Of File----------------------------------*/

