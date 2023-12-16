#pragma once
#define CURL_STATICLIB

#include "curl/curl.h"

#include <string>
#include "lib/nlohmann/json.hpp"
#include <iostream>
#include <sstream>
#include "Bencode.h"
#include "Torrent.h"

#define IS_BETWEEN(ch, low, high) (ch >= low && ch <= high)
#define IS_ALPHA(ch) (IS_BETWEEN(ch, 'A', 'Z') || IS_BETWEEN(ch, 'a', 'z'))
#define IS_DIGIT(ch) IS_BETWEEN(ch, '0', '9')
#define IS_HEXDIG(ch) (IS_DIGIT(ch) || IS_BETWEEN(ch, 'A', 'F') || IS_BETWEEN(ch, 'a', 'f'))

class Http {
private:
	static char hexToASCII(std::string s);
	static std::string urlEncode(const std::string& s);
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* buffer);
	static std::vector<std::string> getPeers(const std::string& trackerRes);
public:
	static std::string createTrackerURL(const std::string& trackerUrl, const std::string& inf_hash, const std::string& peerId, int64_t port, int64_t uploaded, int64_t downloaded, int64_t left, int64_t compact);
	static std::string sendGetReq(const std::string& url);
	static std::vector<std::string> extractPeers(nlohmann::json peerData);
};