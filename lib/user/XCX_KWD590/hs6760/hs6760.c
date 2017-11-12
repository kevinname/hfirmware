/**
 * @file    user/hs6760.c
 * @brief   HS6760 FM transmitter driver.
 * @details 
 *
 * @addtogroup  user
 * @details 
 * @{
 */

#include "lib.h"
#include "hs6760.h"

#if HAL_USE_I2C && HS_USE_HS6760

#define HS6760_SLAVE_ADDR    0x18    /* w/o R/W bit */
#define HS6760_I2C_SPEED     400000  /* 400kHz */

static uint8_t m_hs6760_rev;
static hs_i2c_handler_t m_hs6760_i2c_handle;

static void _hs6760_WriteReg(uint8_t reg_addr, uint8_t reg_val)
{
  hs_i2c_write(m_hs6760_i2c_handle, reg_addr, &reg_val, 1);
}

static uint8_t _hs6760_ReadReg(uint8_t reg_addr)
{
  uint8_t reg_val = 0;
  hs_i2c_read(m_hs6760_i2c_handle, reg_addr, &reg_val, 1);
  return reg_val;
}

/*
 * @brief set frequency deviation
 *
 * @param[in] freq_dev        frequency deviation
 *
 * @return 0 always
 */
int hs6760_fm_set_dev(hs6760_freq_dev_t freq_dev)
{
  uint8_t reg_val;

  reg_val = _hs6760_ReadReg(0x03);
  reg_val &= ~(0x3u << 0);
  reg_val |= freq_dev;
  _hs6760_WriteReg(0x03, reg_val);
  return 0;
}

/*
 * @brief set FM frequency
 *
 * @param[in] freq_dmhz         frequency in MHz/10, 1039 means 103.9MHz
 *
 * @return 0 successfully, -1 out of range
 */
int hs6760_fm_set_freq(int freq_dmhz)
{
  uint8_t reg_val;
  uint16_t freq;

  if (freq_dmhz < 270/*27MHz*/)
    return -1;
  if (freq_dmhz > 1250/*125MHz*/)
    return -1;

  freq = (uint16_t)(freq_dmhz * 2);
  /* low byte */
  _hs6760_WriteReg(0x00, freq & 0xff);
  /* high byte */
  reg_val = _hs6760_ReadReg(0x01);
  reg_val &= ~0x3F;
  reg_val |= (freq>>8) & 0x3F;
  _hs6760_WriteReg(0x01, reg_val);
  osDelay(10);

  /* enable FM channel */
  reg_val = _hs6760_ReadReg(0x02);
  reg_val &=~1;
  _hs6760_WriteReg(0x02,reg_val);
  osDelay(10);
  reg_val |=1;
  _hs6760_WriteReg(0x02,reg_val);
  return 0;
}

/*
 * @brief set work mode: normal, mute or sleep
 *
 * @param[in] mode             0-normal, 1-mute, 2-sleep
 *
 * @return 0 always
 */
int hs6760_fm_set_mode(hs6760_mode_t mode)
{
  uint8_t reg_val;

  reg_val = _hs6760_ReadReg(0x01);
  reg_val &= ~(0x3u << 6);
  reg_val |= (uint8_t)mode << 6;
  _hs6760_WriteReg(0x01, reg_val);

  /* FIXME: if A1,A2, reset FSM to change work mode to normal */
  //if ((m_hs6760_rev < 0x61) && (NORMAL_MODE == mode));
  return 0;
}

/*
 * @brief set audio input: PGA gain, and stereo or mono
 *
 * @param[in] dB         PGA gain in dB, near to low in hardware table
 * @param[in] stereo     true-stereo false-mono
 * +--------+------+------+------+------+------+------+------+------+
 * | PGA    | 0000 | 1000 | 0001 | 1001 | 0010 | 1010 | 0011 | 0100 |
 * +--------+------+------+------+------+------+------+------+------+
 * |GAIN(db)|  -6  | -2.5 |  0   |  3.5 |   6  |  9.5 |  12  |  15  |
 * +--------+------+------+------+------+------+------+------+------+
 * | PGA    | 1011 | 0101 | 1100 | 0110 | 0111 | 1110 | 1101 | 1111 |
 * +--------+------+------+------+------+------+------+------+------+
 * |GAIN(db)| 15.5 |  18  | 18.5 | 17.5 | 20.5 |  21  | 21.5 |  24  |
 * +----------------------------------------------------------------+
 *
 * @return 0 always
 */
