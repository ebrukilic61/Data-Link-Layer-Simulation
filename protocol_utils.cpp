#include "protocol_utils.h"

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
//gui kütüphaneleri de gelecek
using namespace std;

// sabitler
const string CRC_POLY = "10001000000100001"; // x^16 + x^12 + x^5 + 1
const string FRAME_FLAG = "01111110"; // Çerçeve başlangıç/bitiş işareti
const string CHECKSUM_HEADER = "CHECKSUM"; // Checksum paketi için özel başlık
const int FRAME_SIZE = 100; // Her bir çerçevenin bit cinsinden boyutu
const int TIMEOUT_MS = 2000; // Timeout süresi (milisaniye)

// hata simülasyonu olasılıkları
const double PACKET_LOSS_PROBABILITY = 0.10; // %10 paket kaybı
const double DATA_CORRUPTION_PROBABILITY = 0.20; // %20 veri bozulması
const double ACK_LOSS_PROBABILITY = 0.15; // %15 ACK kaybı
const double CHECKSUM_ERROR_PROBABILITY = 0.05; // %5 checksum hatası

// ACK ve NACK
const string ACK = "ACK";
const string NACK = "NACK";

// rastgele sayı üreteci ?? simülasyon kısmı için ai bunu önerdi ama biz başka bir şey düşünürüz
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> prob_dist(0.0, 1.0);

struct Frame {
    string data;
    string crc;
    int frameNumber;
    bool corrupted;
};

#define CRC_POLY "10001000000100001"

using namespace std;

//dosya okuma işlemlerini senderda mı yapmalıyız yoksa burada mı bilemedim, senderda yapmak daha uygun olabilir

string charToBinary(char c) {
    return bitset<8>(c).to_string();
}

long long int toDec(string binData) 
{ 
    long long int num = 0; 
    int i;
    for (i = 0; i < binData.length(); i++) { 
        if (binData.at(i) == '1') 
            num += 1 << (binData.length() - i - 1); 
    }
    return num; 
} 

string xorHesapla(string a, string b)
{
    string result = "";
    int len = a.length()
    int i;
    for(i = 0; i < len; i++)
    {
        result += (a[i] == b[i]) ? '0' : '1';
    }
    return result;
}

//CRC hesaplama: data ve key (polinom degeri) -> pol sbt ve binary halinde olacak
string crcHesapla(string data, string key)
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

// Checksum hesaplama
string checksum(string crcCode, int n) { //doğruluğunu kontrol etmem lazım, önceden yazmıştım bu kodu
    int overflow = 0;
    string firstPart = crcCode.substr(0, n);
    string secondPart = crcCode.substr(n, n);

    string result(n, '0');

    for (int i = n - 1; i >= 0; i--) {
        int bit1 = firstPart[i] - '0';  // '1' → 1, '0' → 0
        int bit2 = secondPart[i] - '0';
        
        int sum = bit1 + bit2 + overflow;
        result[i] = (sum % 2) + '0'; // binary olarak ekle ? chat neden böyle düzeltti?
        overflow = sum / 2;  //taşma varsa 1 olur
    }

    //overflow varsa + 1
    if (overflow == 1) {
        result = "1" + result;
    }

    // checksum'a +1 ekleme işlemi: 
    for (int i = result.length() - 1; i >= 0; i--) {
        if (result[i] == '0') {
            result[i] = '1';
            break; //exit mi kullanmalıyım break yerine?
        } else {
            result[i] = '0';
        }
    }

    if(result.length()>n){
        result = result.substr(result.length()-n, n); //taşan biti almamak için yazdım
    }

    return result;
}

// binary'den Hexadecimal'e dönüştürme
string binaryToHex(string binary) {
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
string applyBitStuffing(const string& data) {
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
string removeBitStuffing(const string& data) {
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

// checksum paketi oluşturma
string createChecksumPacket(const string& checksumValue) {
    
}

string corruptData(const string& data) { //simülasyon kısmı için datayı bozmamız lazım
    
}
