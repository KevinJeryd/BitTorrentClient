#pragma once
#include <string>
#include <stdint.h>
#include <vector>
#include "lib/nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include "lib/sha1/sha1.hpp"


/*
url - Torrent url
length - Size of file in bytes
pieceLength - Number of bytes per piece
pieces - concatenated SHA-1 hashes of each piece
name - Name of file
hash - Unique identifier for the torrent file
pieceHashes - Inidividual hash for each piece
*/
struct TorrentInfo {
	std::string url;
	int64_t length{};
	int64_t pieceLength{};
	std::string pieces{};
	std::string name{};
	std::string hash{};
	std::vector<std::string> pieceHashes{};
};

class Torrent {
private:
	TorrentInfo info;
	std::vector<std::string> setPiecesHash(const std::string& pieces);
	std::string hexStr(const std::string& piece);
public:
	nlohmann::json parseTorrentFileContent(const nlohmann::json& torrentFile);
	void setInfo(const nlohmann::json& torrentContent);
	void sha1Encode(const nlohmann::json& encodedTorrent);
	void printInfo();
	const TorrentInfo &getTorrentInfo();
};