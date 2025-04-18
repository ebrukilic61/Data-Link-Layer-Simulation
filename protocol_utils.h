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
    string crc; // data ve crc arasına flag konulmuş
    string frameNumber; // 0 veya 1
    int fNum;
    string checksum;
    size_t frameSize; //burada orijinal sizedan bahsediyorum, padding durumunu gözetmek için yazıldı -> sadece crc hesaplamak için
    bool isPadded;
    string result;
    string addressRec; //receiver adresi -> gerek yok, tek yönlü iletişim yapıyoruz, bunu çıkarabiliriz
    string addressSend; //sender adresi
};

class ProtocolUtils {
public:
    string charToBinary(char c);
    string xorHesapla(string a, string b);
    string crcHesapla(string data, string key);
    //string checksum(string crcCode, int n);
    string binaryAddStrings(const string& a, const string& b);
    string calculateChecksum(vector<Frame>fr,int frameSize); //yeni checksum kodu
    string binaryToHex(string binary);
    string applyBitStuffing(const string& data);
    string removeBitStuffing(const string& data);
    string createChecksumPacket(const string& checksumValue);
    //string corruptData(const string& data);
    Frame createFrame(const string& data, int frameNum);
    bool receiveFrame(Frame fr);
};

#endif