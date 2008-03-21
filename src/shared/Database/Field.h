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

#ifndef __LEGACY_FIELD_H
#define __LEGACY_FIELD_H

class Field
{
    public:

        enum DataTypes
        {
            DB_TYPE_UNKNOWN = 0x00,
            DB_TYPE_STRING  = 0x01,
            DB_TYPE_INTEGER = 0x02,
            DB_TYPE_FLOAT   = 0x03
        };

        Field();
        Field(Field &f);
        Field(const char *value, const char *name, enum DataTypes type);

        ~Field();

        const char *GetName() const { return mName; }
        enum DataTypes GetType() const { return mType; }

        const char *GetString() const { return mValue; }
        std::string GetCppString() const
        {
            return mValue ? mValue : "";                    // std::string s = 0 have undefine result in C++
        }
        float GetFloat() const { return mValue ? static_cast<float>(atof(mValue)) : 0; }
        bool GetBool() const { return mValue ? atoi(mValue) > 0 : false; }
        int32 GetInt32() const { return mValue ? static_cast<int32>(atol(mValue)) : 0; }
        uint8 GetUInt8() const { return mValue ? static_cast<uint8>(atoi(mValue)) : 0; }
        uint16 GetUInt16() const { return mValue ? static_cast<uint16>(atol(mValue)) : 0; }
        int16 GetInt16() const { return mValue ? static_cast<int16>(atol(mValue)) : 0; }
        uint32 GetUInt32() const { return mValue ? static_cast<uint32>(atol(mValue)) : 0; }
        uint64 GetUInt64() const
        {
            if(mValue)
            {
                uint64 value;
                sscanf(mValue,I64FMTD,&value);
                return value;
            }
            else
                return 0;
        }

        void SetName(const char *name);
        void SetType(enum DataTypes type) { mType = type; }

        void SetValue(const char *value);

    private:
        char *mValue;
        char *mName;
        enum DataTypes mType;
};
#endif
