/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#if __cplusplus >= 201703L
#include <optional>
using std::optional;
#else
#include <initializer_list>
template<class T>
class optional
{
    bool assigned;
    T *value;
    public:
    optional()
    {
        assigned = false;
        value = nullptr;
    }
    operator bool()
    {
        return assigned;
    }
    optional(const optional<T> &o)
    {
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
    }
    optional(optional<T> &&o)
    {
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
    }
    optional(const T &o)
    {
        assigned = true;
        value = new T(o);
    }
    optional(T &&o)
    {
        assigned = true;
        value = new T(o);
    }
    ~optional()
    {
        if (assigned)
            delete value;
    }
    optional &operator = (const optional<T>& o)
    {
        if (assigned)
            delete value;
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
        return *this;
    }
    optional &operator = (optional<T>&& o)
    {
        if (assigned)
            delete value;
        assigned = o.assigned;
        if (assigned)
            value = new T(*o.value);
        return *this;
    }
    optional<T> &operator=(T newval)
    {
        value = new T(newval);
        assigned = true;
        return *this;
    }
    optional<T> &operator=(std::initializer_list<int> l)
    {
        if (assigned)
            delete value;
        assigned = false;
        return *this;
    }
    T &operator*()
    {
        return *value;
    }
    T* operator->()
    {
        return value;
    }
};
#endif