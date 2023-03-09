/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "train_data.h"
#include "fixed_train_data.h"
#include "keyboard.h"
#include "../tcp/server.h"
#include "../../EVC/Parser/nlohmann/json.hpp"
#include "../monitor.h"
#include "running_number.h"
#include <fstream>
using json = nlohmann::json;
fixed_train_data_window::fixed_train_data_window(std::string data) : input_window(gettext("Train Data"), 1, false), SelectType(gettext("Enter\ndata"),60,50)
{
    inputs[0] = new fixed_train_data_input();
    SelectType.setPressedAction([this]() {
        write_command("EnterData", "");
    });
    if (data != "")
    {
        inputs[0]->data = data;
        inputs[0]->setAccepted(true);
    }
    create();
}
void fixed_train_data_window::sendInformation()
{
    write_command("setAcceptedTrainData", inputs[0]->getData());
    //right_menu(new fixed_train_data_validation_window(inputs[0]->getData()));
}
void fixed_train_data_window::setLayout()
{
    input_window::setLayout();
    addToLayout(&SelectType, new RelativeAlignment(&exit_button, 246+30,25,0));
}
fixed_train_data_input::fixed_train_data_input(std::string data) : input_data(data)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    std::ifstream file(filesDir+"/traindata.txt");
#else
    std::ifstream file("../EVC/traindata.txt");
#endif
    json j;
    file >> j;
    vector<string> types;
    for (auto it = j.begin(); it!=j.end(); ++it) {
        types.push_back(it.key());
    }
    keys = getSingleChoiceKeyboard(types, this);
}
void fixed_train_data_input::validate()
{
    valid = true;
}
fixed_train_data_validation_window::fixed_train_data_validation_window(std::string data) : validation_window(gettext("Validate train data"), {new fixed_train_data_input(gettext("Train type"))}), data(data)
{
    validation_data[0]->data = data;
    validation_data[0]->setAccepted(true);
}