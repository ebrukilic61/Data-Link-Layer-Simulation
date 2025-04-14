#ifndef SENDER_H
#define SENDER_H

#include <iostream>
#include <vector>
#include <string>
#include "protocol_utils.h"

using namespace std;
class Sender {
public:
    Sender();
    //void sendData(const string& message); //frameleri sıraya koyup gönderme işlemi
    bool readFileAndCreateFrames(const string &fileName);//createFrames çağrılır, oluşan frame listesi alınır ve her bir frame gönderilir sendFrame de burada çağrılır her bir frame için
    //void sendFrame(const string& frameData, int frameNumber); //stop and wait protocolü
    void sendFrame(int frameNumber);
    vector<Frame> createFrame(const string& data);
    //string crcHesapla(string data, string key);
    //string checksum(string crcCode, int n);
    //string xorHesapla(string a, string b);

    vector<Frame> frames;

private:
    bool waitForACK(int frameNumber);
    //bool simulateSendFrame(const Frame& frame);
    string checksumValue;
    string checksumHex;
    bool allFramesSent;
    int currentFrameIndex;
};

#endif