/***************************************************
  This is a library for the Adafruit PT100/P1000 RTD Sensor w/MAX31865

  Designed specifically to work with the Adafruit RTD Sensor
  ----> https://www.adafruit.com/products/3328

  This sensor uses SPI to communicate, 4 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_MAX31865.h"
#ifdef __AVR
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

#include <stdlib.h>

/**************************************************************************/
/*!
    @brief Create the interface object using software (bitbang) SPI
    @param spi_cs the SPI CS pin to use
    @param spi_mosi the SPI MOSI pin to use
    @param spi_miso the SPI MISO pin to use
    @param spi_clk the SPI clock pin to use
*/
/**************************************************************************/
//
Adafruit_MAX31865::Adafruit_MAX31865(int8_t spi_cs, int8_t spi_mosi,
                                     int8_t spi_miso, int8_t spi_clk)
    : spi_dev(spi_cs, spi_clk, spi_miso, spi_mosi, 1000000,
              SPI_BITORDER_MSBFIRST, SPI_MODE1) {}

/**************************************************************************/
/*!
    @brief Create the interface object using hardware SPI
    @param spi_cs the SPI CS pin to use along with the default SPI device
*/
/**************************************************************************/
Adafruit_MAX31865::Adafruit_MAX31865(int8_t spi_cs)
    : spi_dev(spi_cs, 1000000, SPI_BITORDER_MSBFIRST, SPI_MODE1) {}

/**************************************************************************/
/*!
    @brief Initialize the SPI interface and set the number of RTD wires used
    @param wires The number of wires in enum format. Can be MAX31865_2WIRE,
    MAX31865_3WIRE, or MAX31865_4WIRE
    @return True
*/
/**************************************************************************/
bool Adafruit_MAX31865::begin(max31865_numwires_t wires) {
  spi_dev.begin();

  setWires(wires);
  enableBias(false);
  autoConvert(false);
  clearFault();
  setState(STATE1); // added for asynchronous

  // Serial.print("config: ");
  // Serial.println(readRegister8(MAX31865_CONFIG_REG), HEX);
  return true;
}

/**************************************************************************/
/*!
    @brief Read the raw 8-bit FAULTSTAT register
    @return The raw unsigned 8-bit FAULT status register
*/
/**************************************************************************/
uint8_t Adafruit_MAX31865::readFault(void) {
  return readRegister8(MAX31865_FAULTSTAT_REG);
}

/**************************************************************************/
/*!
    @brief Clear all faults in FAULTSTAT
*/
/**************************************************************************/
void Adafruit_MAX31865::clearFault(void) {
  uint8_t t = readRegister8(MAX31865_CONFIG_REG);
  t &= ~0x2C;
  t |= MAX31865_CONFIG_FAULTSTAT;
  writeRegister8(MAX31865_CONFIG_REG, t);
}

/**************************************************************************/
/*!
    @brief Enable the bias voltage on the RTD sensor between readings
    @param b If true bias is enabled between readings, else disabled
*/
/**************************************************************************/
void Adafruit_MAX31865::enableBias(bool b) {
  uint8_t t = readRegister8(MAX31865_CONFIG_REG);
  if (b) {
    t |= MAX31865_CONFIG_BIAS; // enable bias
  } else {
    t &= ~MAX31865_CONFIG_BIAS; // disable bias
  }
  writeRegister8(MAX31865_CONFIG_REG, t);
  bias = b;
}

/**************************************************************************/
/*!
    @brief Whether we want to have continuous conversions (50/60 Hz)
    @param b If true, continuous conversion is enabled
*/
/**************************************************************************/
void Adafruit_MAX31865::autoConvert(bool b) {
  uint8_t t = readRegister8(MAX31865_CONFIG_REG);
  if (b) {
    t |= MAX31865_CONFIG_MODEAUTO; // enable continuous conversion
  } else {
    t &= ~MAX31865_CONFIG_MODEAUTO; // disable continuous conversion
  }
  writeRegister8(MAX31865_CONFIG_REG, t);
  if (b && !continuous) {
    if (filter50Hz) {
      delay(70);
    } else {
      delay(60);
    } 
  }
  continuous = b;
}

/**************************************************************************/
/*!
    @brief Whether we want filter out 50Hz noise or 60Hz noise
    @param b If true, 50Hz noise is filtered, else 60Hz(default)
*/
/**************************************************************************/

