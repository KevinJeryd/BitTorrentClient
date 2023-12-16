#include "Network.h"

int Network::initiateWinsock()
{
    // Load DLL
    WSADATA wsaData;
    int wsaErr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int port = 55555;

    wsaErr = WSAStartup(wVersionRequested, &wsaData);

    if (wsaErr != 0) {
        std::cout << "Winsock dll could not be found." << std::endl;
        return 0;
    }
    else {
        std::cout << "Winsock dll found." << std::endl;
        std::cout << "Status: " << wsaData.szSystemStatus << std::endl;
        return 1;
    }
}

int Network::createSocket(SOCKET& pSocket, int addressFamily, int type, int protocol)
{
    pSocket = socket(addressFamily, type, protocol);

    if (pSocket == INVALID_SOCKET) {
        std::cout << "Error creating socket." << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }
    else {
        std::cout << "Socket successfully created." << std::endl;
        return 1;
    }
}

int Network::connectToServer(SOCKET& socket, sockaddr_in& service, const int& addressFamily, const char*& ipAddress, const int& port) {
    service.sin_family = addressFamily;
    service.sin_port = htons(port); 

    if (InetPton(addressFamily, ipAddress, &service.sin_addr.S_un) <= 0) {
        std::cerr << "Invalid address/Address not support" << std::endl;
    }

    if (connect(socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "Connection to server failed." << std::endl;
        WSACleanup();
        return 0;
    }
    else {
        std::cout << "Succesfully connected to server." << std::endl;
        std::cout << "Can start sending and receiving data..." << std::endl;
        return 1;
    }
}

int Network::sendMessage(SOCKET& socket, const char* message, const size_t& messageLength, const int& flags)
{
    int byteCount = send(socket, message, messageLength, flags);

    if (byteCount == SOCKET_ERROR) {
        std::cout << "Send error: " << WSAGetLastError() << std::endl;
        return 0;
    }
    else {
        return byteCount;
    }
}

/*
    socket - The socket communicating with a peer.
    messageBuffer - Buffer for saving the received messaged.
    messageLength - How many bytes that's going to be received. This may not be the complete message and therefore a flush of the buffer may be in place before succeeding receives.
    flags - Specific flags for the recv function's behaviour, read more https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv.
*/
int Network::receiveMessage(SOCKET& socket, char* messageBuffer, int messageLength, int flags)
{
    int byteCount = recv(socket, messageBuffer, messageLength, flags);

    if (byteCount == SOCKET_ERROR) {
        std::cout << "Error receiving message: " << WSAGetLastError() << std::endl;
        return 0;
    }
    else {
        return byteCount;
    }
}

void setSocketNonBlocking(SOCKET& socket) {
    u_long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
}

void setSocketBlocking(SOCKET& socket) {
    u_long mode = 0;
    ioctlsocket(socket, FIONBIO, &mode);
}

void Network::flushRecvBuffer(SOCKET& socket) {
    setSocketNonBlocking(socket);

    const int bufferSize = 1;
    char buffer[bufferSize];

    while (true) {
        int bytesRead = recv(socket, buffer, bufferSize, 0);

        if (bytesRead > 0) {
            // Data received, you can process or ignore it
            // If you want to print it, you can uncomment the following line
            // std::cout.write(buffer, bytesRead);
        }
        else if (bytesRead == 0) {
            // No more data, the other side has closed the connection
            break;
        }
        else {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                // No more data available, break out of the loop
                break;
            }
            else {
                // Other error handling, for simplicity we print an error message here
                std::cerr << "Error: " << error << std::endl;
                break;
            }
        }
    }

    // Reset the socket to blocking mode if needed
    setSocketBlocking(socket);
}

void Network::createPeerHandshakeMsg(std::vector<char>& msg, const std::string& infoHash, const std::string& selfID)
{
    char protocolLength = 19;
    std::string protocol = "BitTorrent protocol";

    msg.push_back(protocolLength);
    msg.insert(msg.end(), protocol.begin(), protocol.end());

    //eight reserved bytes
    for (int i = 0; i < 8; ++i) {
        msg.push_back(0);
    }
    
    msg.insert(msg.end(), infoHash.begin(), infoHash.end());
    msg.insert(msg.end(), selfID.begin(), selfID.end());
}

