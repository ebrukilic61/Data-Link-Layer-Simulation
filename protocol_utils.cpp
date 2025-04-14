#include "protocol_utils.h"
#include "receiver.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>
#define CRC_POLY "10001000000100001" // x^16 + x^12 + x^5 + 1
//gui kütüphaneleri de gelecek
using namespace std;

// hata simülasyonu olasılıkları
const double PACKET_LOSS_PROBABILITY = 0.10; // %10 paket kaybı
const double DATA_CORRUPTION_PROBABILITY = 0.20; // %20 veri bozulması
const double ACK_LOSS_PROBABILITY = 0.15; // %15 ACK kaybı
const double CHECKSUM_ERROR_PROBABILITY = 0.05; // %5 checksum hatası

// ACK ve NACK
const string ACK = "ACK";
const string NACK = "NACK";

//dosya okuma işlemlerini senderda mı yapmalıyız yoksa burada mı bilemedim, senderda yapmak daha uygun olabilir

string ProtocolUtils::charToBinary(char c) {
    return bitset<8>(c).to_string();
}

string ProtocolUtils::xorHesapla(string a, string b)
{
    string result = "";
    int len = a.length();
    int i;
    for(i = 0; i < len; i++)
    {
        result += (a[i] == b[i]) ? '0' : '1';
    }
    return result;
}

//CRC hesaplama: data ve key (polinom degeri) -> pol sbt ve binary halinde olacak
string ProtocolUtils::crcHesapla(string data, string key)
{
    int keyLen = key.length();
    int i;
    string temp = data + string(keyLen-1,'0'); //key uzunluğu kadar 0 eklendi -> temp = bölünen data değeri + eklenen 0'lar

    string rem = temp.substr(0,keyLen); //rem = remainder
    for(i=keyLen; i<=temp.length(); i++)
    {
        if(rem[0] == '1'){
            rem = xorHesapla(rem, key);
        }
        rem = rem.substr(1);
        if(i<temp.length()){ //temp'in uzunlugu key.length olana kadar devam eder
            rem += temp[i];
        }
    }
    return rem;
}

string ProtocolUtils::binaryAddStrings(const string& a, const string& b) {
    string result = "";
    int carry = 0;
    int n = max(a.size(), b.size());

    string aPadded = string(n - a.size(), '0') + a;
    string bPadded = string(n - b.size(), '0') + b;

    for (int i = n - 1; i >= 0; i--) {
        int bitA = aPadded[i] - '0';
        int bitB = bPadded[i] - '0';
        int sum = bitA + bitB + carry;

        result = char((sum % 2) + '0') + result;
        carry = sum / 2; //taşma varsa 1 olur
    }

    if (carry == 1)
        result = '1' + result;

    return result;
}

/*
string ProtocolUtils::calculateChecksum(vector<Frame>fr,int frmNum){
    int i;
    int n = 8; //8 bit olmalı demişti hoca
    string concat_data = "";
    string sum = "00000000";
    
    for(int i = 0; i<frmNum-1;i++){
        concat_data += fr[i].crc;
    }
    concat_data += fr[frmNum-1].crc.substr(0,fr[frmNum-1].frameSize);

    //concat olan crc kodunu 8 bitlik parçalara bölük toplama işlemi:
    
    for(size_t i = 0; i<concat_data.size();i+=8){
        string chunk = concat_data.substr(i,8);
        if(chunk.length()<8){
            chunk = string(8 - chunk.length(), '0') + chunk; //padding gerekiyorsa başa (msb) 0 eklenir
        }
        sum = binaryAddStrings(sum, chunk);
    }

    sum = binaryAddStrings(sum, "00000001");

    if(sum.length()>8){
        sum = sum.substr(sum.length()-8,8);
    }
    return sum;
}
*/

