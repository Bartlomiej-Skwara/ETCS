/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _MENU_H
#define _MENU_H
#include "subwindow.h"
class menu : public subwindow
{
    protected:
    Button *empty_button[10];
    Component *hourGlass=nullptr;
    Button* buttons[10];
    public:
    menu(std::string title);
    ~menu();
    void setLayout() override;
    void setHourGlass(bool show);
};
#endif