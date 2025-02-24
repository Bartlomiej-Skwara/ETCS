/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "onboard_test.h"
#include "../DMI/text_message.h"

class MainBrakeTestProcedure : public IOnboardTestProcedure
{
public:
	virtual void proceed(bool startup);
	virtual void handle_test_brake_command();

	bool triggered_manually = false;
	int step = 0;
	text_message* message;
	int64_t last_pressure_change;
	double prev_pipe_pressure;
	double prev_brakecyl_pressure;
};