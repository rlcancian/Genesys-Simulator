#include <netinet/in.h>
#include <vector>

#ifndef UTILS_H
#define UTILS_H

class SocketData {
public:
    int _id;
    unsigned int _seed;
    int _replicationNumber;
    struct sockaddr_in _address;
    int _socket;
};

enum DistributedCommunication {
    INIT_CONNECTION = 1,
    SEND_MODEL = 2,
    RECEIVE_DATA = 3,
    RESULTS = 4,
    CLOSE_CONNECTION = 5,
    NOTHING = 6,
    FAILURE = 7,
    BENCHMARK = 8
};

struct ResultPayload {
    DistributedCommunication code;
    int threadId;
    std::vector<double> results;
};

#endif