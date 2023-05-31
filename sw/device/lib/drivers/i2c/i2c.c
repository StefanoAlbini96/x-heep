/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : i2c.c                                                        **
** date     : 16/05/2023                                                   **
**                                                                         **
*****************************************************************************
**                                                                         **
**                                                                         **
*****************************************************************************
	 
*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   i2c.c
* @date   17/04/2023
* @brief  This is the main file for the HAL of the I2C peripheral
*
* In this files there are definitions of low level HAL functions to interact
* with the registers of the I2C peripheral.
* The functionalities implemented allow to configure the peripheral with
* parameters for the FMT and RX fifos, enable or disable interrupts and write
* and read bytes.
*/


/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "i2c.h"

#include "i2c_regs.h"
#include "i2c_structs.h"

#include "bitfield.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

static const uint32_t kNanosPerKBaud = 1000000;  // One million.

/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

/**
 * Performs a 32-bit integer unsigned division, rounding up. The bottom
 * 16 bits of the result are then returned.
 */
static uint16_t round_up_divide(uint32_t a, uint32_t b);


/**
 * Computes default timing parameters for a particular I2C speed, given the
 * clock period, in nanoseconds.
 */
static i2c_config_t default_timing_for_speed(i2c_speed_t speed,
                                                 uint32_t clock_period_nanos);

/**
 * Calculates the bit index relative to the irq parameter.
*/
bool irq_index(i2c_irq_t irq, bitfield_bit32_index_t *bit_index);

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

i2c_result_t i2c_compute_timing(i2c_timing_config_t timing_config,
                                        i2c_config_t *config) {
  if (config == NULL) {
    return kI2cBadArg;
  }
  uint32_t lowest_target_device_speed_khz;
  switch (timing_config.lowest_target_device_speed) {
    case kI2cSpeedStandard:
      lowest_target_device_speed_khz = 100;
      break;
    case kI2cSpeedFast:
      lowest_target_device_speed_khz = 400;
      break;
    case kI2cSpeedFastPlus:
      lowest_target_device_speed_khz = 1000;
      break;
    default:
      return kI2cBadArg;
  }

  *config = default_timing_for_speed(timing_config.lowest_target_device_speed,
                                     timing_config.clock_period_nanos);

  config->rise_cycles = round_up_divide(timing_config.sda_rise_nanos,
                                        timing_config.clock_period_nanos);
  config->fall_cycles = round_up_divide(timing_config.sda_fall_nanos,
                                        timing_config.clock_period_nanos);

  uint32_t scl_period_nanos = timing_config.scl_period_nanos;
  uint32_t slowest_scl_period_nanos =
      kNanosPerKBaud / lowest_target_device_speed_khz;
  if (scl_period_nanos < slowest_scl_period_nanos) {
    scl_period_nanos = slowest_scl_period_nanos;
  }
  uint16_t scl_period_cycles =
      round_up_divide(scl_period_nanos, timing_config.clock_period_nanos);

  // Lengthen the SCL high period to accommodate the desired SCL period.
  uint16_t lengthened_high_cycles = scl_period_cycles -
                                    config->scl_time_low_cycles -
                                    config->rise_cycles - config->fall_cycles;
  if (lengthened_high_cycles > config->scl_time_high_cycles) {
    config->scl_time_high_cycles = lengthened_high_cycles;
  }

  return kI2cOk;
}

