#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <hidapi/hidapi.h>
#include <cstdint>

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Ramp_Color
{
    uint8_t r_start;
    uint8_t g_start;
    uint8_t b_start;

    uint8_t r_end;
    uint8_t g_end;
    uint8_t b_end;

    uint8_t speed;
};

class keyboard_led_manager
{
    hid_device *_dev;

    keyboard_led_manager() = delete;
    keyboard_led_manager(keyboard_led_manager const&) = delete;
    keyboard_led_manager(keyboard_led_manager &&) = delete;
    keyboard_led_manager& operator=(keyboard_led_manager const&) = delete;
    keyboard_led_manager& operator=(keyboard_led_manager &&) = delete;
    ~keyboard_led_manager() = delete;

  public:
    enum class area
    {
        left   = 0x01,
        middle = 0x02,
        right  = 0x03,
    };

    enum class brightness
    {
        disabled = 0x05,
        low      = 0x06,
        mid      = 0x07,
        high     = 0x0A,
    };

    static bool disable_mode();
    static bool normal_mode(keyboard_led_manager::area area, Color color);
    static bool gaming_mode(Color color);
    static bool breathe_mode(keyboard_led_manager::area area, Ramp_Color color);
    static bool audio_mode();
    static bool wave_mode(keyboard_led_manager::area area, Ramp_Color color);

    static bool change_brightness(keyboard_led_manager::brightness brightness);
};

#endif // __KEYBOARD_H
