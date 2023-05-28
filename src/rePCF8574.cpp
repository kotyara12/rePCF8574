#include "rePCF8574.h"
#include "reI2C.h"
#include "rLog.h"
#include "reEvents.h"

static const char* logTAG = "PCF8574";

#define ERR_CHECK(err, str) if (err != ESP_OK) { \
  rlog_e(logTAG, "%s: #%d (%s)", str, err, esp_err_to_name(err)); \
  return false; \
};

#define PCF8574_TIMEOUT 1000

rePCF8574::rePCF8574(i2c_port_t numI2C, uint8_t addrI2C, cb_gpio_change_t callback)
{
  _numI2C = numI2C; 
  _addrI2C = addrI2C;
  _callback = callback;
}

rePCF8574::~rePCF8574()
{
}

// -----------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------- I2C ---------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

bool rePCF8574::read8(uint8_t* data)
{
  esp_err_t err = readI2C(_numI2C, _addrI2C, nullptr, 0, data, 1, 0, PCF8574_TIMEOUT); 
  if (err == ESP_OK) {
    // rlog_d(logTAG, "Read from PCF8574 %d.0x%2x: 0x%2x", _numI2C, _addrI2C, *data);
    return true;
  };
  rlog_e(logTAG, "Failed to read PCF8574 %d.0x%2x: %d (%s)", _numI2C, _addrI2C, err, esp_err_to_name(err));
  return false;
}

bool rePCF8574::write8(uint8_t data)
{
  esp_err_t err = writeI2C(_numI2C, _addrI2C, nullptr, 0, &data, 1, PCF8574_TIMEOUT);
  if (err == ESP_OK) {
    // rlog_d(logTAG, "Write to PCF8574 %d.0x%2x: 0x%2x", _numI2C, _addrI2C, data);
    return true;
  };
  rlog_e(logTAG, "Failed to write PCF8574 %d.0x%2x: %d (%s)", _numI2C, _addrI2C, err, esp_err_to_name(err));
  return false;
}

uint8_t rePCF8574::get(bool read_i2c)
{
  if (read_i2c) {
    read8(&_data);
  };
  return _data;
}

bool rePCF8574::set(uint8_t data)
{
  if (write8(data)) {
    _data = data;
    return true;
  };
  return false;
}

uint8_t rePCF8574::read(uint8_t pin, bool read_i2c) 
{
  if (read_i2c) {
    if (!read8(&_data)) {
      return PCF8574_READ_ERROR;
    };
  };
  return (_data & (1 << pin)) > 0;
}

bool rePCF8574::write(uint8_t pin, const uint8_t value) 
{
  if (value == 0) {
    _data &= ~(1 << pin);
  } else {
    _data |= (1 << pin);
  };
  return write8(_data);
}

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- Changes detection --------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

uint8_t rePCF8574::update(bool reset)
{
  // Read new data from i2c ( reset INT!!! )
  uint8_t new_data = 0;
  if (!read8(&new_data)) {
    return 0;
  };
  
  // Compare previous and new data
  uint8_t changes = _data ^ new_data;
  // rlog_d(logTAG, "Update: prev data: 0x%02X, new data: 0x%02X, changed: 0x%02X", _data, new_data, changes);
  if (reset) {
    write8(_data);
  } else {
    _data = new_data;
  };

  if (changes != 0) {
    // Prepare data for events
    gpio_data_t data;
    data.bus = (uint8_t)_numI2C + 1;
    data.address = _addrI2C;

    // Send events only for pins that have changed
    for (uint8_t i = 0; i < 8; i++) {
      if ((changes & (1 << i)) > 0) {
        data.pin = i;
        data.value = (uint8_t)((new_data & (1 << i)) > 0);
        eventLoopPost(RE_GPIO_EVENTS, RE_GPIO_CHANGE, &data, sizeof(data), portMAX_DELAY);
        if (_callback) {
          _callback((void*)this, data, 0);
        };
      };
    };
  };

  return changes;
}
