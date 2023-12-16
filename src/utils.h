#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>

class utils {
public:
	static std::string hexToString(const std::string& in);
	static std::string binToHex(const std::string& bin);
};