#include "utils.h"

std::string utils::hexToString(const std::string& in) {

    std::string output;

    if ((in.length() % 2) != 0) {
        throw std::runtime_error("String is not valid length ...");
    }

    size_t cnt = in.length() / 2;

    for (size_t i = 0; cnt > i; ++i) {
        uint32_t s = 0;
        std::stringstream ss;
        ss << std::hex << in.substr(i * 2, 2);
        ss >> s;
        output.push_back(static_cast<unsigned char>(s));
    }
    return output;
}

std::string utils::binToHex(const std::string& bin) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (unsigned char c : bin) {
        ss << std::setw(2) << static_cast<unsigned>(c);
    }

    return ss.str();
}