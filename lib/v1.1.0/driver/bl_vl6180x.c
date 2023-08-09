//==============================================================================
// bl_vl6180x.c
// driver for (STM) VL6180X Time-of-Flight sensor
//
// Created by Hugo Pristauz on 2022-Sep-29
// Copyright © 2022 Bluenetics. All rights reserved.
//==============================================================================

  #include "bluccino.h"
  #include "bl_gpio.h"
	#include "bl_vl6180x.h"

//==============================================================================
// logging shorthands & PMI definition
//==============================================================================

  #define WHO  "bl_vl6180x:"      // who is logging?

  #define LOG                     LOG_VL6180X
  #define LOGO(lvl,col,o,val)     LOGO_VL6180X(lvl,col WHO,o,val)
  #define LOG0(lvl,col,o,val)     LOGO_VL6180X(lvl,col,o,val)

  #define PMI  bl_vl6180x         // public module interface
  BL_PMI(PMI)

//==============================================================================
// macro definitions
//==============================================================================

  #define BYTE(val,i)   BL_BYTE(val,i)

//==============================================================================
// GPIO assignment
//==============================================================================

  #define VL6180_I2C_ADDR	0x29         // VL6180x i2c address
  #define LABEL_I2C       "I2C_0"

//static GP_dev *i2c = DEVICE_DT_GET(DT_NODELABEL(i2c0));
  static GP_dev *i2c;

	static void init_i2c_device(void)
	{
  	i2c = bl_dev(LABEL_I2C);
	}

//==============================================================================
// local variables
// - RANGE_SCALER values for 1x, 2x, 3x scaling
// - see STSW-IMG003 core/src/vl6180x_api.c (ScalerLookUP[])
//==============================================================================

  static uint16_t const scaler_values[] = {0, 253, 127, 84};

  static uint8_t scaling = 0;
  static uint8_t ptp_offset = 0;
  static uint16_t io_timeout = 0;

  static bool did_timeout = false;

