#ifndef PROTOCOL_UTILS_H
#define PROTOCOL_UTILS_H

#include <string>
#include <vector>

using namespace std;

#define FRAME_FLAG "01111110"  // HDLC Flag 0x7E bence buna checksumda da ihtiyaç duymayabiliriz

// Sabitler 
extern const double PACKET_LOSS_PROBABILITY;
extern const double DATA_CORRUPTION_PROBABILITY;
extern const double ACK_LOSS_PROBABILITY;
extern const double CHECKSUM_ERROR_PROBABILITY;

// ACK ve NACK sabitleri
extern const string ACK;
extern const string NACK;

struct Frame {
    string data;
    string crc;
    int frameNumber;
    string flag;
    string checksum;
};

class ProtocolUtils {
public:
    string charToBinary(char c);
    string xorHesapla(string a, string b);
    string crcHesapla(string data, string key);
    string checksum(string crcCode, int n);
    string binaryToHex(string binary);
    string applyBitStuffing(const string& data);
    string removeBitStuffing(const string& data);
    string createChecksumPacket(const string& checksumValue);
    //string corruptData(const string& data);
    Frame createFrame(const string& data, int frameNum);
    bool receiveFrame(Frame fr);
};

#endif