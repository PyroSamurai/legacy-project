/* 
 * Copyright (C) 2008-2008 LeGACY <http://code.google.com/p/legacy-project/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Util.h"

#include "sockets/socket_include.h"

Tokens StrSplit(const std::string &src, const std::string &sep)
{
    Tokens r;
    std::string s;
    for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
    {
        if (sep.find(*i) != std::string::npos)
        {
            if (s.length()) r.push_back(s);
            s = "";
        }
        else
        {
            s += *i;
        }
    }
    if (s.length()) r.push_back(s);
    return r;
}

void stripLineInvisibleChars(std::string &str)
{
    static std::string invChars = " \t\7";

    size_t wpos = 0;

    bool space = false;
    for(size_t pos = 0; pos < str.size(); ++pos)
    {
        if(invChars.find(str[pos])!=std::string::npos)
        {
            if(!space)
            {
                str[wpos++] = ' ';
                space = true;
            }
        }
        else
        {
            if(wpos!=pos)
                str[wpos++] = str[pos];
            else
                ++wpos;
            space = false;
        }
    }

    if(wpos < str.size())
        str.erase(wpos,str.size());
}

std::string secsToTimeString(uint32 timeInSecs, bool shortText, bool hoursOnly)
{
    uint32 secs    = timeInSecs % MINUTE;
    uint32 minutes = timeInSecs % HOUR / MINUTE;
    uint32 hours   = timeInSecs % DAY  / HOUR;
    uint32 days    = timeInSecs / DAY;

    std::ostringstream ss;
    if(days)
        ss << days << (shortText ? "d" : " Day(s) ");
    if(hours)
        ss << hours << (shortText ? "h" : " Hour(s) ");
    if(!hoursOnly)
    {
        if(minutes)
            ss << minutes << (shortText ? "m" : " Minute(s) ");
        if(secs)
            ss << secs << (shortText ? "s" : " Second(s).");
    }

    return ss.str();
}

uint32 TimeStringToSecs(std::string timestring)
{
    uint32 secs       = 0;
    uint32 buffer     = 0;
    uint32 multiplier = 0;

    for(std::string::iterator itr = timestring.begin(); itr != timestring.end(); itr++ )
    {
        if(isdigit(*itr))
        {
            std::string str;                                //very complicated typecast char->const char*; is there no better way?
            str += *itr;
            const char* tmp = str.c_str();

            buffer*=10;
            buffer+=atoi(tmp);
        }
        else
        {
            switch(*itr)
            {
                case 'd': multiplier = DAY;     break;
                case 'h': multiplier = HOUR;    break;
                case 'm': multiplier = MINUTE;  break;
                case 's': multiplier = 1;       break;
                default : return 0;                         //bad format
            }
            buffer*=multiplier;
            secs+=buffer;
            buffer=0;
        }

    }

    return secs;
}

/// Check if the string is a valid ip address representation
bool IsIPAddress(char const* ipaddress)
{
    if(!ipaddress)
        return false;

    // Let the big boys do it.
    // Drawback: all valid ip address formats are recognized e.g.: 12.23,121234,0xABCD)
    return inet_addr(ipaddress) != INADDR_NONE;
}

// internal status if the irand() random number generator
static uint32 holdrand = 0x89abcdef;

// initialize the irand() random number generator
void Rand_Init(uint32 seed)
{
    holdrand = seed;
}

/* Return a random number in the range min .. max.
 * max-min must be smaller than 32768. */
int32 irand(int32 min, int32 max)
{
    assert((max - min) < 32768);

    ++max;
    holdrand = (holdrand * 214013) + 2531011;

    return (((holdrand >> 17) * (max - min)) >> 15) + min;
}

// current state of the random number generator
static int32 rand32_state = 1;

/* Return a pseudo-random number in the range 0 .. RAND32_MAX.
 * Note: Not reentrant - if two threads call this simultaneously, they will likely
 * get the same random number. */
int32 rand32(void)
{
    #   define m   2147483647
    #   define a   48271
    #   define q   (m / a)
    #   define r   (m % a)

    const int32 hi = rand32_state / q;
    const int32 lo = rand32_state % q;
    const int32 test = a * lo - r * hi;

    if (test > 0)
        rand32_state = test;
    else
        rand32_state = test + m;
    return rand32_state - 1;
}

/// create PID file
uint32 CreatePIDFile(std::string filename)
{
    FILE * pid_file = fopen (filename.c_str(), "w" );
    if (pid_file == NULL)
        return 0;

#ifdef WIN32
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif

    fprintf(pid_file, "%d", pid );
    fclose(pid_file);

    return (uint32)pid;
}