void Adafruit_MAX31865::enable50Hz(bool b) {
  uint8_t t = readRegister8(MAX31865_CONFIG_REG);
  if (b) {
    t |= MAX31865_CONFIG_FILT50HZ;
  } else {
    t &= ~MAX31865_CONFIG_FILT50HZ;
  }
  writeRegister8(MAX31865_CONFIG_REG, t);
  filter50Hz = b;
}

/**************************************************************************/
/*!
    @brief How many wires we have in our RTD setup, can be MAX31865_2WIRE,
    MAX31865_3WIRE, or MAX31865_4WIRE
    @param wires The number of wires in enum format
*/
/**************************************************************************/
void Adafruit_MAX31865::setWires(max31865_numwires_t wires) {
  uint8_t t = readRegister8(MAX31865_CONFIG_REG);
  if (wires == MAX31865_3WIRE) {
    t |= MAX31865_CONFIG_3WIRE;
  } else {
    // 2 or 4 wire
    t &= ~MAX31865_CONFIG_3WIRE;
  }
  writeRegister8(MAX31865_CONFIG_REG, t);
}

/**************************************************************************/
/*!
    @brief Read the temperature in C from the RTD through calculation of the
    resistance. Uses
   http://www.analog.com/media/en/technical-documentation/application-notes/AN709_0.pdf
   technique
    @param RTDnominal The 'nominal' resistance of the RTD sensor, usually 100
    or 1000
    @param refResistor The value of the matching reference resistor, usually
    430 or 4300
    @returns Temperature in C
*/
/**************************************************************************/
float Adafruit_MAX31865::temperature(float RTDnominal, float refResistor) {
  float Z1, Z2, Z3, Z4, Rt, temp;

  Rt = readRTD();
  Rt /= 32768;
  Rt *= refResistor;

  // Serial.print("\nResistance: "); Serial.println(Rt, 8);

  Z1 = -RTD_A;
  Z2 = RTD_A * RTD_A - (4 * RTD_B);
  Z3 = (4 * RTD_B) / RTDnominal;
  Z4 = 2 * RTD_B;

  temp = Z2 + (Z3 * Rt);
  temp = (sqrt(temp) + Z1) / Z4;

  if (temp >= 0)
    return temp;

  // ugh.
  Rt /= RTDnominal;
  Rt *= 100; // normalize to 100 ohm

  float rpoly = Rt;

  temp = -242.02;
  temp += 2.2228 * rpoly;
  rpoly *= Rt; // square
  temp += 2.5859e-3 * rpoly;
  rpoly *= Rt; // ^3
  temp -= 4.8260e-6 * rpoly;
  rpoly *= Rt; // ^4
  temp -= 2.8183e-8 * rpoly;
  rpoly *= Rt; // ^5
  temp += 1.5243e-10 * rpoly;

  return temp;
}

/**************************************************************************/
/*!
    @brief Read the raw 16-bit value from the RTD_REG in one shot mode
    @return The raw unsigned 16-bit value, NOT temperature!
*/
/**************************************************************************/
uint16_t Adafruit_MAX31865::readRTD(void) {
  clearFault();
  if (!continuous) {
    if (!bias) {
      uint8_t t = readRegister8(MAX31865_CONFIG_REG);
      t |= MAX31865_CONFIG_BIAS; // enable bias
      writeRegister8(MAX31865_CONFIG_REG, t);
      delay(10);
    }
    uint8_t t = readRegister8(MAX31865_CONFIG_REG);
    t |= MAX31865_CONFIG_1SHOT;
    writeRegister8(MAX31865_CONFIG_REG, t);
    if (filter50Hz) {
      delay(70);
    } else {
      delay(60);
    }  
  }

  uint16_t rtd = readRegister16(MAX31865_RTDMSB_REG);

  if (!bias) {  
    uint8_t t = readRegister8(MAX31865_CONFIG_REG);
    t &= ~MAX31865_CONFIG_BIAS; // disable bias
    writeRegister8(MAX31865_CONFIG_REG, t);
  }

  // remove fault
  rtd >>= 1;

  return rtd;
}