//==============================================================================
// define VL6180X register addresses
//==============================================================================

  enum regAddr
  {
    IDENTIFICATION__MODEL_ID              = 0x000,
    IDENTIFICATION__MODEL_REV_MAJOR       = 0x001,
    IDENTIFICATION__MODEL_REV_MINOR       = 0x002,
    IDENTIFICATION__MODULE_REV_MAJOR      = 0x003,
    IDENTIFICATION__MODULE_REV_MINOR      = 0x004,
    IDENTIFICATION__DATE_HI               = 0x006,
    IDENTIFICATION__DATE_LO               = 0x007,
    IDENTIFICATION__TIME                  = 0x008, // 16-bit
    SYSTEM__MODE_GPIO0                    = 0x010,
    SYSTEM__MODE_GPIO1                    = 0x011,
    SYSTEM__HISTORY_CTRL                  = 0x012,
    SYSTEM__INTERRUPT_CONFIG_GPIO         = 0x014,
    SYSTEM__INTERRUPT_CLEAR               = 0x015,
    SYSTEM__FRESH_OUT_OF_RESET            = 0x016,
    SYSTEM__GROUPED_PARAMETER_HOLD        = 0x017,
    SYSRANGE__START                       = 0x018,
    SYSRANGE__THRESH_HIGH                 = 0x019,
    SYSRANGE__THRESH_LOW                  = 0x01A,
    SYSRANGE__INTERMEASUREMENT_PERIOD     = 0x01B,
    SYSRANGE__MAX_CONVERGENCE_TIME        = 0x01C,
    SYSRANGE__CROSSTALK_COMPENSATION_RATE = 0x01E, // 16-bit
    SYSRANGE__CROSSTALK_VALID_HEIGHT      = 0x021,
    SYSRANGE__EARLY_CONVERGENCE_ESTIMATE  = 0x022, // 16-bit
    SYSRANGE__PART_TO_PART_RANGE_OFFSET   = 0x024,
    SYSRANGE__RANGE_IGNORE_VALID_HEIGHT   = 0x025,
    SYSRANGE__RANGE_IGNORE_THRESHOLD      = 0x026, // 16-bit
    SYSRANGE__MAX_AMBIENT_LEVEL_MULT      = 0x02C,
    SYSRANGE__RANGE_CHECK_ENABLES         = 0x02D,
    SYSRANGE__VHV_RECALIBRATE             = 0x02E,
    SYSRANGE__VHV_REPEAT_RATE             = 0x031,
    SYSALS__START                         = 0x038,
    SYSALS__THRESH_HIGH                   = 0x03A,
    SYSALS__THRESH_LOW                    = 0x03C,
    SYSALS__INTERMEASUREMENT_PERIOD       = 0x03E,
    SYSALS__ANALOGUE_GAIN                 = 0x03F,
    SYSALS__INTEGRATION_PERIOD            = 0x040,
    RESULT__RANGE_STATUS                  = 0x04D,
    RESULT__ALS_STATUS                    = 0x04E,
    RESULT__INTERRUPT_STATUS_GPIO         = 0x04F,
    RESULT__ALS_VAL                       = 0x050, // 16-bit
    RESULT__HISTORY_BUFFER_0              = 0x052, // 16-bit
    RESULT__HISTORY_BUFFER_1              = 0x054, // 16-bit
    RESULT__HISTORY_BUFFER_2              = 0x056, // 16-bit
    RESULT__HISTORY_BUFFER_3              = 0x058, // 16-bit
    RESULT__HISTORY_BUFFER_4              = 0x05A, // 16-bit
    RESULT__HISTORY_BUFFER_5              = 0x05C, // 16-bit
    RESULT__HISTORY_BUFFER_6              = 0x05E, // 16-bit
    RESULT__HISTORY_BUFFER_7              = 0x060, // 16-bit
    RESULT__RANGE_VAL                     = 0x062,
    RESULT__RANGE_RAW                     = 0x064,
    RESULT__RANGE_RETURN_RATE             = 0x066, // 16-bit
    RESULT__RANGE_REFERENCE_RATE          = 0x068, // 16-bit
    RESULT__RANGE_RETURN_SIGNAL_COUNT     = 0x06C, // 32-bit
    RESULT__RANGE_REFERENCE_SIGNAL_COUNT  = 0x070, // 32-bit
    RESULT__RANGE_RETURN_AMB_COUNT        = 0x074, // 32-bit
    RESULT__RANGE_REFERENCE_AMB_COUNT     = 0x078, // 32-bit
    RESULT__RANGE_RETURN_CONV_TIME        = 0x07C, // 32-bit
    RESULT__RANGE_REFERENCE_CONV_TIME     = 0x080, // 32-bit
    RANGE_SCALER                          = 0x096, // 16-bit - see STSW-IMG003 core/inc/vl6180x_def.h
    READOUT__AVERAGING_SAMPLE_PERIOD      = 0x10A,
    FIRMWARE__BOOTUP                      = 0x119,
    FIRMWARE__RESULT_SCALER               = 0x120,
    I2C_SLAVE__DEVICE_ADDRESS             = 0x212,
    INTERLEAVED_MODE__ENABLE              = 0x2A3,
  };

//==============================================================================
// i2c: write bytes
//==============================================================================

	 static int write_bytes(const struct device *i2c, uint16_t addr,
			       uint8_t *data, uint32_t num_bytes)
	 {
		uint8_t wr_addr[2];
		struct i2c_msg msgs[2];

		/* VL6180 address */
		wr_addr[0] = (addr >> 8) & 0xFF;
		wr_addr[1] = addr & 0xFF;

		/* Setup I2C messages */

		/* Send the address to write to */
		msgs[0].buf = wr_addr;
		msgs[0].len = 2U;
		msgs[0].flags = I2C_MSG_WRITE;

		/* Data to be written, and STOP after this. */
		msgs[1].buf = data;
		msgs[1].len = num_bytes;
		msgs[1].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

		return i2c_transfer(i2c, &msgs[0], 2, VL6180_I2C_ADDR);
	 }

