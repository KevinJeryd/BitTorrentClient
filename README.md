Proper read me and documentation will be added when project is done, currently implementing threading and optimising the client. 

# -- Overview of the BitTorrentClient --

## Pipeline

1. Parse metadata from torrent file
2. Construct tracker url from torrent data
3. Send GET request to the/a tracker to discover peers
4. Establish TCP connection with one or several of the peers discovered from the previous GET request
5. Download the file

## Terminology/Useful information

### BitTorrent
BitTorrent is a communication protocol used to share files directly between peers in a decentralised manner, this is called Peer-to-Peer communication or P2P. More concretely, what this means that there isn't a server hosting the files. Instead, every peer that possess the file essentially works as a "mini server" that can be downloaded from. Peers in the BitTorrent network connect to each other directly without the need for a central server. Each peer can act as both a downloader and an uploader, contributing to the distribution of the files.

### The torrent file
The torrent file is a so called "metadata file" that contains information about which files is to be distributed and the tracker (more on the tracker below), but not the actual data of the files to be shared. The sharing of this torrent file is what contributes to the sharing of the files on a peer computer.
Worth mentioning is that the information in a torrent file is "Bencoded", which is the serialisation format used by the BitTorrent Protocol and hence cannot be directly parsed, but firstly has to be dencoded. 

### Trackers
The tracker is a server that keeps track of the connected peers (users) and helps them discover each other. The tracker also keeps track of where file copies reside on the peer machines and which peers are available to download of at the time of the request. Note: Not all torrents use trackers; some torrents are "trackerless" and rely on a distributed hash table (DHT) or other methods for peer discovery. 

### The downloading of a file
BitTorrent makes use of "Piece-wise Downloading" which means that instead of downloading the file sequentially, the file is divided into several pieces, each which can be downloaded at any time and then combined to create the complete file. This allows for efficient utilisation of available bandwidth and enables users to start sharing the parts they've already downloaded.

### Choking and Optimistic Unchoking:
To optimize download speeds and encourage fair sharing, BitTorrent uses a mechanism called "choking." A peer will limit the number of connections it makes to other peers (choking them) based on their download/upload performance. Periodically, a peer will "optimistically unchoke" a connection, giving an opportunity to a new peer to download.

### Seeding
Seeding is the heart behind "torrenting" and the BitTorrent Protocol. Once a user has downloaded the entire file, they can choose to continue sharing it with others. This is known as "seeding," and it helps maintain the health of the BitTorrent network by ensuring a constant supply of upload capacity.
