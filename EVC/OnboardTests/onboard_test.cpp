/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "onboard_test.h"
#include "main_brake_test.h"
#include "self_test.h"
#include "../DMI/windows.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
std::vector<OnboardTest> LoadedOnboardTests;

void load_onboard_tests()
{
    auto contents = platform->read_file("onboard_tests.json");
    if (!contents)
        return;

    json j = json::parse(*contents);

    if (!j.contains("Tests") || !j["Tests"].is_array()) {
        return;
    }

    LoadedOnboardTests.clear();

    for (const auto& item : j["Tests"]) {
        if (!item.contains("Type"))
            continue;

        IOnboardTestProcedure* procedure;
        std::string type = item["Type"].get<std::string>();
        if (type == "MainBrakeTest") {
            procedure = new MainBrakeTestProcedure();
        } else if (type == "SelfTest") {
            procedure = new SelfTestProcedure();
        }
        else {
            procedure = nullptr;
        }

        LoadedOnboardTests.push_back({
            item["Type"].get<std::string>(),
            procedure,
            item.contains("ValidityTime") ? item["ValidityTime"].get<int>() : -1,
            item.contains("ValidityTimeReminder") ? item["ValidityTimeReminder"].get<int>() : -1,
            item.contains("ValidityDistance") ? item["ValidityDistance"].get<int>() : -1,
            item.contains("PrepareOnStartup") ? item["PrepareOnStartup"].get<bool>() : false,
            item.contains("LastExecutionTime") ? item["LastExecutionTime"].get<int>() : -1
            });
    }
}

void perform_startup_tests() {
    for (const auto& item : LoadedOnboardTests) {
        if (item.PrepareOnStartup) {
            item.Procedure->proceed(item);
        }
    }
}

bool any_test_in_progress() {
    for (const auto& item : LoadedOnboardTests) {
        if (item.Procedure->running) {
            return true;
        }
    }

    if (active_dialog == dialog_sequence::StartUpTest) {
        active_dialog = dialog_sequence::StartUp;
    }

    return false;
}
