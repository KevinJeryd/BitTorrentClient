#include "Torrent.h"
#include "Bencode.h"

std::string readFile(std::string fileName) {
    std::ifstream file(fileName);
    std::string filedata;

    if (file.is_open()) {
        char character;
        uint16_t position = 0;
        while (file) {
            character = file.get();
            filedata.insert(position, 1, character);
            position++;
        }
    }

    file.close();

    return filedata;
}

// Maybe change naming convention to initTorrent?
void Torrent::setInfo(const nlohmann::json& torrentContent)
{
    info.url = torrentContent["announce"];
    info.length = torrentContent["info"]["length"];
    info.name = torrentContent["info"]["name"];
    info.pieceLength = torrentContent["info"]["piece length"];
    info.pieces = torrentContent["info"]["pieces"];
    info.pieceHashes = setPiecesHash(info.pieces);

    /*    
    for (auto pieceHash : info.pieceHashes) {
        std::cout << "TORRENT PIECEHASHES: " << pieceHash << std::endl;
    }

    std::cout << "\n";

    std::cout << "TORRENT PIECES: " << hexStr(info.pieces) << std::endl;
    */
}

std::vector<std::string> Torrent::setPiecesHash(const std::string& pieces)
{
    std::vector<std::string> res;
    int64_t len = pieces.length() / 20;

    for (int i = 0; i < len; i++) {
        res.push_back(hexStr(pieces.substr(i * 20, 20)));
    }

    return res;
}


// To represent the piece correctly
std::string Torrent::hexStr(const std::string& piece)
{
    std::ostringstream ret;

    for (std::string::size_type i = 0; i < piece.length(); i++) {
        ret << std::hex << std::setfill('0') << std::setw(2) << (int)(unsigned char)piece[i];
    }

    return ret.str();
}

nlohmann::json Torrent::parseTorrentFileContent(const nlohmann::json& torrentFile) {
    const std::string encodedTorrentInfo = readFile(torrentFile);

    // Dont know if I like Bencode reference here
    nlohmann::json decodedTorrentInfo = Bencode::decodeBencodedValue(encodedTorrentInfo).value;

    if (!decodedTorrentInfo.contains("announce") || !decodedTorrentInfo.contains("info")) {
        throw std::runtime_error("Invalid torrent file");
    }

    return decodedTorrentInfo;
}

void Torrent::sha1Encode(const nlohmann::json& encodedTorrent)
{
    SHA1 checksum;
    checksum.update(encodedTorrent);
    info.hash = checksum.final(); // To make this function more applicable to other stuff, move this somewhere else
}

void Torrent::printInfo()
{
    std::cout << "Tracker URL: " << info.url << std::endl;
    std::cout << "Length: " << info.length << std::endl;
    std::cout << "Info Hash: " << info.hash << std::endl;
    std::cout << "Piece Length: " << info.pieceLength << std::endl;
    std::cout << "Piece Hashes: " << std::endl;
    for (auto pieceHash : info.pieceHashes) {
        std::cout << pieceHash << std::endl;
    }
}

const TorrentInfo &Torrent::getTorrentInfo()
{
    return info;
}
