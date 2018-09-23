#include <iostream>
#include <array>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>

#include <cstdint>
#include <cstdlib>
#include <csignal>
#include <cstring>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "keyboard.h"
#include "utils.h"

#define FIFO_DIR  "/var/run/msi-keyboard-manager"
#define FIFO_NAME FIFO_DIR "/cmd"

static bool end = false;
static int32_t fifofd = -1;

void signal_handler(int32_t param)
{
    (void)param;
    end = true;
}

std::vector<std::string> parse_line( std::string const& line )
{
    std::istringstream iss(line);
    return std::vector<std::string>(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>());
}

bool param_brightness( std::vector<std::string>::const_iterator &it, std::vector<std::string>::const_iterator &endit )
{
    if( ++it == endit )
        return false;

    switchstr(*it)
    {
        casestr("disabled"): keyboard_led_manager::change_brightness(keyboard_led_manager::brightness::disabled); return true;
        casestr("low")     : keyboard_led_manager::change_brightness(keyboard_led_manager::brightness::low)     ; return true;
        casestr("mid")     : keyboard_led_manager::change_brightness(keyboard_led_manager::brightness::mid)     ; return true;
        casestr("high")    : keyboard_led_manager::change_brightness(keyboard_led_manager::brightness::high)    ; return true;
    }
    return false;
}

bool param_normal( std::vector<std::string>::const_iterator &it, std::vector<std::string>::const_iterator &endit )
{
    keyboard_led_manager::area area;
    if( ++it == endit )
        return false;

    switchstr(*it)
    {
        casestr("left")  : area = keyboard_led_manager::area::left; break;
        casestr("right") : area = keyboard_led_manager::area::right; break;
        casestr("middle"): area = keyboard_led_manager::area::middle; break;
        default: return false;
    }

    if( ++it == endit || it->at(0) != '#' || it->length() != 7 )
        return false;

    std::string const& str = *it;
    try
    {
        uint32_t color = std::stoi(str.c_str()+1, 0, 16);
        keyboard_led_manager::normal_mode(area, {static_cast<uint8_t>(color>>16), static_cast<uint8_t>(color>>8), static_cast<uint8_t>(color&0xFF)});
        return true;
    }
    catch( std::exception &e )
    {
        return false;
    }
}

bool param_breathing( std::vector<std::string>::const_iterator &it, std::vector<std::string>::const_iterator &endit )
{
    keyboard_led_manager::area area;
    if( ++it == endit )
        return false;

    switchstr(*it)
    {
        casestr("left")  : area = keyboard_led_manager::area::left; break;
        casestr("right") : area = keyboard_led_manager::area::right; break;
        casestr("middle"): area = keyboard_led_manager::area::middle; break;
        default: return false;
    }

    if( ++it == endit || it->at(0) != '#' || it->length() != 7 )
        return false;

    try
    {
        Ramp_Color rcolor;
        int32_t color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_start = static_cast<uint8_t>(color>>16);
        rcolor.g_start = static_cast<uint8_t>(color>>8);
        rcolor.b_start = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit || it->at(0) != '#' || it->length() != 7 )
            return false;

        color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_end = static_cast<uint8_t>(color>>16);
        rcolor.g_end = static_cast<uint8_t>(color>>8);
        rcolor.b_end = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit )
            return false;

        rcolor.speed = static_cast<uint8_t>(std::stoi(*it));

        keyboard_led_manager::breathe_mode(area, rcolor);
        return true;
    }
    catch( std::exception &e )
    {
        return false;
    }
}

bool param_wave( std::vector<std::string>::const_iterator &it, std::vector<std::string>::const_iterator &endit )
{
    keyboard_led_manager::area area;
    if( ++it == endit )
        return false;

    switchstr(*it)
    {
        casestr("left")  : area = keyboard_led_manager::area::left; break;
        casestr("right") : area = keyboard_led_manager::area::right; break;
        casestr("middle"): area = keyboard_led_manager::area::middle; break;
        default: return false;
    }

    if( ++it == endit || it->at(0) != '#' || it->length() != 7 )
        return false;

    try
    {
        Ramp_Color rcolor;
        int32_t color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_start = static_cast<uint8_t>(color>>16);
        rcolor.g_start = static_cast<uint8_t>(color>>8);
        rcolor.b_start = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit || it->at(0) != '#' || it->length() != 7 )
            return false;

        color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_end = static_cast<uint8_t>(color>>16);
        rcolor.g_end = static_cast<uint8_t>(color>>8);
        rcolor.b_end = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit )
            return false;

        rcolor.speed = static_cast<uint8_t>(std::stoi(*it));

        keyboard_led_manager::wave_mode(area, rcolor);
        return true;
    }
    catch( std::exception &e )
    {
        return false;
    }
}

int main()
{
    hid_init();

    struct stat st;
    memset(&st, 0, sizeof(struct stat));

    if (stat(FIFO_DIR, &st) == -1)
    {
        if( mkdir(FIFO_DIR, 0755) == -1 )
        {
            std::cerr << "Can't make " FIFO_DIR " !" << std::endl;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);

    umask(0000);
    if( mkfifo(FIFO_NAME, 0666) != -1 )
    {
        fifofd = open(FIFO_NAME, O_RDWR);
        if( fifofd != -1 )
        {
            fd_set readset;
            timeval timeout;
            std::array<char, 8192> buffer;
            size_t len;
            std::vector<std::string> params;

            while( !end )
            {
                FD_ZERO(&readset);
                FD_SET(fifofd, &readset);
                timeout.tv_usec = 0;
                timeout.tv_sec = 5;
                if( select(fifofd+1, &readset, 0, 0, &timeout) == 1 )
                {
                    len = read(fifofd, buffer.data(), 8191);
                    if( len > 0 )
                    {
                        bool ok;
                        params = move(parse_line(std::string(buffer.begin(), buffer.begin()+len)));
                        std::vector<std::string>::const_iterator endit = params.cend();
                        for( std::vector<std::string>::const_iterator it = params.cbegin(); it != endit; ++it )
                        {
                            switchstr(*it)
                            {
                                casestr("brightness")    : ok = param_brightness(it, endit); break;
                                casestr("normal_mode")   : ok = param_normal    (it, endit); break;
                                casestr("breathing_mode"): ok = param_breathing (it, endit); break;
                                casestr("wave_mode")     : ok = param_wave      (it, endit); break;
                                default: ok = false;
                            }
                            if( !ok )
                            {
                                break;
                            }
                        }
                    }
                }
            }
            close(fifofd);
        }
        unlink(FIFO_NAME);
    }
    else
    {
        std::cerr << "Can't open " FIFO_NAME " !" << std::endl;
    }

    hid_exit();
    return 0;
}
