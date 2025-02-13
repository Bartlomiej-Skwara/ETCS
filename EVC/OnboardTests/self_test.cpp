/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "self_test.h"
#include "../DMI/text_message.h"
#include "../language/language.h"
#include "../Supervision/supervision.h"


void SelfTestProcedure::proceed(OnboardTest test, bool startup)
{
	return;
	running = true;

	int64_t time = get_milliseconds();
	if (mode == Mode::SB)
		add_message(text_message(get_text("Self test OK"), true, false, false, [time](text_message& t) { return time + 60000 < get_milliseconds(); }));
	else
		add_message(text_message(get_text("System failure!"), true, false, false, [time](text_message& t) { return false; }));

	running = false;
}

void SelfTestProcedure::handle_test_brake_command() {

}