i2c_result_t i2c_configure(i2c_config_t config)
{
  // Configure TIMING0 register
  i2c_peri->TIMING0 = bitfield_write(i2c_peri->TIMING0, 
                                      I2C_TIMING0_THIGH_MASK, 
                                      I2C_TIMING0_THIGH_OFFSET,
                                      config.scl_time_high_cycles);
  i2c_peri->TIMING0 = bitfield_write(i2c_peri->TIMING0, 
                                      I2C_TIMING0_TLOW_MASK, 
                                      I2C_TIMING0_TLOW_OFFSET,
                                      config.scl_time_low_cycles);
  
  // Configure TIMING1 register
  i2c_peri->TIMING1 = bitfield_write(i2c_peri->TIMING1,
                                      I2C_TIMING1_T_R_MASK,
                                      I2C_TIMING1_T_R_OFFSET,
                                      config.rise_cycles);
  i2c_peri->TIMING1 = bitfield_write(i2c_peri->TIMING1,
                                      I2C_TIMING1_T_F_MASK,
                                      I2C_TIMING1_T_F_OFFSET,
                                      config.fall_cycles);
        
  // Configure TIMING2 register
  i2c_peri->TIMING2 = bitfield_write(i2c_peri->TIMING2,
                                      I2C_TIMING2_TSU_STA_MASK,
                                      I2C_TIMING2_TSU_STA_OFFSET,
                                      config.start_signal_setup_cycles);
  i2c_peri->TIMING2 = bitfield_write(i2c_peri->TIMING2,
                                      I2C_TIMING2_THD_STA_MASK,
                                      I2C_TIMING2_THD_STA_OFFSET,
                                      config.start_signal_hold_cycles);

  // Configure TIMING3 register
  i2c_peri->TIMING3 = bitfield_write(i2c_peri->TIMING3,
                                      I2C_TIMING3_TSU_DAT_MASK,
                                      I2C_TIMING3_TSU_DAT_OFFSET,
                                      config.data_signal_setup_cycles);
  i2c_peri->TIMING3 = bitfield_write(i2c_peri->TIMING3,
                                      I2C_TIMING3_THD_DAT_MASK,
                                      I2C_TIMING3_THD_DAT_OFFSET,
                                      config.data_signal_hold_cycles);
                                    
  // Configure TIMING4 register
  i2c_peri->TIMING4 = bitfield_write(i2c_peri->TIMING4,
                                      I2C_TIMING4_TSU_STO_MASK,
                                      I2C_TIMING4_TSU_STO_OFFSET,
                                      config.stop_signal_setup_cycles);
  i2c_peri->TIMING4 = bitfield_write(i2c_peri->TIMING4,
                                      I2C_TIMING4_T_BUF_MASK,
                                      I2C_TIMING4_T_BUF_OFFSET,
                                      config.stop_signal_hold_cycles);

}



i2c_result_t i2c_irq_is_pending(i2c_irq_t irq, bool *is_pending)
{
  if (is_pending == NULL) {
    return kI2cBadArg;
  }

  if(irq < 0 || irq > kI2cNIrqTypes)
  {
    return kI2cBadArg;
  }

  // bitfield_bit32_index_t index = 0;
  // if (!irq_index(irq, &index))
  // {
  //   return kI2cBadArg;
  // }


  *is_pending = bitfield_read(i2c_peri->INTR_STATE, BIT_MASK_1, irq);

  return kI2cOk;
}


i2c_result_t i2c_irq_acknowledge(i2c_irq_t irq) {

  if(irq < 0 || irq > kI2cNIrqTypes)
  {
    return kI2cBadArg;
  }
  // bitfield_bit32_index_t index = 0;
  // if (!irq_index(irq, &index)) {
  //   return kI2cBadArg;
  // }

  i2c_peri->INTR_STATE = bitfield_write(0, 
                                        BIT_MASK_1,
                                        irq,
                                        true);

  return kI2cOk;
}


i2c_result_t i2c_irq_get_enabled(i2c_irq_t irq, i2c_toggle_t *state)
{

  if(irq < 0 || irq > kI2cNIrqTypes)
  {
    return kI2cBadArg;
  }
  // bitfield_bit32_index_t index = 0;

  // if(!irq_index(irq, &index))
  // {
  //   return kI2cBadArg;
  // }

  *state = bitfield_read(i2c_peri->INTR_ENABLE, BIT_MASK_1, irq);

  return kI2cOk;
}