static int m_pga_tbl[] = {-6, 0, 6, 12, 15, 18, 17, 20,
                          -2, 3, 9, 15, 18, 21, 21, 24};
int hs6760_fm_set_audio(int dB, bool stereo)
{
  uint8_t reg_val, idx, idx_min = 0;

  for (idx = 0; idx < 16; idx++) {
    if (m_pga_tbl[idx] <= dB)
      idx_min = idx;
    if (m_pga_tbl[idx] == dB)
      break;
  }
  reg_val = _hs6760_ReadReg(0x02);
  reg_val &= ~(0xfu << 4);
  reg_val |= idx_min << 4;
  _hs6760_WriteReg(0x02, reg_val);

  reg_val = _hs6760_ReadReg(0x03);
  if (stereo)
    reg_val |= (0x1u << 7);
  else
    reg_val &= ~(0x1u << 7);
  _hs6760_WriteReg(0x03, reg_val);
  return 0;
}

/*
 * @brief set FM transmitter PA power
 *
 * @param[in] lvl         PA power level
 *
 * @return 0 always
*/
int hs6760_fm_set_power(int lvl)
{
  uint8_t reg_val;

  reg_val = _hs6760_ReadReg(0x07);
  reg_val &= ~(0x1fu << 0);
  reg_val |= lvl << 0;
  _hs6760_WriteReg(0x07, reg_val);

  /* set low: reset FSM to change PA power */
  reg_val &= ~(0x1u << 7);
  _hs6760_WriteReg(0x07, reg_val);
  osDelay(10);
  reg_val |= (0x1u << 7);
  _hs6760_WriteReg(0x07, reg_val);

  return 0;
}

hs_i2c_handler_t hs6760_fm_open(hs6760_xtal_t xtal)
{
  uint8_t reg_val;

  m_hs6760_i2c_handle = hs_i2c_init(HS6760_I2C_SPEED, HS6760_SLAVE_ADDR, 7/*7-bit slave address*/, 1/*1-byte addressing*/);
  if (!m_hs6760_i2c_handle)
    return 0;

  m_hs6760_rev = _hs6760_ReadReg(0x09);
  hs_printf("chip ID 0x%x: HS6760A%u\r\n", m_hs6760_rev, m_hs6760_rev >> 5);

  /* crystal */
  reg_val = _hs6760_ReadReg(0x02);
  reg_val &= ~(0x3u << 1);
  reg_val |= (uint8_t)xtal << 1;
  _hs6760_WriteReg(0x02, reg_val);
  osDelay(10);

  /* audio source: PGA, stereo */
  hs6760_fm_set_audio(10/*dB*/, true/*stereo*/);

  /* work mode: mute in default */
  hs6760_fm_set_mode(MUTE_MODE);

#if 0
  /* auto mute */
  _hs6760_WriteReg(0x04, 0xff);
  reg_val = _hs6760_ReadReg(0x05);
  reg_val &= ~(0x7u << 4);
  reg_val |= 0x1u << 4; //auto-mute detect window time: 2s
  _hs6760_WriteReg(0x05, reg_val);
#endif

  reg_val=0x01;
  //_hs6760_WriteReg(0x11,reg_val);    //audio path
  osDelay(10);

  /* FM channel */
  hs6760_fm_set_dev(DEV_22K5);
  //hs6760_fm_set_freq(905/*90.5MHz*/);

  /* PA power */
  hs6760_fm_set_power(31);
    
  return m_hs6760_i2c_handle;
}

int hs6760_fm_close(void)
{
  hs_i2c_uninit(m_hs6760_i2c_handle);
  m_hs6760_i2c_handle = 0;
  return 0;
}

#endif
