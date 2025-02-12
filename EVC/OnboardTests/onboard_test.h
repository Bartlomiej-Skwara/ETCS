/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <vector>
#pragma once

void load_onboard_tests();
void perform_startup_tests();
bool any_test_in_progress();
class IOnboardTestProcedure;

struct OnboardTest {
    std::string Type;
    IOnboardTestProcedure* Procedure;
    int ValidityTime;
    int ValidityTimeReminder;
    int ValidityDistance;
    bool PrepareOnStartup;
    int LastExecutionTime;
};

class IOnboardTestProcedure
{
public:
    virtual ~IOnboardTestProcedure() {}
    virtual void proceed(OnboardTest test) = 0;
    virtual void handle_test_brake_command() = 0;
    bool running = false;
};