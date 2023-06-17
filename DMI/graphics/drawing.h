/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _DRAWING_H
#define _DRAWING_H
#include "color.h"
#include <string>
#define CENTER 0
#define RIGHT 1
#define LEFT 2
#define UP 4
#define DOWN 8
void startDisplay(bool fullscreen, int display, int width, int height, bool borderless, bool rotate);
void display();
void quitDisplay();
void init_video();
void loop_video();
#endif