int Network::connectToPeer(SOCKET& clientSocket, const std::string& infoHash, const std::string& selfID, const std::string& peer) {
    sockaddr_in clientService;
    std::vector<char> msg;
    size_t divider = peer.find(":");
    const std::string peerIP = peer.substr(0, divider);
    const char* cPeerIP = &peerIP[0]; // Better way to do this?
    const int peerPort = stoi(peer.substr(divider + 1, peer.size()));

    createSocket(clientSocket, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connectToServer(clientSocket, clientService, AF_INET, cPeerIP, peerPort);
    createPeerHandshakeMsg(msg, infoHash, selfID);
    sendMessage(clientSocket, msg.data(), msg.size(), 0);

    std::vector<char> handshakeResp(msg.size());
    receiveMessage(clientSocket, handshakeResp.data(), handshakeResp.size(), MSG_WAITALL);

    if (!handshakeResp.empty()) {
        std::string resp(handshakeResp.end() - 20, handshakeResp.end());
        std::cout << "Peer ID: " << utils::binToHex(resp) << std::endl;
        return 1;
    }
    else {
        std::cerr << "Couldn't connect to peers" << std::endl;
        return 0;
    }
}

int Network::sendInterested(SOCKET& socket) {
    Msg interestMsg {htonl(1), 2};
    if (sendMessage(socket, (char*) &interestMsg, sizeof(interestMsg), 0)) {
        return 1;
    }
    else {
        return 0;
    }
}

// Returns which pieces a peer has
int Network::receiveBitfieldMsg(SOCKET& socket, std::vector<int>& pieceIndices) {
    std::array<char, 4> bitfieldPayloadLen = {};
    char bitfieldID;
    receiveMessage(socket, (char*) &bitfieldPayloadLen, 4, MSG_WAITALL);
    receiveMessage(socket, &bitfieldID, 1, MSG_WAITALL);
    size_t payloadLen = ntohl(*reinterpret_cast<u_long*>(bitfieldPayloadLen.data())) - 1;

    std::cout << "PayloadLength: " << payloadLen << std::endl;
    std::cout << "BITFIELD ID: " << (int) bitfieldID << std::endl;

    std::vector<char> bitfieldPayload(payloadLen);
    receiveMessage(socket, bitfieldPayload.data(), bitfieldPayload.size(), MSG_WAITALL);
    flushRecvBuffer(socket); // Just to ensure that nothing is left over for next call

    if (!bitfieldPayload.empty()) {

        if (bitfieldID != 5) {
            // Debug
            std::cerr << "Fatal error, wrong ID." << std::endl;
            return 0;
        }
        else {
            // TODO: Fix so peers without all pieces can send
            // Store which pieces it has in a vector maybe?
            // Then send pieceIndex to downloadPiece function
            // For threads, maybe include which peer has what pieces and have a map or work queue that threads can pick and pop from
            // Probably include if the piece from a piece is taken also, something like
            // [<peerIP:peerPort> - {1, 3, 4}, <peer2IP:peer2Port> - {2, 3, 5} etc], if it takes 3 it will have to delete it from all peers that have the piece 3 since it shouldn't be downloaded twice
            // If a pieceIndex exist it means that it's currently not being downloaded

            for (int i = 0; i < bitfieldPayload.size(); i++) {
                for (int j = 0; j < 8; j++) {
                    if (((bitfieldPayload[i] << j) & 0b10000000)) {
                        pieceIndices.push_back((j) + i*8);
                    }
                }
            }

            std::cout << "PiecesIndices: ";
            for (int pieceIndex : pieceIndices) {
                std::cout << pieceIndex;
            }

            std::cout << std::endl;
        }

        return 1;
    } 
    else {
        return 0;
    }
}

// Function is similar to receiveBitfieldMsg
// Could potentially create a 'receivePeerMsg' instead and take in the message as parameters instead of creating in function
// Note: Different ids, so expected ID will have to be passed into function or received ID will have to be returned for error checking
int Network::receiveUnchokeMsg(SOCKET& socket) {
    std::vector<char> unchokeMsgLen(4);
    char unchokeID;
    receiveMessage(socket, unchokeMsgLen.data(), unchokeMsgLen.size(), MSG_WAITALL);
    receiveMessage(socket, &unchokeID, 1, MSG_WAITALL);
    flushRecvBuffer(socket);

    if (unchokeID != 1) {
        // Debug
        std::cerr << "Fatal error, wrong ID" << std::endl;
        return 0;
    }
    else {
        std::cout << "Unchoke message received successfully" << std::endl;
        return 1;
    }
}

/*
    socket - The socket communicating with a peer.
    index - The piece that is requested.
    offset - Since the piece is downloaded in parts, offset represents how far into the current piece we are.
    blockLength - The size of the part of the piece we want to download, will be 16kiB for all except last block.
*/
int Network::requestPiece(SOCKET& socket, const uint32_t& index, const uint32_t& offset, const uint32_t& blockLength) {
    ReqMsg reqMsg = { htonl(13), 6, htonl(index), htonl(offset), htonl(blockLength) };
    if (sendMessage(socket, (char*)&reqMsg, sizeof(reqMsg), 0)) {
        return 1;
    }
    else {
        return 0;
    }
}

int Network::getPieceLength(SOCKET& socket) {
    // Receiving piece message
    std::vector<char> lengthBuffer(4);
    int bytesReceived = Network::receiveMessage(socket, lengthBuffer.data(), lengthBuffer.size(), 0);

    if (bytesReceived != 4) {
        std::cerr << "Error receiving message length or incomplete read: " << bytesReceived << std::endl;
    }

    return (ntohl(*reinterpret_cast<u_long*>(lengthBuffer.data())));
}

int Network::writePieceData(std::ofstream& outfile, const std::vector<char>& message, size_t& remaining, size_t& blockLength, size_t& offset, std::string& piece) {
    if (message[0] == 7) {
        // Extract block data from message
        std::vector<char> receivedBlock(message.begin() + 9, message.end()); // Skip 1 byte of ID, 4 bytes of index, 4 bytes of begin
        outfile.write(receivedBlock.data(), receivedBlock.size());
        piece.append(receivedBlock.data(), receivedBlock.size());

        // Update remaining and offset
        remaining -= blockLength;
        offset += blockLength;

        // Check if this was the last block
        if (remaining == 0) {
            return 1;
        }

        return 0;
    }

    return 0;
}

// Want to return the piece downloaded so I can verify the integrity.
// Probably not efficient to save them all down and then verify, so might verify each piece as they come and if it doesn't match I'll cancel the download.
// TODO: Refactor function into smaller pieces
std::string Network::downloadPiece(const int& pieceIndex, std::ofstream& outfile, size_t& numPieces, const TorrentInfo& torrentInfo, SOCKET& socket, const std::string& outputPath) {
    outfile.open(outputPath, std::ios::app);
    std::string piece{};

    // Determine the size of the current piece, either take the full piece length or if it's the last piece take the remainder of what's left
    size_t currentPieceSize = (pieceIndex == numPieces - 1) ? (torrentInfo.length % torrentInfo.pieceLength) : torrentInfo.pieceLength;
    size_t pieceLength = 16384; // 16kiB

    if (currentPieceSize == 0) {
        currentPieceSize = torrentInfo.pieceLength;  // Handle case where file size is an exact multiple of piece length
    }

    size_t remaining = currentPieceSize;  // Remaining data to download for the current piece
    size_t offset = 0;  // Offset within the piece

    while (remaining > 0) {
        int totalBytesReceived = 0;
        size_t blockLength = min(pieceLength, remaining);

        Network::requestPiece(socket, pieceIndex, offset, blockLength);
        int messageLength = Network::getPieceLength(socket);
        std::vector<char> message(messageLength);

        while (totalBytesReceived < messageLength) {
            int bytesReceived = recv(socket, message.data() + totalBytesReceived, messageLength - totalBytesReceived, 0);

            if (bytesReceived <= 0) {
                std::cerr << "Error receiving message or connection closed" << std::endl;
                break;
            }

            totalBytesReceived += bytesReceived;
        }

        if (Network::writePieceData(outfile, message, remaining, blockLength, offset, piece)) {
            break; // Last block of data has been received, break out of loop
        }
    }

    outfile.close();

    return piece;
}

// Creates a connection to a peer, gathers what pieces the peer has and downloads them.
int Network::downloadPieces(const TorrentInfo& torrentInfo, const std::string& selfID, const std::string& peer, std::string& outputPath, std::vector<int>& downloadedPieces)
{
    std::cout << "Initialising connection with peer:" << std::endl;
    std::vector<int> pieceIndices;

    Network::initiateWinsock();
    const std::string infoHash = utils::hexToString(torrentInfo.hash);
    SOCKET clientSocket;

    if (!Network::connectToPeer(clientSocket, infoHash, selfID, peer)) {
        std::cerr << "Connection failed, restart the program." << std::endl;
        // Should maybe exit program
    }

    if (!Network::receiveBitfieldMsg(clientSocket, pieceIndices)) {
        std::cerr << "Error receiving bitfield." << std::endl;
        // Should maybe exit program
    }

    if (!Network::sendInterested(clientSocket)) {
        std::cerr << "Could not send peer message \"Interested\"" << std::endl;
        // Should maybe exit program
    }

    if (!Network::receiveUnchokeMsg(clientSocket)) {
        std::cerr << "Error receiving unchoke message." << std::endl;
        // Should maybe exit program
    }

    // TODO: Should probably notify if file already exist and ask if they want to overwrite
    std::ofstream outfile;
    outfile.open(outputPath, std::ofstream::out | std::ofstream::trunc); // Clear file so it doesn't append on a file that might exist
    outfile.close();

    size_t pieceLength = 16384; // 16kiB
    size_t numPieces = (torrentInfo.length + torrentInfo.pieceLength - 1) / torrentInfo.pieceLength; // + pieceLength - 1 is to round result up
    
    for (size_t pieceIndex = 0; pieceIndex < pieceIndices.size(); pieceIndex++) {
        if (std::find(downloadedPieces.begin(), downloadedPieces.end(), pieceIndices[pieceIndex]) == downloadedPieces.end()) { // Ensure piece hasn't already been downloaded
            std::string piece = Network::downloadPiece(pieceIndices[pieceIndex], outfile, numPieces, torrentInfo, clientSocket, outputPath);
        
            // Verifying piece integrity
            SHA1 checksum;
            checksum.update(piece);
            std::string pieceHash = checksum.final();
     
            if (pieceHash != torrentInfo.pieceHashes[pieceIndices[pieceIndex]]) {
                std::cout << "Fatal error, piecehash not matching, cannot very integrity of file!" << std::endl;
                break;
            }

            downloadedPieces.push_back(pieceIndices[pieceIndex]);
        }
    }

    std::cout << "File succesfully downloaded to: " << outputPath << std::endl;

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}