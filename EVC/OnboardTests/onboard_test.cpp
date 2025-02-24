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
#include "../platform/platform.h"
#include "../DMI/windows.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
std::vector<OnboardTest> LoadedOnboardTests;
json LoadedOnboardTestsJson;

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
		if (type == "ETCS braking test") {
			procedure = new MainBrakeTestProcedure();
		}
		else if (type == "Self test") {
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
			item.contains("CanBeStartedManually") ? item["CanBeStartedManually"].get<bool>() : false,
			item.contains("LastSuccessTimestamp") ? item["LastSuccessTimestamp"].get<int>() : -1,
			item.contains("LastFailureTimestamp") ? item["LastFailureTimestamp"].get<int>() : -1
			});
	}

	save_onboard_tests();
}

void perform_startup_tests() {
	for (const auto& item : LoadedOnboardTests) {
		if (item.PrepareOnStartup) {
			item.Procedure->proceed(true);
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

void save_onboard_tests() {
	for (auto& item : LoadedOnboardTests) {
		switch (item.Procedure->result) {
		case 1:
			item.LastSuccessTimestamp = platform->get_local_time().to_unix_timestamp();
			break;
		case -1:
			item.LastFailureTimestamp = platform->get_local_time().to_unix_timestamp();
			break;
		default:
			break;
		}
		item.Procedure->result = 0;
	}

	json json_array = json::array();
	for (auto& item : LoadedOnboardTests) {
		json::object_t test =
		{
			{"Type", item.Type},
			{"ValidityTime", item.ValidityTime },
			{"ValidityTimeReminder", item.ValidityTimeReminder},
			{"PrepareOnStartup", item.PrepareOnStartup},
			{"CanBeStartedManually", item.CanBeStartedManually},
			{"LastSuccessTimestamp", item.LastSuccessTimestamp},
			{"LastFailureTimestamp", item.LastFailureTimestamp},
			{"InProgress", item.Procedure->running},
		};
		json_array.push_back(test);
	}
	json::object_t main_obj = { {"Tests", json_array} };
	json main_json(main_obj);
	LoadedOnboardTestsJson = main_json;
	platform->write_file("onboard_tests.json", main_json.dump(1));
}
