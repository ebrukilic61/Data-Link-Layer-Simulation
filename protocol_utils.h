#ifdef PROTOCOL_UTILS_H
#define PROTOCOL_UTILS_H

#include <string>
#include <vector>

using namespace std;

string charToBinary(char c);
string xorHesapla(string a, string b);
string crcHesapla(string data, string key);
string checksum(vector<string> crcKodu);
string binaryToHex(string binary); //bunu da vectorden dönüştürmem gerekebilir
string applyBitStuffing(const string& data)
string removeBitStuffing(const string& data);
string createChecksumPacket(const string& checksumValue);
string corruptData(const string& data);

#endif