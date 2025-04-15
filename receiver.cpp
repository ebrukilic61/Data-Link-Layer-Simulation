#include "receiver.h"
#include "protocol_utils.h"
#include <iostream>   // cout ve endl için
#include <fstream>    // ofstream için
#include <bitset>     // bitset için

#define CRC_POLY "10001000000100001"
#define FRAME_SIZE 100
#define TIMEOUT 1000 

using namespace std;
Receiver::Receiver() {
    // Gerekli bir başlangıç işlemi varsa buraya ekleyebilirsiniz
    std::cout << "Receiver nesnesi olusturuldu." << std::endl;
}

bool Receiver::receiveFrame(const Frame& frame) {
    // ProtocolUtils ile CRC hesaplanacak ve kontrol edilecek
    ProtocolUtils pr;
    string dataToCheck = frame.isPadded ? frame.data.substr(0, frame.frameSize) : frame.data;
    string calculatedCRC = pr.crcHesapla(dataToCheck, CRC_POLY);

    // Frame’in alındığını ve CRC kontrolünü yapıyoruz
    cout << "Frame " << frame.frameNumber << " alindi. CRC kontrolu yapiliyor..." << endl;

    // Eğer CRC doğruysa ACK gönderiliyor, değilse NACK
    if (calculatedCRC == frame.crc) {
        cout << "CRC dogru. ACK gonderildi." << endl;
        receivedFrames.push_back(frame); // Doğru CRC ile gelen frame kaydediliyor
        return true; // ACK gönderildi
    } else {
        cout << "CRC hatali. NACK gonderildi." << endl;
        return false; // NACK gönderildi
    }
}

void Receiver::finalizeReception() {
    string allBits;
    Receiver receiver;
    // Bütün doğru gelen frame'lerin verisini birleştiriyoruz
    for (const auto& frame : receivedFrames) {
        allBits += frame.data;
    }

    // Bit dosyası oluşturuluyor
    ofstream bitFile("received_bits.txt");
    bitFile << allBits;
    bitFile.close();

    // Karakter dosyası oluşturuluyor
    ofstream charFile("received_chars.dat");
    for (size_t i = 0; i + 7 < allBits.length(); i += 8) {
        string byteStr = allBits.substr(i, 8);
        bitset<8> byteBits(byteStr);
        charFile << static_cast<char>(byteBits.to_ulong());
    }

    // Eğer son byte eksikse, kalan kısmı 0 ile tamamla
    size_t remainingBits = allBits.length() % 8;
    if (remainingBits > 0) {
        string lastByteStr = allBits.substr(allBits.length() - remainingBits);
        lastByteStr.append(8 - remainingBits, '0');  // eksik bitleri 0 ile doldur
        bitset<8> lastByteBits(lastByteStr);
        charFile << static_cast<char>(lastByteBits.to_ulong());
    }

    charFile.close();

    ProtocolUtils pr;

    // Checksum hesaplama
    string checks=pr.calculateChecksum(receiver.receivedFrames,receiver.receivedFrames.size());

    cout << "Tum frameler alindi." << endl;
    cout << "Checksum: " << checks << endl;
}