#pragma once
#include "lib/nlohmann/json.hpp"
#include <sstream>
#include "Torrent.h"

struct decodedPair {
	nlohmann::json value{};
	size_t lastPos{};
};

class Bencode {
private:
	static decodedPair decodedString(const std::string& encodedValue);
	static decodedPair decodedInteger(const std::string& encodedValue);
	static decodedPair decodedList(const std::string& encodedValue);
	static decodedPair decodedDictionary(const std::string& encodedValue);
	static bool isNumber(const std::string& s);
public:
	static decodedPair decodeBencodedValue(const std::string& encodedValue);
	static nlohmann::json bencodeTorrent(const Torrent torrent);
};