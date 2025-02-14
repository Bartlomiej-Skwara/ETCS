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

extern double pipe_pressure;
extern double brakecyl_pressure;

/*
   Step 0 - Start of the procedure
   Step 1 - Initial message to confirm
   Step 2 - Delay between acknowledgment and start of the main test
   Step 3 - First braking, waiting for the brake to apply
   Step 4 - First releasing, waiting for the brake to release
   Step 5 - Second braking, waiting for the brake to apply
   Step 6 - Second releasing, waiting for the brake to release
*/
void MainBrakeTestProcedure::proceed(OnboardTest test, bool startup)
{
	running = true;
	step = 0;

	if (startup && step == 0) {
		int64_t time = get_milliseconds();
		message_to_ack = &add_message(text_message(get_text("Wymagany test hamulcow. Potwierdz, aby rozpoczac."), true, true, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
		step = 1;

	}
	else {
		step = 1;
	}

	prev_brakecyl_pressure = brakecyl_pressure;
	prev_pipe_pressure = pipe_pressure;
}

void MainBrakeTestProcedure::handle_test_brake_command() {
	int64_t time = get_milliseconds();

	if (abs(prev_brakecyl_pressure - brakecyl_pressure) > 0.01 || abs(prev_pipe_pressure - pipe_pressure) > 0.01) {
		last_pressure_change = get_milliseconds();
		prev_brakecyl_pressure = brakecyl_pressure;
		prev_pipe_pressure = pipe_pressure;
	}

	if (step > 1 && last_pressure_change > 0 && time - last_pressure_change > 10000) {
		running = false;
		failed = true;
		EB_command = true;
		release_command = false;
		add_message(text_message(get_text("Test zakonczony niepowodzeniem!"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
	}

	if (step == 1 && message_to_ack != nullptr && message_to_ack->acknowledged)
	{
		message_to_ack = nullptr;
		add_message(text_message(get_text("Test trwa ..."), true, false, false, [this](text_message& t) { return !(running || EB_command || release_command); }));
		platform->delay(1000).then([this]() {
			step = 2;
			}).detach();
	}
	else if (step == 2) {
		last_pressure_change = get_milliseconds();
		EB_command = true;
		release_command = false;
		step = 3;
		add_message(text_message(get_text("Test hamulcow ..."), true, false, false, [this](text_message& t) { return !(EB_command || release_command); }));
	}
	else if (step == 3) {
		if (pipe_pressure < 0.5 && brakecyl_pressure > 1) {
			EB_command = false;
			release_command = true;
			step = 4;
		}
	}
	else if (step == 4) {
		if (pipe_pressure > 4.9 && brakecyl_pressure < 0.1) {
			EB_command = true;
			release_command = false;
			step = 5;
			add_message(text_message(get_text("Test hamulcow ..."), true, false, false, [this](text_message& t) { return !(EB_command || release_command); }));
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
		if (pipe_pressure > 4.9 && brakecyl_pressure < 0.1) {
			running = false;
			EB_command = false;
			release_command = false;
			add_message(text_message(get_text("Test OK!"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
		}
	}
}