//==============================================================================
// i2c: read bytes
//==============================================================================

	 static int read_bytes(const struct device *i2c, uint16_t addr,
	 		      uint8_t *data, uint32_t num_bytes)
	 {
		uint8_t wr_addr[2];
		struct i2c_msg msgs[2];

		  // now try to read back from VL6180
		  // VL6180 address

		wr_addr[0] = (addr >> 8) & 0xFF;
		wr_addr[1] = addr & 0xFF;

		  // setup I2C messages
		  // send the address to read from

		msgs[0].buf = wr_addr;
		msgs[0].len = 2U;
		msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

		  // read from device, STOP after this

		msgs[1].buf = data;
		msgs[1].len = num_bytes;
		msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;
		return i2c_transfer(i2c, &msgs[0], 2, VL6180_I2C_ADDR);
	}

//==============================================================================
// writes an 8-bit register
//==============================================================================

  static void write_reg_8bit(BL_wd reg, BL_byte val)
  {
    int err = write_bytes(i2c, reg, &val, 1);
    bl_err(err,"writing to vl6180x");
  }

//==============================================================================
// writes a 16-bit register
//==============================================================================

  static void write_reg_16bit(BL_wd reg, uint16_t val)
  {
    BL_byte buf[2] = {BYTE(val,1), BYTE(val,0)};

    int err = write_bytes(i2c, reg, buf, sizeof(buf));
    bl_err(err,"writing to vl6180x");
  }

//==============================================================================
// writes a 32-bit register
//==============================================================================

  static void write_reg_32bit(BL_wd reg, uint32_t val)
  {
    uint8_t buf[4] = { BYTE(val,3), BYTE(val,2), BYTE(val,1), BYTE(val,0)};

    int err = write_bytes(i2c, reg, buf, sizeof(buf));
    bl_err(err,"writing to vl6180x");
  }

//==============================================================================
// reads a 8-bit register
//==============================================================================

	static uint8_t read_reg_8bit(uint16_t reg)
	{
	  uint8_t value;

	  read_bytes(i2c, reg, &value, 1);

	  return value;
	}

//==============================================================================
// reads a 16-bit register
//==============================================================================

  static uint16_t read_reg_16bit(uint16_t reg)
  {
    uint16_t value;
    uint8_t value16[2]={0};

    read_bytes(i2c, reg, value16, 2);

    value  = (uint16_t)value16[0] << 8; // value high byte
    value |= (uint16_t)value16[1];
    return value;
  }

//==============================================================================
// reads a 32-bit register
//==============================================================================

	static uint32_t read_reg_32bit(uint16_t reg)
	{
	  uint32_t value;
	  uint8_t value32[4]={0};

    read_bytes(i2c, reg, value32, 4);

    value  = (uint32_t)value32[0] << 24; // value highest byte
	  value |= (uint32_t)value32[1] << 16;
	  value |= (uint16_t)value32[2] << 8;
	  value |=           value32[3];       // value lowest byte
	  return value;
	}

//==============================================================================
// set/get timeout settings
//==============================================================================

    static inline void set_timeout(uint16_t timeout)
    {
      io_timeout = timeout;
    }

    static inline uint16_t get_timeout(void)
    {
      return io_timeout;
    }

//==============================================================================
// i2c: init I2C
//==============================================================================

 static int i2c_init(void)
 {
   LOG(4,BL_B"init I2C device for VL6180X");

   init_i2c_device();

   if( i2c_configure(i2c, I2C_SPEED_SET(I2C_SPEED_FAST)))
     return bl_err(-1,"I2C config failed");

	 if (!device_is_ready(i2c))
	   return bl_err(-2,"I2C device not ready");

   if (read_reg_8bit(IDENTIFICATION__MODEL_ID) != 0xB4)
     return bl_err(-3,"VL6180X not found");

   return 0;
 }

