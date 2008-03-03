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

#ifndef __LEGACY_OBJECTREGISTRY_H
#define __LEGACY_OBJECTREGISTRY_H

#include "Platform/Define.h"
#include "Utilities/HashMap.h"
#include "Policies/Singleton.h"

#include <string>
#include <map>

/** ObjectRegistry holds all registry item of the same type
 */
template<class T, class Key = std::string>
class LEGACY_DLL_DECL ObjectRegistry
{
    typedef std::map<Key, T *> RegistryMapType;
    RegistryMapType i_registeredObjects;
    friend class LeGACY::OperatorNew<ObjectRegistry<T, Key> >;

    // protected for friend use since it should be a singleton
    ObjectRegistry() {}
    ~ObjectRegistry()
    {
        for(typename RegistryMapType::iterator iter=i_registeredObjects.begin(); iter != i_registeredObjects.end(); ++iter)
            delete iter->second;
        i_registeredObjects.clear();
    }

    public:

        /// Returns a registry item
        const T* GetRegistryItem(Key key) const
        {
            typename RegistryMapType::const_iterator iter = i_registeredObjects.find(key);
            return( iter == i_registeredObjects.end() ? NULL : iter->second );
        }

        /// Inserts a registry item
        bool InsertItem(T *obj, Key key, bool override = false)
        {
            typename RegistryMapType::iterator iter = i_registeredObjects.find(key);
            if( iter != i_registeredObjects.end() )
            {
                if( !override )
                    return false;
                delete iter->second;
                i_registeredObjects.erase(iter);
            }

            i_registeredObjects[key] = obj;
            return true;
        }

        /// Removes a registry item
        void RemoveItem(Key key, bool delete_object = true)
        {
            typename RegistryMapType::iterator iter = i_registeredObjects.find(key);
            if( iter != i_registeredObjects.end() )
            {
                if( delete_object )
                    delete iter->second;
                i_registeredObjects.erase(iter);
            }
        }

        /// Returns true if registry contains an item
        bool HasItem(Key key) const
        {
            return (i_registeredObjects.find(key) != i_registeredObjects.end());
        }

        /// Return a list of registered items
        unsigned int GetRegisteredItems(std::vector<Key> &l) const
        {
            unsigned int sz = l.size();
            l.resize(sz + i_registeredObjects.size());
            for(typename RegistryMapType::const_iterator iter = i_registeredObjects.begin(); iter != i_registeredObjects.end(); ++iter)
                l[sz++] = iter->first;
            return i_registeredObjects.size();
        }
};
#endif
