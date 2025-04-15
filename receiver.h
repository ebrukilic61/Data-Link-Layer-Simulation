/*
#ifndef RECEIVER_H
#define RECEIVER_H

#include <vector>
#include <string>
#include "protocol_utils.h"

class Receiver {
public:
    Receiver();
<<<<<<< HEAD
    bool receiveFrame(const Frame& frame);   // CRC kontrolü yaparak ACK/NACK gönderir
    void finalizeReception();                // Bit ve karakter dosyası + checksum hesaplar

private:
=======
    void receiveFrame(const Frame& frameData);
   // bool verifyChecksum(const string& receivedChecksum, const vector<string>& crcList);
   void finalizeReception();
private:
    vector<Frame> receivedFrames;
    //vector<string> receivedCrc;
};

#endif*/

// receiver.h
#ifndef RECEIVER_H
#define RECEIVER_H

#include <vector>
#include <string>
#include "protocol_utils.h"

class Receiver {
public:
    Receiver();
    bool receiveFrame(const Frame& frame);   // CRC kontrolü yaparak ACK/NACK gönderir
    void finalizeReception();                // Bit ve karakter dosyası + checksum hesaplar

private:
>>>>>>> 17d2921ff034cf89ab316e430c76362377481623
    std::vector<Frame> receivedFrames;      // Alınan frame'leri saklamak için vektör
};

#endif