//==============================================================================
// hhelper: init registers
//==============================================================================

  static void register_init(void)
  {
    write_reg_8bit(0x207, 0x01);
    write_reg_8bit(0x208, 0x01);
    write_reg_8bit(0x096, 0x00);
    write_reg_8bit(0x097, 0xFD); // RANGE_SCALER = 253
    write_reg_8bit(0x0E3, 0x01);
    write_reg_8bit(0x0E4, 0x03);
    write_reg_8bit(0x0E5, 0x02);
    write_reg_8bit(0x0E6, 0x01);
    write_reg_8bit(0x0E7, 0x03);
    write_reg_8bit(0x0F5, 0x02);
    write_reg_8bit(0x0D9, 0x05);
    write_reg_8bit(0x0DB, 0xCE);
    write_reg_8bit(0x0DC, 0x03);
    write_reg_8bit(0x0DD, 0xF8);
    write_reg_8bit(0x09F, 0x00);
    write_reg_8bit(0x0A3, 0x3C);
    write_reg_8bit(0x0B7, 0x00);
    write_reg_8bit(0x0BB, 0x3C);
    write_reg_8bit(0x0B2, 0x09);
    write_reg_8bit(0x0CA, 0x09);
    write_reg_8bit(0x198, 0x01);
    write_reg_8bit(0x1B0, 0x17);
    write_reg_8bit(0x1AD, 0x00);
    write_reg_8bit(0x0FF, 0x05);
    write_reg_8bit(0x100, 0x05);
    write_reg_8bit(0x199, 0x05);
    write_reg_8bit(0x1A6, 0x1B);
    write_reg_8bit(0x1AC, 0x3E);
    write_reg_8bit(0x1A7, 0x1F);
    write_reg_8bit(0x030, 0x00);
    write_reg_8bit(SYSTEM__FRESH_OUT_OF_RESET, 0);
  }

//==============================================================================
// Set range scaling factor
// - the sensor uses 1x scaling by default, giving range measurements in units
//   of mm
// - increasing the scaling to 2x or 3x makes it give raw values in units of
//   2 mm or 3 mm instead
// - in other words, a bigger scaling factor increases the sensor's potential
//   maximum range but reduces its resolution.
// - implemented using ST's VL6180X API as a reference (STSW-IMG003)
// - see VL6180x_UpscaleSetScaling() in vl6180x_api.c.
//==============================================================================

  void set_scaling(uint8_t new_scaling)
  {
      // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

    uint8_t const DefaultCrosstalkValidHeight = 20;

      // do nothing if scaling value is invalid

    if (new_scaling < 1 || new_scaling > 3) return;

    scaling = new_scaling;
    write_reg_16bit(RANGE_SCALER, scaler_values[scaling]);

      // apply scaling on part-to-part offset

    write_reg_8bit(SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset / scaling);

      // apply scaling on CrossTalkValidHeight

    write_reg_8bit(SYSRANGE__CROSSTALK_VALID_HEIGHT,
      DefaultCrosstalkValidHeight / scaling);

      // this function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.
      // enable early convergence estimate only at 1x scaling

    uint8_t rce = read_reg_8bit(SYSRANGE__RANGE_CHECK_ENABLES);
    write_reg_8bit(SYSRANGE__RANGE_CHECK_ENABLES, (rce & 0xFE) | (scaling==1));
  }

//==============================================================================
// Configure some settings for the sensor's default behavior from AN4545 -
// "Recommended : Public registers" and "Optional: Public registers"
//
// Note that this function does not set up GPIO1 as an interrupt output as
// suggested, though you can do so by calling:
// writeReg(SYSTEM__MODE_GPIO1, 0x10);
//==============================================================================

  void configure_default()
  {
      // "Recommended : Public registers"
      // readout__averaging_sample_period = 48

    write_reg_8bit(READOUT__AVERAGING_SAMPLE_PERIOD, 0x30);

      // sysals__analogue_gain_light = 6 (ALS gain = 1 nominal, actually 1.01
      // according to table "Actual gain values" in datasheet)

    write_reg_8bit(SYSALS__ANALOGUE_GAIN, 0x46);

      // sysrange__vhv_repeat_rate = 255 (auto Very High Voltage temperature
      //  recalibration after every 255 range measurements)

    write_reg_8bit(SYSRANGE__VHV_REPEAT_RATE, 0xFF);

      // sysals__integration_period = 99 (100 ms)

    write_reg_16bit(SYSALS__INTEGRATION_PERIOD, 0x0063);

      // sysrange__vhv_recalibrate = 1 (manually trigger a VHV recalibration)

    write_reg_8bit(SYSRANGE__VHV_RECALIBRATE, 0x01);

      // "Optional: Public registers"
      // sysrange__intermeasurement_period = 9 (100 ms)

    write_reg_8bit(SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09);

      // sysals__intermeasurement_period = 49 (500 ms)

    write_reg_8bit(SYSALS__INTERMEASUREMENT_PERIOD, 0x31);

      // als_int_mode = 4 (ALS new sample ready interrupt);
      // range_int_mode = 4 (range new sample ready interrupt)

    write_reg_8bit(SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24);

      // Reset other settings to power-on defaults
      // sysrange__max_convergence_time = 49 (49 ms)

    write_reg_8bit(SYSRANGE__MAX_CONVERGENCE_TIME, 0x31);

      // disable interleaved mode

    write_reg_8bit(INTERLEAVED_MODE__ENABLE, 0);

      // reset range scaling factor to 1x

    set_scaling(1);
  }

