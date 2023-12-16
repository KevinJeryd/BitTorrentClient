#include "Http.h"

// Maybe refactor some functions to a helper class of some sort
char Http::hexToASCII(std::string s) {
    return (char)(int)strtol(s.c_str(), nullptr, 16);
}

std::string Http::urlEncode(const std::string& s)
{
    std::ostringstream ss;

    for (int i = 0; i < s.size() - 1; i += 2)
    {
        char c1 = s[i];
        char c2 = s[i + 1];
        auto c3 = std::string(1, c1) + c2;

        char hexDig = hexToASCII(c3);

        if ((IS_ALPHA(hexDig) || IS_DIGIT(hexDig))) {
            ss << hexDig;
        }
        else {
            ss << "%" << c3;
        }
    }

    return ss.str();
}

std::string Http::createTrackerURL(
    const std::string& trackerUrl,
    const std::string& InfoHash,
    const std::string& peerId,
    int64_t port,
    int64_t uploaded,
    int64_t downloaded,
    int64_t left,
    int64_t compact) 
{

    std::ostringstream ss;

    std::string encodedInfoHash = urlEncode(InfoHash);

    ss << trackerUrl << "?"
        << "info_hash=" << encodedInfoHash << "&" // Don't hardcode, fix url encoding somehow
        << "peer_id=" << peerId << "&"
        << "port=" << port << "&"
        << "uploaded=" << uploaded << "&"
        << "downloaded=" << downloaded << "&"
        << "left=" << left << "&"
        << "compact=" << compact;

    return ss.str();
}

size_t Http::WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* buffer) {

    size_t newLength = size * nmemb;
    buffer->insert(buffer->end(), (char*)contents, (char*)contents + newLength);

    return newLength;

}

std::string Http::sendGetReq(const std::string& url)
{
    std::cout << "Discovering peers..." << std::endl;

    CURL* curl;
    CURLcode result;

    std::string response;

    curl = curl_easy_init();

    if (!curl) {
        std::cout << stderr << "HTTP Request Failed" << std::endl;
        exit(1);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    std::vector<char> responseBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    
    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::cout << stderr << ", Error: " << curl_easy_strerror(result) << std::endl;
        exit(1);
    }
    else {
        response.assign(responseBuffer.begin(), responseBuffer.end());
    }

    curl_easy_cleanup(curl);

    std::cout << "\n";

    return response;
}

std::vector<std::string> Http::getPeers(const std::string& trackerRes) {
    std::vector<std::string> peers;

    size_t len = trackerRes.length() / 6;

    for (auto i = 0; i < len; i++) {

        std::string peerString = trackerRes.substr(i * 6, 6);
        std::ostringstream out;

        for (int j = 0; j < 4; j++) {
            out << (int)(unsigned char)peerString[j];

            if (j != 3) {
                out << ".";
            }
            else {
                out << ":";
            }
        }

        uint16_t port = (uint8_t)peerString[5] | (uint8_t)peerString[4] << 8;
        out << port;

        peers.push_back(out.str());
    }

    return peers;

}

std::vector<std::string> Http::extractPeers(nlohmann::json trackerRes)
{
    nlohmann::json BencodedTracker = Bencode::decodeBencodedValue(trackerRes).value;
    std::string peersStr = BencodedTracker["peers"];

    std::vector<std::string> peers = getPeers(peersStr);

    if (!peers.empty()) {
        std::cout << "Peers found: " << std::endl;

        for (auto peer : peers) {
            std::cout << peer << std::endl;
        }

        std::cout << "\n";

        return peers;
    }
    else {
        std::cerr << "No peers found" << std::endl;
        return peers;
    }
}

