/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "menu_tests.h"
#include "../tcp/server.h"

menu_tests::menu_tests() : menu(get_text("Component testing"))
{
    entries[0] = {get_text("Test1"), "", []
    {
        write_command("Adhesion","");
    }, false};
    
    buildMenu();
}

void menu_tests::setEnabled()
{

}