i2c_result_t i2c_irq_set_enabled(i2c_irq_t irq, i2c_toggle_t state)
{

  if (state != kI2cToggleEnabled && state != kI2cToggleDisabled)
  {
    return kI2cBadArg;
  }

  if(irq < 0 || irq > kI2cNIrqTypes)
  {
    return kI2cBadArg;
  }
  // bitfield_bit32_index_t index = 0;
  // if (!irq_index(irq, &index))
  // {
  //   return kI2cBadArg;
  // }

  i2c_peri->INTR_ENABLE = bitfield_write(i2c_peri->INTR_ENABLE, 
                                          BIT_MASK_1, 
                                          irq, 
                                          state);

  return kI2cOk;
}


i2c_result_t i2c_irq_force(i2c_irq_t irq)
{
  if(irq < 0 || irq > kI2cNIrqTypes)
  {
    return kI2cBadArg;
  }

  // bitfield_bit32_index_t index;
  // if (!irq_index(irq, &index)) 
  // {
  //   return kI2cBadArg;
  // }

  i2c_peri->INTR_TEST = bitfield_write(0,
                                        BIT_MASK_1,
                                        irq,
                                        true);

  return kI2cOk;
}


i2c_result_t i2c_irq_disable_all(i2c_irq_snapshot_t *snapshot)
{

  if (snapshot != NULL) {
    *snapshot = i2c_peri->INTR_ENABLE;
  }

  i2c_peri->INTR_ENABLE = 0;

  return kI2cOk;
}


i2c_result_t i2c_irq_restore_all(const i2c_irq_snapshot_t *snapshot)
{
  if (snapshot == NULL) {
    return kI2cBadArg;
  }

  i2c_peri->INTR_ENABLE = *snapshot;

  return kI2cOk;
}
i2c_result_t i2c_reset_rx_fifo() {

  i2c_peri->FIFO_CTRL = bitfield_write(i2c_peri->FIFO_CTRL,
                                        BIT_MASK_1,
                                        I2C_FIFO_CTRL_RXRST_BIT,
                                        true);
  return kI2cOk;
}

i2c_result_t i2c_reset_fmt_fifo() {

  i2c_peri->FIFO_CTRL = bitfield_write(i2c_peri->FIFO_CTRL,
                                        BIT_MASK_1,
                                        I2C_FIFO_CTRL_FMTRST_BIT,
                                        true);
  return kI2cOk;
}


i2c_result_t i2c_set_watermarks(i2c_level_t rx_level, i2c_level_t fmt_level) {

  // TODO: is it bad in these 2 cases to use directly the value of
  // the level_t enum to avoid the case switch and to read the MACRO?
  // The values are the same anyway
  ptrdiff_t rx_level_value;
  switch (rx_level) {
    case kI2cLevel1Byte:
      rx_level_value = I2C_FIFO_CTRL_RXILVL_VALUE_RXLVL1;
      break;
    case kI2cLevel4Byte:
      rx_level_value = I2C_FIFO_CTRL_RXILVL_VALUE_RXLVL4;
      break;
    case kI2cLevel8Byte:
      rx_level_value = I2C_FIFO_CTRL_RXILVL_VALUE_RXLVL8;
      break;
    case kI2cLevel16Byte:
      rx_level_value = I2C_FIFO_CTRL_RXILVL_VALUE_RXLVL16;
      break;
    case kI2cLevel30Byte:
      rx_level_value = I2C_FIFO_CTRL_RXILVL_VALUE_RXLVL30;
      break;
    default:
      return kI2cBadArg;
  }

  ptrdiff_t fmt_level_value;
  switch (fmt_level) {
    case kI2cLevel1Byte:
      fmt_level_value = I2C_FIFO_CTRL_FMTILVL_VALUE_FMTLVL1;
      break;
    case kI2cLevel4Byte:
      fmt_level_value = I2C_FIFO_CTRL_FMTILVL_VALUE_FMTLVL4;
      break;
    case kI2cLevel8Byte:
      fmt_level_value = I2C_FIFO_CTRL_FMTILVL_VALUE_FMTLVL8;
      break;
    case kI2cLevel16Byte:
      fmt_level_value = I2C_FIFO_CTRL_FMTILVL_VALUE_FMTLVL16;
      break;
    default:
      return kI2cBadArg;
  }

  i2c_peri->FIFO_CTRL = bitfield_write(i2c_peri->FIFO_CTRL, 
                                      I2C_FIFO_CTRL_RXILVL_MASK, 
                                      I2C_FIFO_CTRL_RXILVL_OFFSET, 
                                      rx_level_value);
  i2c_peri->FIFO_CTRL = bitfield_write(i2c_peri->FIFO_CTRL, 
                                      I2C_FIFO_CTRL_FMTILVL_MASK, 
                                      I2C_FIFO_CTRL_FMTILVL_OFFSET, 
                                      fmt_level_value);


  return kI2cOk;
}


