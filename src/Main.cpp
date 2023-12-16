#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include "lib/nlohmann/json.hpp"
#include "lib/sha1/sha1.hpp"
#include "Torrent.h"
#include "Bencode.h"
#include "Http.h"
#include "Network.h"

void initTorrent(Torrent& torrent, const std::string& torrentName) {
    std::cout << "Extracting torrent info..." << std::endl;

    nlohmann::json torrentContent = torrent.parseTorrentFileContent(torrentName); // Parse content from torrent file

    if (torrentContent.size() > 0) {
        // Fill torrent members with info
        torrent.setInfo(torrentContent);

        // B-Encode info dictionary
        const std::string bencodedInfo = Bencode::bencodeTorrent(torrent);
        torrent.sha1Encode(bencodedInfo);

        std::cout << "\n";
    }
    else {
        std::cout << "Error extracting torrent information." << std::endl;
    }
}

std::string constructUrlFromTorrent(TorrentInfo torrentInfo) {

    // Probably create struct for this
    int64_t port = 6881;
    int64_t uploaded = 0;
    int64_t downloaded = 0;
    std::string peerId = "00112233445566778899";
    int64_t left = torrentInfo.length;
    int64_t compact = 1;

    std::string val = Http::createTrackerURL(torrentInfo.url, torrentInfo.hash, peerId, port, uploaded, downloaded, left, compact);

    return val;
}

int main(int argc, char* argv[]) {
    std::string command = argv[1];
    // Initialise container for torrent information
    Torrent torrent;

    if (argc < 1) {
        std::cerr << "Usage: " << argv[0] << " <torrent>" << std::endl;
    }

    // TODO: Add input path into argv as -o or something
    // Todo check argv input

    const std::string torrentName = argv[1];
    initTorrent(torrent, torrentName);
    std::string url = constructUrlFromTorrent(torrent.getTorrentInfo());
    nlohmann::json response = Http::sendGetReq(url);
    std::vector<std::string> peers = Http::extractPeers(response); 
    const std::string peer = peers[0]; // Make so thread chooses a peer that isn't currently in use
    TorrentInfo torrentInfo = torrent.getTorrentInfo();
    std::string name = torrentInfo.name;
    std::cout << "TORRENT NAME: " << name << std::endl;
    const std::string selfID = "00112233445566778899";
    std::string path = "D:/Dev/BittorrentClient/"; // TODO: Get input from argument
    std::string fullPath = path.append(name);
    std::vector<int> downloadedPieces;

    Network::downloadPieces(torrentInfo, selfID, peer, fullPath, downloadedPieces);

    return 0;
}
