#include "keyboard.h"

/*
Format d'une trame USB:
-----------------------------------------------------
| Octet | 0 | 1 |  2  |  3  |  4  |  5  |  6  |  7  |
|-------|---|---|-----|-----------|-----|-----|-----|
|       |   |   |     |     |     |     |     |     |
| Trame | 1 | 2 | CMD |AREA | dat1| dat2| dat3|  0  |
|       |   |   |     |     |     |     |     |     |
-----------------------------------------------------

Si CMD ==
0x40 : AREA = Keyboard part to light, dat1 = RED, dat2 = GREEN, dat3 = BLUE
0x41 : AREA = MODE, dat1 = 0, dat2 = 0, dat3 = 0
0x44 : Il faut 3 trames
   1): AREA = <AREA>_RAMP_END  , dat1 = RED, dat2 = GREEN, dat3 = BLUE
   2): AREA = <AREA>_RAMP_START, dat1 = RED, dat2 = GREEN, dat3 = BLUE
   3): AREA = <AREA>_RAMP_SPEED, dat1 = RED SPEED, dat2 = GREEN SPEED, dat3 = BLUE SPEED (0 speed is 0.5sec)
0x80 : AREA = brightness (5 -> 10)
*/

#define BUFSIZE 8
#define PRODUCT_ID 0x1770
#define VENDOR_ID  0xff00

enum class keyboard_cmd
{
    color      = 0x40,
    mode       = 0x41,
    ramp       = 0x44,
    brightness = 0x50,
};
enum class keyboard_mode
{
    disable       = 0x00,
    normal        = 0x01,
    gaming        = 0x02,
    breathing     = 0x03,
    audio         = 0x04,
    wave          = 0x05,
    dualcolor     = 0x06,
    idleoff       = 0x07,
    idlebreathing = 0x08,
    idlewave      = 0x09,
};
enum class keyboard_ramp
{
    left_area   = 0x1,
    middle_area = 0x4,
    right_area  = 0x7,
};

struct color_buffer
{
    union
    {
        const uint8_t _buffer[BUFSIZE];
        struct
        {
            const uint8_t _reserved1;
            const uint8_t _reserved2;
            const uint8_t _reserved3;
            uint8_t area;
            uint8_t r;
            uint8_t g;
            uint8_t b;
            const uint8_t _reserved4;
        };
    };
    color_buffer():_reserved1(1), _reserved2(2), _reserved3(static_cast<uint8_t>(keyboard_cmd::color)), _reserved4(0)
    {}
};

struct ramp_buffer
{
    union
    {
        const uint8_t _buffer[BUFSIZE];
        struct
        {
            const uint8_t _reserved1;
            const uint8_t _reserved2;
            const uint8_t _reserved3;
            uint8_t area;
            uint8_t r;
            uint8_t g;
            uint8_t b;
            const uint8_t _reserved4;
        };
    };
    ramp_buffer():_reserved1(1), _reserved2(2), _reserved3(static_cast<uint8_t>(keyboard_cmd::ramp)), _reserved4(0)
    {}
};

struct brightness_buffer
{
    union
    {
        const uint8_t _buffer[BUFSIZE];
        struct
        {
            const uint8_t _reserved1;
            const uint8_t _reserved2;
            const uint8_t _reserved3;
            uint8_t       brightness;
            const uint8_t _reserved4;
            const uint8_t _reserved5;
            const uint8_t _reserved6;
            const uint8_t _reserved7;
        };
    };
    brightness_buffer():_reserved1(1), _reserved2(2), _reserved3(static_cast<uint8_t>(keyboard_cmd::brightness)), _reserved4(0), _reserved5(0), _reserved6(0), _reserved7(0)
    {}
};

struct mode_buffer
{
    union
    {
        const uint8_t _buffer[BUFSIZE];
        struct
        {
            const uint8_t _reserved1;
            const uint8_t _reserved2;
            const uint8_t _reserved3;
            uint8_t mode;
            const uint8_t _reserved4;
            const uint8_t _reserved5;
            const uint8_t _reserved6;
            const uint8_t _reserved7;
        };
    };
    mode_buffer():
        _reserved1(1), _reserved2(2),
        _reserved3(static_cast<uint8_t>(keyboard_cmd::mode)),
        _reserved4(0), _reserved5(0), _reserved6(0), _reserved7(0)
    {}
};

hid_device* open_keyboard(uint16_t product_id = PRODUCT_ID, uint16_t vendor_id = VENDOR_ID)
{
    return hid_open(product_id, vendor_id, 0);
}

void close_keyboard(hid_device *dev)
{
    if( dev != nullptr )
        hid_close(dev);
}

bool send_cmd( hid_device *dev, const uint8_t (&buf)[BUFSIZE] )
{
    if( dev == nullptr )
        return false;

    return hid_send_feature_report(dev, buf, BUFSIZE) != BUFSIZE;
}

bool keyboard_led_manager::disable_mode()
{
    hid_device *dev;
    mode_buffer mode;

    dev = open_keyboard();
    if( dev == nullptr )
        return false;

    mode.mode = static_cast<uint8_t>(keyboard_mode::disable);
    send_cmd(dev, mode._buffer);

    close_keyboard(dev);
    return true;
}