i2c_result_t i2c_host_set_enabled(i2c_toggle_t state) {

  if (state != kI2cToggleEnabled && state != kI2cToggleDisabled)
  {
    return kI2cBadArg;
  }

  i2c_peri->CTRL = bitfield_write(i2c_peri->CTRL,
                                  BIT_MASK_1,
                                  I2C_CTRL_ENABLEHOST_BIT,
                                  state);

  return kI2cOk;
}

i2c_result_t i2c_override_set_enabled(i2c_toggle_t state) {

  if (state != kI2cToggleEnabled && state != kI2cToggleDisabled)
  {
    return kI2cBadArg;
  }

  i2c_peri->OVRD = bitfield_write(i2c_peri->OVRD,
                                  BIT_MASK_1,
                                  I2C_OVRD_TXOVRDEN_BIT,
                                  state);

  return kI2cOk;
}

i2c_result_t i2c_override_drive_pins(bool scl, bool sda) {

  i2c_peri->OVRD = bitfield_write(i2c_peri->OVRD,
                                  BIT_MASK_1,
                                  I2C_OVRD_SCLVAL_BIT,
                                  scl);
  i2c_peri->OVRD = bitfield_write(i2c_peri->OVRD,
                                  BIT_MASK_1,
                                  I2C_OVRD_SDAVAL_BIT,
                                  sda);
  return kI2cOk;
}

i2c_result_t i2c_override_sample_pins(uint16_t *scl_samples,
                                      uint16_t *sda_samples) {

  if (scl_samples != NULL)
  {
    bitfield_read(i2c_peri->VAL, I2C_VAL_SCL_RX_MASK, I2C_VAL_SCL_RX_OFFSET);
  }

  if (sda_samples != NULL)
  {
    bitfield_read(i2c_peri->VAL, I2C_VAL_SDA_RX_MASK, I2C_VAL_SDA_RX_OFFSET);
  }

  return kI2cOk;
}


i2c_result_t i2c_get_fifo_levels(uint8_t *fmt_fifo_level,
                                  uint8_t *rx_fifo_level) {

  if (fmt_fifo_level != NULL)
  {
    *fmt_fifo_level = bitfield_read(i2c_peri->FIFO_STATUS,
                                    I2C_FIFO_STATUS_FMTLVL_MASK,
                                    I2C_FIFO_STATUS_FMTLVL_OFFSET);
  }

  if (rx_fifo_level != NULL)
  {
    *rx_fifo_level = bitfield_read(i2c_peri->FIFO_STATUS,
                                    I2C_FIFO_STATUS_RXLVL_MASK,
                                    I2C_FIFO_STATUS_RXLVL_OFFSET);
  }

  return kI2cOk;
}

i2c_result_t i2c_read_byte(uint8_t *byte) {

  if (byte != NULL)
  {
    *byte = bitfield_read(i2c_peri->RDATA, I2C_RDATA_RDATA_MASK, I2C_RDATA_RDATA_OFFSET);
  }

  return kI2cOk;
}

