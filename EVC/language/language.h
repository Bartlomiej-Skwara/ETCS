#pragma once
#include <string>
std::string get_text(std::string id);
std::string get_text_context(std::string context, std::string id);
void set_language(std::string name);
void load_language();
extern std::string language;