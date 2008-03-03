/* 
 * Copyright (C) 2008,2008 LeGACY <http://code.google.com/p/legacy-project/>
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

#if !defined (CONFIG_H)
#define CONFIG_H

#include <Policies/Singleton.h>

class Config 
{
    public:
        Config();
        ~Config();

        bool SetSource(const char *file, bool ignorecase = true);

        bool GetString(const char* name, std::string *value);
        std::string GetStringDefault(const char* name, const char* def);

        bool GetBool(const char* name, bool *value);
        bool GetBoolDefault(const char* name, const bool def = false);

        bool GetInt(const char* name, int *value);
        int GetIntDefault(const char* name, const int def);

        bool GetFloat(const char* name, float *value);
        float GetFloatDefault(const char* name, const float def);

    private:
        DOTCONFDocument *mConf;
};

#define sConfig LeGACY::Singleton<Config>::Instance()
#endif
