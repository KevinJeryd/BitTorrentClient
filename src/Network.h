#pragma once

#include <Winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "Http.h"
#include <fstream>
#include <algorithm>
#include "lib/sha1/sha1.hpp" // Move to utils instead
#include "utils.h"

#pragma comment(lib, "Ws2_32.lib")

class Network {
private:

	// Peer messages consist of a message length prefix (4 bytes), message id (1 byte) and a optional payload (variable size).
#pragma pack(push, 1)
	struct Msg {
		uint32_t length;
		uint8_t id;
	};
#pragma pack(pop)

	/*
    The message id for a request is 6.
    The payload consists of:
    index: the zero-based piece index
    begin: the zero-based byte offset within the piece - This'll be 0 for the first block, 2^14 for the second block, 2*2^14 for the third block etc.
	length: the length of the block in bytes - This'll be 2^14 (16 * 1024) for all blocks except the last one. The last block will contain 2^14 bytes or less.
	*/
#pragma pack(push, 1)
	struct ReqMsg {
		uint32_t length;
		uint8_t id;
		uint32_t index;
		uint32_t begin;
		uint32_t blockLength;
	};
#pragma pack(pop)

	static int initiateWinsock();
	static int connectToServer(SOCKET& socket, sockaddr_in& service, const int& addressFamily, const char*& ipAddress, const int& port);
	static int createSocket(SOCKET& socket, int addressFamily, int type, int protocol);
	static int sendMessage(SOCKET& socket, const char* message, const size_t& messageLength, const int& flags);
	static int receiveMessage(SOCKET& socket, char* message, int messageLength, int flags);
	static void createPeerHandshakeMsg(std::vector<char>& msg, const std::string& infoHash, const std::string& selfID);
	static void flushRecvBuffer(SOCKET& socket);
	static int connectToPeer(SOCKET& socket, const std::string& infoHash, const std::string& seldID, const std::string& peer);
	static int sendInterested(SOCKET& socket);
	static int receiveBitfieldMsg(SOCKET& socket, std::vector<int>& pieceIndices);
	static int receiveUnchokeMsg(SOCKET& socket);
	static int requestPiece(SOCKET& socket, const uint32_t& index, const uint32_t& begin, const uint32_t& blockLength);
	static std::string downloadPiece(const int& pieceIndex, std::ofstream& outfile, size_t& numPieces, const TorrentInfo& torrentInfo, SOCKET& socket, const std::string& outputPath);
	static int getPieceLength(SOCKET& socket);
	static int writePieceData(std::ofstream& outfile, const std::vector<char>& message, size_t& remaining, size_t& blockLength, size_t& offset, std::string& piece);
public:
	static int downloadPieces(const TorrentInfo& torrentInfo, const std::string& seldID, const std::string& peer, std::string& outputPath, std::vector<int>& downloadedPieces);
};