i2c_result_t i2c_write_byte_raw(uint8_t byte, i2c_fmt_flags_t flags) {

  // Validate that "write only" flags and "read only" flags are not set
  // simultaneously.
  bool has_write_flags = flags.start || flags.stop || flags.suppress_nak_irq;
  bool has_read_flags = flags.read || flags.read_cont;
  if (has_write_flags && has_read_flags) {
    return kI2cBadArg;
  }

  // Also, read_cont requires read.
  if (flags.read_cont && !flags.read) {
    return kI2cBadArg;
  }

  /*
    Here I cannot perform every bitfield write directly in the register (using the
    `i2c_peri` struct) because each write counts as a single entry (so it would
    be counted as 6 entries instead of 1). 
  */
  uint32_t fmt_entry = 0;
  fmt_entry = i2c_peri->FDATA;
  fmt_entry = bitfield_write(fmt_entry, I2C_FDATA_FBYTE_MASK, I2C_FDATA_FBYTE_OFFSET, byte);
  fmt_entry = bitfield_write(fmt_entry, BIT_MASK_1, I2C_FDATA_START_BIT, flags.start);
  fmt_entry = bitfield_write(fmt_entry, BIT_MASK_1, I2C_FDATA_STOP_BIT, flags.stop);
  fmt_entry = bitfield_write(fmt_entry, BIT_MASK_1, I2C_FDATA_READ_BIT, flags.read);
  fmt_entry = bitfield_write(fmt_entry, BIT_MASK_1, I2C_FDATA_RCONT_BIT, flags.read_cont);
  fmt_entry = bitfield_write(fmt_entry, BIT_MASK_1, I2C_FDATA_NAKOK_BIT, flags.suppress_nak_irq);
  i2c_peri->FDATA = fmt_entry;
  

  return kI2cOk;
}

i2c_result_t i2c_write_byte(uint8_t byte, i2c_fmt_t code, bool suppress_nak_irq) {

  // Validate that `suppress_nak_irq` has not been mixed with an Rx code.
  if (suppress_nak_irq) {
    switch (code) {
      case kI2cFmtRx:
      case kI2cFmtRxContinue:
      case kI2cFmtRxStop:
        return kI2cBadArg;
      default:
        break;
    }
  }

  // Convert the format code into flags.
  i2c_fmt_flags_t flags = {.suppress_nak_irq = suppress_nak_irq};
  switch (code) {
    case kI2cFmtStart:
      flags.start = true;
      break;
    case kI2cFmtTx:
      break;
    case kI2cFmtTxStop:
      flags.stop = true;
      break;
    case kI2cFmtRx:
      flags.read = true;
      break;
    case kI2cFmtRxContinue:
      flags.read = true;
      flags.read_cont = true;
      break;
    case kI2cFmtRxStop:
      flags.read = true;
      flags.stop = true;
      break;
    default:
      return kI2cBadArg;
  }

  return i2c_write_byte_raw(byte, flags);
}


/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/


uint16_t round_up_divide(uint32_t a, uint32_t b) {
  if (a == 0) {
    return 0;
  }

  return ((a - 1) / b) + 1;
}


i2c_config_t default_timing_for_speed(i2c_speed_t speed,
                                                 uint32_t clock_period_nanos) {
  // All values are given in nanoseconds
  switch (speed) {
    case kI2cSpeedStandard:
      return (i2c_config_t){
          .scl_time_high_cycles = round_up_divide(4000, clock_period_nanos),
          .scl_time_low_cycles = round_up_divide(4700, clock_period_nanos),
          .start_signal_setup_cycles =
              round_up_divide(4700, clock_period_nanos),
          .start_signal_hold_cycles = round_up_divide(4000, clock_period_nanos),
          .data_signal_setup_cycles = round_up_divide(250, clock_period_nanos),
          .data_signal_hold_cycles = 0,
          .stop_signal_setup_cycles = round_up_divide(4000, clock_period_nanos),
          .stop_signal_hold_cycles = round_up_divide(4700, clock_period_nanos),
      };
    case kI2cSpeedFast:
      return (i2c_config_t){
          .scl_time_high_cycles = round_up_divide(600, clock_period_nanos),
          .scl_time_low_cycles = round_up_divide(1300, clock_period_nanos),
          .start_signal_setup_cycles = round_up_divide(600, clock_period_nanos),
          .start_signal_hold_cycles = round_up_divide(600, clock_period_nanos),
          .data_signal_setup_cycles = round_up_divide(100, clock_period_nanos),
          .data_signal_hold_cycles = 0,
          .stop_signal_setup_cycles = round_up_divide(600, clock_period_nanos),
          .stop_signal_hold_cycles = round_up_divide(1300, clock_period_nanos),
      };
    case kI2cSpeedFastPlus:
      return (i2c_config_t){
          .scl_time_high_cycles = round_up_divide(260, clock_period_nanos),
          .scl_time_low_cycles = round_up_divide(500, clock_period_nanos),
          .start_signal_setup_cycles = round_up_divide(260, clock_period_nanos),
          .start_signal_hold_cycles = round_up_divide(260, clock_period_nanos),
          .data_signal_setup_cycles = round_up_divide(50, clock_period_nanos),
          .data_signal_hold_cycles = 0,
          .stop_signal_setup_cycles = round_up_divide(260, clock_period_nanos),
          .stop_signal_hold_cycles = round_up_divide(500, clock_period_nanos),
      };
    default:
      return (i2c_config_t){0};
  }
}


