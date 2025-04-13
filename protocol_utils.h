#ifndef PROTOCOL_UTILS_H
#define PROTOCOL_UTILS_H

#include <string>
#include <vector>

using namespace std;

struct Frame {
    string data;
    string crc;
    int frameNumber;
    string flag;
    string checksum;
};

string charToBinary(char c);
string xorHesapla(string a, string b);
string crcHesapla(string data, string key);
string checksum(string crcCode, int n);
string binaryToHex(string binary);
string applyBitStuffing(const string& data);
string removeBitStuffing(const string& data);
string createChecksumPacket(const string& checksumValue);
string corruptData(const string& data);
Frame createFrame(const string& data, int frameNum);
bool receiveFrame(Frame fr);

#endif