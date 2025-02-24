/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <functional>
#include <list>
#include <algorithm>
#include "main_brake_test.h"
#include "../TrainSubsystems/train_interface.h"
#include "../TrainSubsystems/brake.h"
#include "../DMI/text_message.h"
#include "../language/language.h"
#include "../Supervision/supervision.h"
#include "../platform/platform.h"

extern double pipe_pressure;
extern double brakecyl_pressure;
extern std::vector<OnboardTest> LoadedOnboardTests;

/*
   Step 0 - Start of the procedure
   Step 1 - Initial message to confirm
   Step 2 - Delay between acknowledgment and start of the main test
   Step 3 - First braking, waiting for the brake to apply
   Step 4 - First releasing, waiting for the brake to release
   Step 5 - Second braking, waiting for the brake to apply
   Step 6 - Second releasing, waiting for the brake to release
*/
void MainBrakeTestProcedure::proceed(bool startup)
{
	auto time = platform->get_local_time().to_unix_timestamp();
	add_message(text_message("Timestamp = " + std::to_string(time), true, true, false, [this](text_message& t) { return false; }));

	running = true;
	step = 0;

	if (startup && step == 0) {
		for (const auto& item : LoadedOnboardTests) {
			if (item.Procedure == this) {
				long current_timestamp = platform->get_local_time().to_unix_timestamp();
				long last_success_timestamp = item.LastSuccessTimestamp;
				long last_failure_timestamp = item.LastFailureTimestamp;
				long delta_time = current_timestamp - last_success_timestamp;
 				if ((item.ValidityTime > 0 && delta_time > item.ValidityTime) || last_failure_timestamp > last_success_timestamp) {
					int64_t time = get_milliseconds();
					message = &add_message(text_message(get_text("ETCS brake test required. Confirm to proceed."), true, true, false, [this](text_message& t) { return step > 1; }));
					step = 1;
				}
 				else if (item.ValidityTime > 0 && delta_time > item.ValidityTimeReminder) {
					int64_t time = get_milliseconds();
					message = &add_message(text_message(get_text("ETCS brake test recommended."), true, false, false, [time](text_message& t) { return time + 30000 < get_milliseconds(); }));
					running = false;
					EB_command = false;
					release_command = false;
					save_onboard_tests();
					return;
				}
				else {
					running = false;
					EB_command = false;
					release_command = false;
					save_onboard_tests();
					return;
				}
			}
		}
	}
	else {
		step = 1;
	}

	triggered_manually = !startup;
	prev_brakecyl_pressure = brakecyl_pressure;
	prev_pipe_pressure = pipe_pressure;
}

void MainBrakeTestProcedure::handle_test_brake_command() {
	int64_t time = get_milliseconds();
	result = 0;

	if (abs(prev_brakecyl_pressure - brakecyl_pressure) > 0.01 || abs(prev_pipe_pressure - pipe_pressure) > 0.01) {
		last_pressure_change = get_milliseconds();
		prev_brakecyl_pressure = brakecyl_pressure;
		prev_pipe_pressure = pipe_pressure;
	}

	if ((step > 1 && last_pressure_change > 0 && time - last_pressure_change > 10000) || (step == 1 && pipe_pressure < 4.5 && message != nullptr && message->acknowledged)) {
		running = false;
		failed = true;
		EB_command = true;
		release_command = false;
		add_message(text_message(get_text("Test failed!"), true, false, false, [time](text_message& t) { return false; }));
		result = -1;
		save_onboard_tests();
	}

	if (step == 1 && (message != nullptr && message->acknowledged || triggered_manually))
	{
		prev_brakecyl_pressure = brakecyl_pressure;
		prev_pipe_pressure = pipe_pressure;
		last_pressure_change = get_milliseconds();

		message = nullptr;
		triggered_manually = false;

		add_message(text_message(get_text("Test in progress..."), true, false, false, [this](text_message& t) { return !running; }));
		platform->delay(1000).then([this]() {
			step = 2;
			}).detach();
	}
	else if (step == 2) {
		EB_command = true;
		release_command = false;
		step = 3;
		add_message(text_message(get_text("Braking test..."), true, false, false, [this](text_message& t) { return !EB_command || !running; }));
	}
	else if (step == 3) {
		if (pipe_pressure < 0.5 && brakecyl_pressure > 1) {
			EB_command = false;
			release_command = true;
			step = 4;
		}
	}
	else if (step == 4) {
		if (pipe_pressure > 4.5 && brakecyl_pressure < 0.1) {
			EB_command = true;
			release_command = false;
			step = 5;
			add_message(text_message(get_text("Braking test..."), true, false, false, [this](text_message& t) { return !EB_command || !running; }));
		}
	}
	else if (step == 5) {
		if (pipe_pressure < 0.5 && brakecyl_pressure > 1) {
			EB_command = false;
			release_command = true;
			step = 6;
		}
	}
	else if (step == 6) {
		if (pipe_pressure > 4.5 && brakecyl_pressure < 0.1) {
			running = false;
			EB_command = false;
			release_command = false;
			add_message(text_message(get_text("Test successful!"), true, false, false, [time](text_message& t) { return time + 30000 < get_milliseconds(); }));
			result = 1;
			save_onboard_tests();
		}
	}
}