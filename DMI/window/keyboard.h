/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include <vector>
#include <string>
#include "../graphics/button.h"
struct keyboard
{
    std::vector<Button*> keys;
    std::vector<Component*> labels;
    Button* del = nullptr;
};
#endif