string ProtocolUtils::calculateChecksum(vector<Frame> fr, int frmNum) {
    int n = 8;  // 8-bitlik parça
    string concat_data = "";
    string sum = "00000000"; // Başlangıçta sum 8-bit olarak 0

    // Tüm frame'lerin CRC'lerini concat_data'ya ekleyelim
    for (int i = 0; i < frmNum; i++) {
        concat_data += fr[i].crc.substr(0, fr[i].frameSize);  // CRC'yi frame boyutuna göre alıyoruz
    }

    // concat_data'yı 8-bitlik parçalara ayırıp toplama işlemi yapalım
    for (size_t i = 0; i < concat_data.size(); i += 8) {
        string chunk = concat_data.substr(i, 8);  // 8-bitlik parçayı alıyoruz
        if (chunk.length() < 8) {
            chunk = string(8 - chunk.length(), '0') + chunk;  // Eğer eksikse başa 0 ekleyip tamamlıyoruz
        }

        // sum ile bu 8-bitlik chunk'ı topluyoruz
        sum = binaryAddStrings(sum, chunk);
    }

    // Son adımda 1 ekliyoruz
    sum = binaryAddStrings(sum, "00000001");

    // Eğer toplamda 8 bitten uzun bir sonuç olduysa, sadece son 8 bit kalacak şekilde kesiyoruz
    if (sum.length() > 8) {
        sum = sum.substr(sum.length() - 8, 8);
    }

    return sum;
}

// binary'den Hexadecimal'e dönüştürme
string ProtocolUtils::binaryToHex(string binary) {
    stringstream ss;
    ss << hex << setfill('0');
    
    for(size_t i = 0; i < binary.length(); i += 4) {
        string nibble = binary.substr(i, 4);
        int decimal = 0;
        
        for(size_t j = 0; j < nibble.length(); j++) {
            if(nibble[j] == '1') {
                decimal += 1 << (nibble.length() - j - 1);
            }
        }
        
        ss << setw(1) << uppercase << decimal;
    }
    return ss.str();
}

// bit stuffing uygulama -> stuffing işlemleri için ai yardım etti, kontrol etmem lazım
string ProtocolUtils::applyBitStuffing(const string& data) {
    string result = "";
    int consecutive_bit = 0; //ardışık bitler için
    
    for(char bit : data) {
        result += bit;
        if(bit == '1') {
            consecutive_bit++;
            if(consecutive_bit == 5) {
                result += '0';
                consecutive_bit = 0;
            }
        } else {
            consecutive_bit = 0;
        }
    }
    
    return result;
}

// bit stuffing kaldırma
string ProtocolUtils::removeBitStuffing(const string& data) {
    string result = "";
    int consecutive_bit = 0;
    
    for(size_t i = 0; i < data.length(); i++) {
        char bit = data[i];
        result += bit;
        
        if(bit == '1') {
            consecutive_bit++;
            if(consecutive_bit == 5 && i + 1 < data.length() && data[i + 1] == '0') {
                i++; // stuffed bit'i atla
                consecutive_bit = 0;
            }
        } else {
            consecutive_bit = 0;
        }
    }
    return result;
}

Frame ProtocolUtils::createFrame(const string& data, int frameNum){
    Frame fr; //frame structını doldurdum
    fr.data = data;
    fr.frameNumber = frameNum;
    fr.crc = crcHesapla(data, CRC_POLY);
    return fr;
}
/*
bool ProtocolUtils::receiveFrame(Frame fr){
    string expectedCRC = crcHesapla(fr.data, CRC_POLY);

    if(expectedCRC == fr.crc){
        cout << "ack gonderiliyor" <<endl;
        Receiver rc;
        rc.ackNo = fr.frameNum;
        return true;
    }else{
        cout<<"receiver: crc hatali"<<endl;
        return false;
    }
}
*/
// checksum paketi oluşturma
string ProtocolUtils::createChecksumPacket(const string& checksumValue) {
    string packet = FRAME_FLAG + checksumValue + FRAME_FLAG; //FRAME_FLAG + CHECKSUM_HEADER + checksumValue + FRAME_FLAG; -> checksum header eklenmeli mi bilemedim?
    return packet;
}
