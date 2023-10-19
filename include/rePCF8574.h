/* 
   EN: Class for working with the PCF8574 port expander
   RU: Класс для работы с расширителем портов PCF8574
   --------------------------------------------------------------------------------
   (с) 2022 Разживин Александр | Razzhivin Alexander
   kotyara12@yandex.ru | https://kotyara12.ru | tg: @kotyara1971
*/

#ifndef __RE_PCF8574_H__
#define __RE_PCF8574_H__

#include <stdint.h>
#include <esp_err.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "rTypes.h"
#include "reGpio.h"

#define PCF8574_READ_ERROR 0x80

#ifdef __cplusplus
extern "C" {
#endif

class rePCF8574 {
  public:
    rePCF8574(i2c_port_t numI2C, uint8_t addrI2C, cb_gpio_change_t callback);
    ~rePCF8574();

    // Reading and writing data from the chip with changing the internal buffer
    uint8_t get(bool read_i2c = true);
    bool set(uint8_t data);

    // Reading and writing individual bits (stateful in internal buffer)
    uint8_t read(uint8_t pin, bool read_i2c = true);
    bool write(uint8_t pin, const uint8_t value);

    // Reread data from i2c. If changes are detected, events will be posted to the event loop. Returns the bits that have been changed
    uint8_t update(bool reset);
  private:
    i2c_port_t _numI2C = I2C_NUM_0; 
    uint8_t _addrI2C = 0;
    uint8_t _data = 0xFF;
    cb_gpio_change_t _callback = nullptr;

    // Physical reading and writing data from the chip without changing the internal buffer
    bool read8(uint8_t* data);
    bool write8(uint8_t data);
};

#ifdef __cplusplus
}
#endif

#endif // __RE_PCF8574_H__