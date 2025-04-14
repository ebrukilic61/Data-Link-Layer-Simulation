#ifndef RECEIVER_H
#define RECEIVER_H

#include "protocol_utils.h"
#include <vector>
#include <string>

class Receiver {
public:
    Receiver();
    void receiveFrame(const string& frameData);
    bool verifyChecksum(const string& receivedChecksum, const vector<string>& crcList);
private:
    vector<Frame> receivedFrames;
    vector<string> receivedCrc;
};

#endif