/**************************************************************************/
/*!
    @brief Read the temperature in C from the RTDAsync through calculation of the
    resistance. Uses
   http://www.analog.com/media/en/technical-documentation/application-notes/AN709_0.pdf
   technique
    @param RT, The measured resistance obtained using the function readRTDAsync()
    @param RTDnominal The 'nominal' resistance of the RTD sensor, usually 100
    or 1000
    @param refResistor The value of the matching reference resistor, usually
    430 or 4300
    @returns Temperature in C
*/
/**************************************************************************/
 float Adafruit_MAX31865::temperatureAsync(float Rt, float RTDnominal, float refResistor) {
  float Z1, Z2, Z3, Z4, temp;

  Rt /= 32768;
  Rt *= refResistor;

  Z1 = -RTD_A;
  Z2 = RTD_A * RTD_A - (4 * RTD_B);
  Z3 = (4 * RTD_B) / RTDnominal;
  Z4 = 2 * RTD_B;

  temp = Z2 + (Z3 * Rt);
  temp = (sqrt(temp) + Z1) / Z4;

  if (temp >= 0)
    return temp;

  // ugh.
  Rt /= RTDnominal;
  Rt *= 100; // normalize to 100 ohm

  float rpoly = Rt;

  temp = -242.02;
  temp += 2.2228 * rpoly;
  rpoly *= Rt; // square
  temp += 2.5859e-3 * rpoly;
  rpoly *= Rt; // ^3
  temp -= 4.8260e-6 * rpoly;
  rpoly *= Rt; // ^4
  temp -= 2.8183e-8 * rpoly;
  rpoly *= Rt; // ^5
  temp += 1.5243e-10 * rpoly;

  return temp;
}

 



/**************************************************************************/
/*!
    @brief Read the raw 16-bit value from the RTD_REG in one shot mode but asynchronously using a state machine, 
           and update the variable  rtd : The raw unsigned 16-bit value, NOT temperature!
    @return boolean value: false = conversion not finished, true = conversion finished, the RTD value has been measured
*/
/**************************************************************************/
bool Adafruit_MAX31865::readRTDAsync(uint16_t & rtd) {
  bool valueAvailable = false;
  uint32_t timeoutVbias = 10;
  uint32_t timeoutf50Hz = 75;
  uint32_t timeoutf60Hz = 65;

  switch (this->state) {
    case STATE1:
      clearFault();
      //enableBias(true);
      if (!bias) {
        uint8_t t = readRegister8(MAX31865_CONFIG_REG);
        t |= MAX31865_CONFIG_BIAS; // enable bias
        writeRegister8(MAX31865_CONFIG_REG, t);
      }
      chrono = millis();
      setState(STATE2);
      break;

    case STATE2:
      if (millis() - chrono >= timeoutVbias) {
        uint8_t t = readRegister8(MAX31865_CONFIG_REG);
        t |= MAX31865_CONFIG_1SHOT;
        writeRegister8(MAX31865_CONFIG_REG, t);
        chrono = millis();
        setState(STATE3);
      }
      break;

    case STATE3:
      if(filter50Hz){
        if (millis() - chrono >= timeoutf50Hz) {
          rtd = readRegister16(MAX31865_RTDMSB_REG);
          rtd >>= 1;        // remove fault
          setState(STATE1); // get ready for next time
          valueAvailable = true; // signal computation is done
        }
      }
      else{
        if (millis() - chrono >= timeoutf60Hz) {
          rtd = readRegister16(MAX31865_RTDMSB_REG);
          rtd >>= 1;        // remove fault
          setState(STATE1); // get ready for next time
          valueAvailable = true; // signal computation is done
        }
      }
      break;
  }
  return valueAvailable;
}

void Adafruit_MAX31865::setState(t_state new_state){
  this->state = new_state;
}


/**********************************************/

uint8_t Adafruit_MAX31865::readRegister8(uint8_t addr) {
  uint8_t ret = 0;
  readRegisterN(addr, &ret, 1);

  return ret;
}

uint16_t Adafruit_MAX31865::readRegister16(uint8_t addr) {
  uint8_t buffer[2] = {0, 0};
  readRegisterN(addr, buffer, 2);

  uint16_t ret = buffer[0];
  ret <<= 8;
  ret |= buffer[1];

  return ret;
}

void Adafruit_MAX31865::readRegisterN(uint8_t addr, uint8_t buffer[],
                                      uint8_t n) {
  addr &= 0x7F; // make sure top bit is not set

  spi_dev.write_then_read(&addr, 1, buffer, n);
}

void Adafruit_MAX31865::writeRegister8(uint8_t addr, uint8_t data) {
  addr |= 0x80; // make sure top bit is set

  uint8_t buffer[2] = {addr, data};
  spi_dev.write(buffer, 2);
}

//**** DEBUGGING****
#ifdef MAX31865_DEBUG_LIBRARY
  uint8_t  Adafruit_MAX31865:: debugConfigRegister(void){
    uint8_t  t = readRegister8(MAX31865_CONFIG_REG);
    //t |= MAX31865_CONFIG_MODEOFF;
    //t = 0x00;
    return t;
  }
#endif