bool keyboard_led_manager::normal_mode(keyboard_led_manager::area area, Color color)
{
    hid_device *dev;
    mode_buffer mode;
    color_buffer color_buf;

    dev = open_keyboard();
    if( dev == nullptr )
        return false;

    mode.mode = static_cast<uint8_t>(keyboard_mode::normal);
    send_cmd(dev, mode._buffer);

    color_buf.area = static_cast<uint8_t>(area);
    color_buf.r = color.r;
    color_buf.g = color.g;
    color_buf.b = color.b;
    send_cmd(dev, color_buf._buffer);

    close_keyboard(dev);
    return true;
}

bool keyboard_led_manager::gaming_mode(Color color)
{
    hid_device *dev;
    mode_buffer mode;
    color_buffer color_buf;

    dev = open_keyboard();
    if( dev == nullptr )
        return false;

    mode.mode = static_cast<uint8_t>(keyboard_mode::gaming);
    send_cmd(dev, mode._buffer);

    color_buf.area = static_cast<uint8_t>(keyboard_led_manager::area::left);
    color_buf.r = color.r;
    color_buf.g = color.g;
    color_buf.b = color.b;
    send_cmd(dev, color_buf._buffer);

    return true;
}

bool keyboard_led_manager::breathe_mode(keyboard_led_manager::area area, Ramp_Color color)
{
    hid_device *dev;
    mode_buffer mode;
    ramp_buffer ramp;

    dev = open_keyboard();
    if( dev == nullptr )
        return false;

    // End color Frame
    // 1 = end color
    // 2 = start color
    // 3 = color speed
    switch( area )
    {
        // Start at 1
        case area::left:   ramp.area = static_cast<uint8_t>(keyboard_ramp::left_area); break;
        // Start at 4
        case area::middle: ramp.area = static_cast<uint8_t>(keyboard_ramp::middle_area); break;
        // Start at 7
        case area::right:  ramp.area = static_cast<uint8_t>(keyboard_ramp::right_area); break;
    }

    ramp.r = color.r_end;
    ramp.g = color.g_end;
    ramp.b = color.b_end;
    // Send end color Frame
    send_cmd(dev, ramp._buffer);

    // Start Color Frame
    ++ramp.area;
    ramp.r = color.r_start;
    ramp.g = color.g_start;
    ramp.b = color.b_start;
    // Send end color Frame
    send_cmd(dev, ramp._buffer);

    // Start Speed Frame
    ++ramp.area;
    ramp.r = color.speed;
    ramp.g = color.speed;
    ramp.b = color.speed;
    // Send Speed Frame
    send_cmd(dev, ramp._buffer);

    // Send breathe mode
    mode.mode = static_cast<uint8_t>(keyboard_mode::breathing);
    send_cmd(dev, mode._buffer);

    close_keyboard(dev);

    return true;
}

bool keyboard_led_manager::audio_mode()
{
    hid_device *dev;
    mode_buffer mode;
    ramp_buffer ramp;

    dev = open_keyboard();
    if( dev == nullptr )
        return false;

    mode.mode = static_cast<uint8_t>(keyboard_mode::audio);
    send_cmd(dev, mode._buffer);

    close_keyboard(dev);

    return true;
}

bool keyboard_led_manager::wave_mode(keyboard_led_manager::area area, Ramp_Color color)
{
    hid_device *dev;
    mode_buffer mode;
    ramp_buffer ramp;

    dev = open_keyboard();
    if( dev == nullptr )
        return false;

    // End color Frame
    // 1 = end color
    // 2 = start color
    // 3 = color speed
    switch( area )
    {
        // Start at 1
        case area::left:   ramp.area = static_cast<uint8_t>(keyboard_ramp::left_area); break;
        // Start at 4
        case area::middle: ramp.area = static_cast<uint8_t>(keyboard_ramp::middle_area); break;
        // Start at 7
        case area::right:  ramp.area = static_cast<uint8_t>(keyboard_ramp::right_area); break;
    }

    ramp.r = color.r_end;
    ramp.g = color.g_end;
    ramp.b = color.b_end;
    // Send end color Frame
    send_cmd(dev, ramp._buffer);

    // Start Color Frame
    ++ramp.area;
    ramp.r = color.r_start;
    ramp.g = color.g_start;
    ramp.b = color.b_start;
    // Send end color Frame
    send_cmd(dev, ramp._buffer);

    // Start Speed Frame
    ++ramp.area;
    ramp.r = color.speed;
    ramp.g = color.speed;
    ramp.b = color.speed;
    // Send Speed Frame
    send_cmd(dev, ramp._buffer);

    // Send breathe mode
    mode.mode = static_cast<uint8_t>(keyboard_mode::wave);
    send_cmd(dev, mode._buffer);

    close_keyboard(dev);

    return true;
}

bool keyboard_led_manager::change_brightness(keyboard_led_manager::brightness brightness)
{
    hid_device *dev;
    brightness_buffer buffer;

    dev = open_keyboard();
    if( dev == nullptr )
        return false;

    buffer.brightness = static_cast<uint8_t>(brightness);
    send_cmd(dev, buffer._buffer);

    close_keyboard(dev);

    return true;
}

