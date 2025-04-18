#include "receiver.h"
#include "protocol_utils.h"
#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <string>

#define CRC_POLY "10001000000100001"
#define FRAME_SIZE 100
#define TIMEOUT 1000
#define FRAME_FLAG "01111110"

using namespace std;

Receiver::Receiver() {
    cout << "Receiver nesnesi olusturuldu." << endl;
}

bool Receiver::receiveFrame(string fullFrame) {
    ProtocolUtils pr;
    Frame frame;
    string receivedData;
    string receivedCRC;
    string senderAddr;
    string frameNum;
    
    size_t flagPos = fullFrame.find(FRAME_FLAG);

    if(flagPos != string::npos){
        senderAddr = fullFrame.substr(0,7);
        frameNum = fullFrame.substr(7,1);
        receivedData  = fullFrame.substr(8, flagPos-8);
        receivedCRC = fullFrame.substr(flagPos+8);
        cout<<"sender address: "<<senderAddr<<endl;
        cout<<"frame number: "<<frameNum<<endl;
        cout <<"data: "<<receivedData<<endl;
        cout<<"received crc: "<<receivedCRC<<endl;
    }else{
        cout<<"flag yok datada"<<endl;
    }

    string dataUnstuff = pr.removeBitStuffing(receivedData);

    string calculatedCRC = pr.crcHesapla(receivedData, CRC_POLY);

    frame.result = receivedData;
    frame.crc = calculatedCRC;
    frame.data = dataUnstuff;
    frame.frameSize = dataUnstuff.length();
    frame.isPadded = false;
    frame.frameNumber = "0";
    cout <<"calculated crc: "<<calculatedCRC<<endl;
    cout << "Frame alindi. CRC kontrolu yapiliyor..." << endl;

    if (calculatedCRC == receivedCRC) {
        cout << "CRC dogru. ACK gonderildi." << endl;
        receivedFrames.push_back(frame);
        return true;
    } else {
        cout << "CRC hatali. NACK gonderildi." << endl;
        return false;
    }
}

void Receiver::finalizeReception() {
    if (receivedFrames.empty()) {
        cout << "Hic frame alinmadi!" << endl;
        return;
    }
    
    cout << "\n=== Alinan verilerin islenmesi ===\n" << endl;
    cout << "Toplam alinan frame sayisi: " << receivedFrames.size() << endl;
    
    // Tüm frame'lerin verilerini birleştir
    string allBits;
    for (size_t i = 0; i < receivedFrames.size(); i++) {
        const Frame& frame = receivedFrames[i];
        
        // Padded frame ise sadece gerçek veriyi al
        if (frame.isPadded) {
            allBits += frame.data.substr(0, frame.frameSize); //unstuffed datayı ekliyoruz
        } else {
            allBits += frame.data;
        }
        
        cout << "Frame " << i << " verisi eklendi (boyut: " << frame.frameSize << " bits)" << endl;
    }
    
    cout << "Toplam bit sayisi: " << allBits.length() << endl;
    
    // Bit verilerini dosyaya yaz
    ofstream bitFile("received_bits.txt");
    if (bitFile.is_open()) {
        bitFile << allBits;
        bitFile.close();
        cout << "Bit verisi 'received_bits.txt' dosyasina yazildi." << endl;
    } else {
        cerr << "Bit dosyasi olusturulamadi!" << endl;
    }
    
    // Bitleri karakterlere dönüştür ve binary dosyaya yaz
    ofstream charFile("received_chars.dat", ios::binary);
    if (charFile.is_open()) {
        // Her 8 bit bir byte olarak işlenir
        for (size_t i = 0; i + 7 < allBits.length(); i += 8) {
            string byteStr = allBits.substr(i, 8);
            
            // 8 bitlik string'i byte'a dönüştür (LSB -> MSB sıralaması ile)
            bitset<8> bits;
            for (int j = 0; j < 8; j++) {
                bits[j] = (byteStr[7-j] == '1');
            }
            
            char byte = static_cast<char>(bits.to_ulong());
            charFile.write(&byte, 1);
        }
        
        // Kalan bitleri işle (8'in katı değilse)
        size_t remainingBits = allBits.length() % 8;
        if (remainingBits > 0) {
            string lastByteStr = allBits.substr(allBits.length() - remainingBits);
            lastByteStr.append(8 - remainingBits, '0');  // Sıfırlarla doldur
            
            bitset<8> bits;
            for (int j = 0; j < 8; j++) {
                bits[j] = (j < lastByteStr.length() && lastByteStr[7-j] == '1');
            }
            
            char byte = static_cast<char>(bits.to_ulong());
            charFile.write(&byte, 1);
        }
        
        charFile.close();
        cout << "Karakter verisi 'received_chars.dat' dosyasina yazildi." << endl;
    } else {
        cerr << "Karakter dosyasi olusturulamadi!" << endl;
    }
    
    // Checksum hesapla ve kontrol et
    ProtocolUtils pr;
    string checksum = pr.calculateChecksum(receivedFrames, receivedFrames.size());
    
    cout << "Alinan framelerin checksum degeri: " << checksum << endl;
    cout << "islem tamamlandi." << endl;
}