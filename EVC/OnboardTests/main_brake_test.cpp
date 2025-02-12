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
void MainBrakeTestProcedure::proceed(OnboardTest test)
{
	running = true;
	step = 0;

	if (step == 0) {
		int64_t time = get_milliseconds();
		message_to_ack = &add_message(text_message(get_text("Wymagany test hamulca. Potwierdz, aby rozpoczac."), true, true, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
		step = 1;
	}
}

void MainBrakeTestProcedure::handle_test_brake_command() {
	int64_t time = get_milliseconds();

	if (step == 1 && message_to_ack != nullptr && message_to_ack->acknowledged)
	{
		message_to_ack = nullptr;
		platform->delay(1000).then([this]() {
			step = 2;
			}).detach();
	}
	else if (step == 2) {
		add_message(text_message(get_text("Trwa hamowanie ..."), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
		EB_command = true;
		step = 3;
	}
	else if (step == 3) {
		if (pipe_pressure < 0.5 && brakecyl_pressure > 1) {
			add_message(text_message(get_text("Trwa luzowanie ..."), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
			EB_command = false;
			step = 4;
		}
	}
	else if (step == 4) {
		if (pipe_pressure > 4.9 && brakecyl_pressure < 0.1) {
			add_message(text_message(get_text("Trwa hamowanie ..."), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
			EB_command = true;
			step = 5;
		}
	}
	else if (step == 5) {
		if (pipe_pressure < 0.5 && brakecyl_pressure > 1) {
			add_message(text_message(get_text("Trwa luzowanie ..."), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
			EB_command = false;
			step = 6;
		}
	}
	else if (step == 6) {
		if (pipe_pressure > 4.9 && brakecyl_pressure < 0.1) {
			add_message(text_message(get_text("Test hamulca: OK"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
			running = false;
		}
	}
}