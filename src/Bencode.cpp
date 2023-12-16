#include "Bencode.h"


bool Bencode::isNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

decodedPair Bencode::decodeBencodedValue(const std::string& encodedValue) {
    char first_value = encodedValue[0];

    if (std::isdigit(first_value)) {
        return decodedString(encodedValue);
    }
    else if (std::isalpha(first_value)) {
        if (first_value == 'i') {
            return decodedInteger(encodedValue);
        }
        else if (first_value == 'l') {
            return decodedList(encodedValue);
        }
        else if (first_value == 'd') {
            return decodedDictionary(encodedValue);
        }
        else {
            throw std::runtime_error("Unhandled encoded value: " + encodedValue);
        }
    }
    else {
        throw std::runtime_error("Unhandled encoded value: " + encodedValue);
    }
}

// Why cant make torrent const?
nlohmann::json Bencode::bencodeTorrent(Torrent torrent)
{
    std::stringstream ss;

    // Should probably generalise this algorithm
    ss << 'd';
    ss << "6:length";
    ss << 'i' << torrent.getTorrentInfo().length << 'e';
    ss << "4:name";
    ss << torrent.getTorrentInfo().name.length() << ':' << torrent.getTorrentInfo().name;
    ss << "12:piece length";
    ss << 'i' << torrent.getTorrentInfo().pieceLength << 'e';
    ss << "6:pieces";
    ss << torrent.getTorrentInfo().pieces.length() << ':' << torrent.getTorrentInfo().pieces;
    ss << 'e';

    /*
    std::string s = obj;

    for (int i = 0; i < obj.size(); i++) {
        std::string subString = s.substr(i, obj.size());

        if (std::isdigit(subString.front())) {
            ss << 'i';
            ss << subString.front();
            ss << 'e';
        }
        else if (std::isalpha(subString.front())) {
            // How to differentiate if content is in a string, list or dictionary?
        }
    }
     */

    return ss.str();
}

decodedPair Bencode::decodedString(const std::string& encodedValue) {
    size_t colon_index = encodedValue.find(':');
    if (colon_index != std::string::npos) {
        std::string number_string = encodedValue.substr(0, colon_index);
        int64_t number = std::atoll(number_string.c_str());
        std::string str = encodedValue.substr(colon_index + 1, number);
        return { nlohmann::json(str), number + colon_index + 1 };
    }
    else {
        throw std::runtime_error("Invalid encoded value: " + encodedValue);
    }
}

decodedPair Bencode::decodedInteger(const std::string& encodedValue) {
    std::size_t lastIndex = encodedValue.find_first_of("e");
    std::string intString = encodedValue.substr(1, lastIndex - 1); //-1 because we don't want to include the 'e' in the intString
    if (lastIndex != std::string::npos) {
        if (isNumber(intString)) { // Error handling so that content between i and e is indeed a digit, should maybe assert instead so that conversion doesn't exit the application if error?
            std::int64_t integer = std::atoll(intString.c_str());
            return { nlohmann::json(integer), lastIndex + 1 };
        }
    }
    else {
        throw std::runtime_error("Unahndled encoded value: " + encodedValue);
    }
}

decodedPair Bencode::decodedList(const std::string& encodedValue) {

    std::vector<nlohmann::json> result;
    std::string currentString = encodedValue.substr(1);

    while (currentString[0] != 'e') {
        auto decodedValue = decodeBencodedValue(currentString);
        result.push_back(decodedValue.value);
        currentString = currentString.substr(decodedValue.lastPos);
    }

    return { result, encodedValue.length() - currentString.length() + 1 };
}

decodedPair Bencode::decodedDictionary(const std::string& encodedValue) {
    std::string currentString = encodedValue.substr(1);
    std::map<std::string, nlohmann::json> map;

    while (currentString[0] != 'e') {
        auto decodedKeyPair = decodeBencodedValue(currentString);
        currentString = currentString.substr(decodedKeyPair.lastPos);

        auto decodedValuePair = decodeBencodedValue(currentString);
        currentString = currentString.substr(decodedValuePair.lastPos);

        map.emplace(decodedKeyPair.value, decodedValuePair.value);
    }

    return { map, encodedValue.length() - currentString.length() + 1 };
}