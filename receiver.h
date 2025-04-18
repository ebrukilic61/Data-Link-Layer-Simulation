#ifndef RECEIVER_H
#define RECEIVER_H

#include <vector>
#include <string>
#include "protocol_utils.h"

class Receiver {
public:
    Receiver();
    bool receiveFrame(string fullFrame);   // CRC kontrolü yaparak ACK/NACK gönderir
    void finalizeReception();                // Bit ve karakter dosyası + checksum hesaplar

private:
    std::vector<Frame> receivedFrames;      // Alınan frame'leri saklamak için vektör
};

#endif