#include <iostream>
#include <array>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <fstream>

#include <cstdint>
#include <cstdlib>
#include <csignal>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "keyboard.h"
#include "utils.h"

#define CONF_DIR  "/etc/msi-keyboard-manager"
#define CONF_FILE CONF_DIR "/start.conf"
#define FIFO_DIR  "/var/run/msi-keyboard-manager"
#define FIFO_NAME FIFO_DIR "/cmd"

static bool end = false;
static int32_t fifofd = -1;

void signal_handler(int32_t param)
{
    (void)param;
    end = true;
}

std::vector<std::string> split_line( std::string const& line )
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
    {
        std::cerr << "'breathing_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
        return false;
    }

    switchstr(*it)
    {
        casestr("left")  : area = keyboard_led_manager::area::left; break;
        casestr("right") : area = keyboard_led_manager::area::right; break;
        casestr("middle"): area = keyboard_led_manager::area::middle; break;
        default: std::cerr << "'breathing_mode' 1st parameter (area) should be left, right or middle." << std::endl;return false;
    }

    if( ++it == endit )
    {
        std::cerr << "'breathing_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
        return false;
    }
    else if(it->at(0) != '#' || it->length() != 7)
    {
        std::cerr << "'breathing_mode' 2nd parameter should be the starting color formated like so: #RRGGBB." << std::endl;
        return false;
    }

    try
    {
        Ramp_Color rcolor;
        int32_t color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_start = static_cast<uint8_t>(color>>16);
        rcolor.g_start = static_cast<uint8_t>(color>>8);
        rcolor.b_start = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit )
        {
            std::cerr << "'breathing_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
            return false;
        }
        else if(it->at(0) != '#' || it->length() != 7)
        {
            std::cerr << "'breathing_mode' 3rd parameter should be the ending color formated like so: #RRGGBB." << std::endl;
            return false;
        }

        color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_end = static_cast<uint8_t>(color>>16);
        rcolor.g_end = static_cast<uint8_t>(color>>8);
        rcolor.b_end = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit )
        {
            std::cerr << "'breathing_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
            return false;
        }

        rcolor.speed = static_cast<uint8_t>(std::stoi(*it));

        keyboard_led_manager::breathe_mode(area, rcolor);
        return true;
    }
    catch( std::exception &e )
    {
        std::cerr << "Error while parsing parameter: number contains non-numerical characters." << std::endl;
        return false;
    }
}

bool param_wave( std::vector<std::string>::const_iterator &it, std::vector<std::string>::const_iterator &endit )
{
    keyboard_led_manager::area area;
    if( ++it == endit )
    {
        std::cerr << "'wave_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
        return false;
    }

    switchstr(*it)
    {
        casestr("left")  : area = keyboard_led_manager::area::left; break;
        casestr("right") : area = keyboard_led_manager::area::right; break;
        casestr("middle"): area = keyboard_led_manager::area::middle; break;
        default: std::cerr << "'wave_mode' 1st parameter (area) should be left, right or middle." << std::endl;return false;
    }

    if( ++it == endit )
    {
        std::cerr << "'wave_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
        return false;
    }
    else if(it->at(0) != '#' || it->length() != 7)
    {
        std::cerr << "'wave_mode' 2nd parameter should be the starting color formated like so: #RRGGBB." << std::endl;
        return false;
    }

    try
    {
        Ramp_Color rcolor;
        int32_t color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_start = static_cast<uint8_t>(color>>16);
        rcolor.g_start = static_cast<uint8_t>(color>>8);
        rcolor.b_start = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit )
        {
            std::cerr << "'wave_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
            return false;
        }
        else if(it->at(0) != '#' || it->length() != 7)
        {
            std::cerr << "'wave_mode' 3rd parameter should be the ending color formated like so: #RRGGBB." << std::endl;
            return false;
        }

        color = std::stoi(it->c_str()+1, 0, 16);

        rcolor.r_end = static_cast<uint8_t>(color>>16);
        rcolor.g_end = static_cast<uint8_t>(color>>8);
        rcolor.b_end = static_cast<uint8_t>(color&0xFF);

        if( ++it == endit )
        {
            std::cerr << "'wave_mode' must have 4 parameters: (area) (start_color) (end_color) (color_speed)" << std::endl;
            return false;
        }

        rcolor.speed = static_cast<uint8_t>(std::stoi(*it));

        keyboard_led_manager::wave_mode(area, rcolor);
        return true;
    }
    catch( std::exception &e )
    {
        std::cerr << "Error while parsing parameter: number contains non-numerical characters." << std::endl;
        return false;
    }
}

bool parse_line( std::vector<std::string> const& params )
{
    bool ok = false;
    std::vector<std::string>::const_iterator endit = params.cend();
    for( std::vector<std::string>::const_iterator it = params.cbegin(); it != endit; ++it )
    {
        switchstr(*it)
        {
            casestr("brightness")    : ok = param_brightness(it, endit); break;
            casestr("normal_mode")   : ok = param_normal    (it, endit); break;
            casestr("breathing_mode"): ok = param_breathing (it, endit); break;
            casestr("wave_mode")     : ok = param_wave      (it, endit); break;
            default: ok = false; break;
        }
        if( !ok ) break;
    }
    return ok;
}

void read_conf_file()
{
    std::ifstream conf_file(CONF_FILE);
    std::string line;
    while( std::getline(conf_file, line) )
    {
        parse_line(move(split_line(line)));
    }
}

int main()
{
    hid_init();

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

            read_conf_file();

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
                        if( !parse_line(move(split_line(std::string(buffer.begin(), buffer.begin()+len)))) )
                        {
                        }
                    }
                }
            }
            close(fifofd);
        }
        else
        {
            std::cerr << "Can't open command file " FIFO_NAME " !" << std::endl;
        }
        unlink(FIFO_NAME);
    }
    else
    {
        std::cerr << "Can't create command file " FIFO_NAME " !" << std::endl;
    }

    hid_exit();
    return 0;
}