// bool irq_index(i2c_irq_t irq, bitfield_bit32_index_t *bit_index)
// {
//   // // Check if the irq type is among the possible ones
//   // if (irq < 0 || irq > kI2cNIrqTypes)
//   // {
//   //   return false;
//   // } else {
//   //   return true;
//   // }

//   switch (irq) 
//   {
//     case kI2cIrqFmtWatermarkUnderflow:
//     *bit_index = I2C_INTR_COMMON_FMT_WATERMARK_BIT;
//     break;
//     case kI2cIrqRxWatermarkOverflow:
//       *bit_index = I2C_INTR_COMMON_RX_WATERMARK_BIT;
//       break;
//     case kI2cIrqFmtFifoOverflow:
//       *bit_index = I2C_INTR_COMMON_FMT_OVERFLOW_BIT;
//       break;
//     case kI2cIrqRxFifoOverflow:
//       *bit_index = I2C_INTR_COMMON_RX_OVERFLOW_BIT;
//       break;
//     case kI2cIrqNak:
//       *bit_index = I2C_INTR_COMMON_NAK_BIT;
//       break;
//     case kI2cIrqSclInterference:
//       *bit_index = I2C_INTR_COMMON_SCL_INTERFERENCE_BIT;
//       break;
//     case kI2cIrqSdaInterference:
//       *bit_index = I2C_INTR_COMMON_SDA_INTERFERENCE_BIT;
//       break;
//     case kI2cIrqClockStretchTimeout:
//       *bit_index = I2C_INTR_COMMON_STRETCH_TIMEOUT_BIT;
//       break;
//     case kI2cIrqSdaUnstable:
//       *bit_index = I2C_INTR_COMMON_SDA_UNSTABLE_BIT;
//       break;
//     case kI2cIrqTransComplete:
//       *bit_index = I2C_INTR_STATE_TRANS_COMPLETE_BIT;
//       break;
//     case kI2cIrqTxEmpty:
//       *bit_index = I2C_INTR_STATE_TX_EMPTY_BIT;
//       break;
//     case kI2cIrqTxNonEmpty:
//       *bit_index = I2C_INTR_STATE_TX_NONEMPTY_BIT;
//       break;
//     case kI2cIrqTxOverflow:
//       *bit_index = I2C_INTR_STATE_TX_OVERFLOW_BIT;
//       break;
//     case kI2cIrqAcqOverflow:
//       *bit_index = I2C_INTR_STATE_ACQ_OVERFLOW_BIT;
//       break;
//     case kI2cIrqAckStop:
//       *bit_index = I2C_INTR_STATE_ACK_STOP_BIT;
//       break;
//     case kI2cIrqHostTimeout:
//       *bit_index = I2C_INTR_STATE_HOST_TIMEOUT_BIT;
//       break;
//     default:
//       return false;
//   }

//   return true;
// }

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
