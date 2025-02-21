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
#include "../../EVC/OnboardTests/onboard_test.h"


menu_tests::menu_tests(json onboard_tests) : menu(get_text("Component testing"))
{
	int i = 0;
	if (onboard_tests.contains("Tests"))
	{
		for (const auto& item : onboard_tests["Tests"])
		{
			int last_success = item["LastSuccessTimestamp"];
			int last_fail = item["LastFailureTimestamp"];
			std::string visible_status;
			std::string visible_time;
			if (item["InProgress"])
			{
				visible_status = "W trakcie ...";
				visible_time = "";
			}
			else {
				if (last_success < 0 && last_fail < 0) {
					visible_status = "";
					visible_time = "";
				}
				else if (last_success > last_fail) {
					visible_status = "\nOK\n";
					visible_time = BasePlatform::DateTime(last_success).to_string_dd_mm_hh_ii();
				}
				else if (last_fail > last_success) {
					visible_status = "\nERROR\n";
					visible_time = BasePlatform::DateTime(last_fail).to_string_dd_mm_hh_ii();
				}
			}

			entries[i] = { get_text(item["Type"]) + visible_status + visible_time, "", [i]
			{
				write_command("ComponentTesting", std::to_string(i));
			}, false };
			i++;
		}
	}
	buildMenu();
}

void menu_tests::setEnabled(bool enabled)
{
	for (const auto& item : buttons)
		item->enabled = enabled;
}