#ifndef SENDER_H
#define SENDER_H

#include "protocol_utils.h"
#include <iostream>
#include <vector>
#include <string>

class Sender {
public:
    Sender();
    void sendData(const string& message); //frameleri sıraya koyup gönderme işlemi
    bool readFileAndCreateFrames(const string &fileName);//createFrames çağrılır, oluşan frame listesi alınır ve her bir frame gönderilir sendFrame de burada çağrılır her bir frame için
private:
    void sendFrame(const string& frameData, int frameNumber); //stop and wait protocolü
    vector<Frame> createFrames(const string& data);
};

#endif