//==============================================================================
// Returns a range reading when continuous mode is activated
// (read_dist_single() also calls this function after starting a single-shot
// range measurement)
//==============================================================================

  uint8_t read_dist_cont()
  {
    uint16_t millis_start = k_uptime_get_32();

    while ((read_reg_8bit(RESULT__INTERRUPT_STATUS_GPIO) & 0x04) == 0)
    {
      if (io_timeout > 0 && ((uint16_t)k_uptime_get_32() - millis_start) > io_timeout)
      {
        did_timeout = true;
        return 255;
      }
    }

    uint8_t range = read_reg_8bit(RESULT__RANGE_VAL);
    write_reg_8bit(SYSTEM__INTERRUPT_CLEAR, 0x01);
    return range;
  }

//==============================================================================
// Returns an ambient light reading when continuous mode is activated
// (readAmbientSingle() also calls this function after starting a single-shot
// ambient light measurement)
//==============================================================================

  uint16_t read_ambient_cont()
  {
    uint16_t millis_start = k_uptime_get_32();
    while ((read_reg_8bit(RESULT__INTERRUPT_STATUS_GPIO) & 0x20) == 0)
    {
      if (io_timeout > 0 && ((uint16_t)k_uptime_get_32() - millis_start) > io_timeout)
      {
        did_timeout = true;
        return 0;
      }
    }

    uint16_t ambient = read_reg_16bit(RESULT__ALS_VAL);
    write_reg_8bit(SYSTEM__INTERRUPT_CLEAR, 0x02);
    return ambient;
  }

//==============================================================================
// perform single-shot distance measurement
//==============================================================================

  uint8_t read_dist_single()
  {
    write_reg_8bit(SYSRANGE__START, 0x01);
    return read_dist_cont();
  }

//==============================================================================
// Performs a single-shot ambient light measurement
//==============================================================================

  uint16_t readAmbientSingle()
  {
    write_reg_8bit(SYSALS__START, 0x01);
    return read_ambient_cont();
  }

//==============================================================================
// Starts continuous ranging measurements with the given period in ms
// (10 ms resolution; defaults to 100 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section "Continuous mode limits" in the datasheet
// for details.
//==============================================================================

void startRangeContinuous(uint16_t period)
{
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = BL_SAT(period_reg, 0, 254);

  write_reg_8bit(SYSRANGE__INTERMEASUREMENT_PERIOD, period_reg);
  write_reg_8bit(SYSRANGE__START, 0x03);
}

//==============================================================================
// Starts continuous ambient light measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section "Continuous mode limits" in the datasheet
// for details.
//==============================================================================

