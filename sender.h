#ifndef SENDER_H
#define SENDER_H

#include <iostream>
#include <vector>
#include <string>
#include "protocol_utils.h"
#include "receiver.h"

using namespace std;
class Sender {
public:
    Sender();
    //void sendData(const string& message); //frameleri sıraya koyup gönderme işlemi
    bool readFileAndCreateFrames(const string &fileName);//createFrames çağrılır, oluşan frame listesi alınır ve her bir frame gönderilir sendFrame de burada çağrılır her bir frame için
    //void sendFrame(const string& frameData, int frameNumber); //stop and wait protocolü
    void sendFrame(int frameNumber, Receiver& receiver);
    void sendAllFrames(Receiver& receiver);
    //bool waitForACK(int frameNumber);
    //void simulateFrameTransmission(int frameNumber);
    vector<Frame> frames;
private:
    bool waitForACK(int frameNumber);
    string checksumValue;
    string checksumHex;
    bool allFramesSent;
    int currentFrameIndex;
};

#endif