void startAmbientContinuous(uint16_t period)
{
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = BL_SAT(period_reg, 0, 254);

  write_reg_8bit(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
  write_reg_8bit(SYSALS__START, 0x03);
}

//==============================================================================
// Starts continuous interleaved measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified). In this mode, each
// ambient light measurement is immediately followed by a range measurement.
//
// The datasheet recommends using this mode instead of running "range and ALS
// continuous modes simultaneously (i.e. asynchronously)".
//
// The period must be greater than the time it takes to perform both
// measurements. See section "Continuous mode limits" in the datasheet
// for details.
//==============================================================================

void startInterleavedContinuous(uint16_t period)
{
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = BL_SAT(period_reg, 0, 254);

  write_reg_8bit(INTERLEAVED_MODE__ENABLE, 1);
  write_reg_8bit(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
  write_reg_8bit(SYSALS__START, 0x03);
}

//==============================================================================
// Stops continuous mode. This will actually start a single measurement of range
// and/or ambient light if continuous mode is not active, so it's a good idea to
// wait a few hundred ms after calling this function to let that complete
// before starting continuous mode again or taking a reading.
//==============================================================================

  void stopContinuous()
  {
    write_reg_8bit(SYSRANGE__START, 0x01);
    write_reg_8bit(SYSALS__START, 0x01);
    write_reg_8bit(INTERLEAVED_MODE__ENABLE, 0);
  }

//==============================================================================
// did a timeout occur in one of the read functions since last call?
//==============================================================================

  bool timeout_occurred()
  {
    bool tmp = did_timeout;
    did_timeout = false;
    return tmp;
  }

//==============================================================================
// helper: read distance in single mode
//==============================================================================

  static uint16_t read_dist_single_mm(void)
  {
    return (uint16_t)scaling * read_dist_single();
  }

//==============================================================================
// helper: read distance in continuous mode
//==============================================================================

  static uint16_t read_dist_cont_mm(void)
  {
    return (uint16_t)scaling * read_dist_cont();
  }

//==============================================================================
// handler: [TOF:DIST] read distance
// - returns (positive) distance in mm or err=-1 in case of timeout
//==============================================================================

  static int tof_dist(BL_ob *o, int val)
  {
    int dist = read_dist_single_mm();

	  if (timeout_occurred())
      return -1;
    else
      return dist;
  }

//==============================================================================
// handler: [SYS:INIT (cb)] system init
//==============================================================================

  static int sys_init(BL_ob *o, int val)
  {
    LOG(4,BL_B "init bl_vl6180x");

    (void)read_dist_cont_mm;           // don't warn
    (void)read_reg_32bit;              // don't warn
    (void)write_reg_32bit;             // don't warn

	  int err = i2c_init();
    if (err)
      return bl_err(-2,"cannot init I2C");

      // store part-to-part range offset so it can be adjusted if scaling
      // is changed

    ptp_offset = read_reg_8bit(SYSRANGE__PART_TO_PART_RANGE_OFFSET);

    if (read_reg_8bit(SYSTEM__FRESH_OUT_OF_RESET) == 1)
    {
      register_init();
      scaling = 1;

      configure_default();
      set_timeout(500);
      return 0;
    }

      // otherwise sensor has already been initialized, so try to get scaling
      // settings by reading registers

    uint16_t s = read_reg_16bit(RANGE_SCALER);

    if (s == scaler_values[3])
      scaling = 3;
    else if (s == scaler_values[2])
      scaling = 2;
    else
      scaling = 1;

    // Adjust the part-to-part range offset value read earlier to account for
    // existing scaling. If the sensor was already in 2x or 3x scaling mode,
    // precision will be lost calculating the original (1x) offset, but this can
    // be resolved by resetting the sensor and Arduino again.

    ptp_offset *= scaling;

    configure_default();
    set_timeout(500);

    return 0;
  }

//==============================================================================
// PMI
//==============================================================================
//
// (H) := bl_hw;  (U) := bl_up;
//
//                  +--------------------+
//                  |     bl_vl6180x     |
//                  +--------------------+
//                  |        SYS:        | SYS input interface
// (H)->     INIT ->|        (cb)        | init module, store callback
//                  +--------------------+
//                  |        TOF:        | TOF input interface
// (H)->     DIST ->|                    | read distance (in millimeters)
//                  +--------------------+
//
//==============================================================================

  int bl_vl6180x(BL_ob *o, int val)
  {
    static BL_oval U = NULL;

    switch (bl_id(o))
    {
      case BL_ID(_SYS,INIT_):
        U = bl_cb(o,(U),WHO"(U)");     // store callback
        return sys_init(o,val);

      case BL_ID(_TOF,DIST_):
        return tof_dist(o,val);
/*
      case _BL_ID(_TOF,DATA_):
        return bl_out(o,val,(U));
*/
      default:
        return BL_VOID;
    }
  }
