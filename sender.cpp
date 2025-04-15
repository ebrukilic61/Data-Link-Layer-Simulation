#include "sender.h"
#include "receiver.h"
#include "protocol_utils.h"
#include <iostream>
#include <fstream>
#include <bitset>
#include <chrono>
#include <thread>

#define CRC_POLY "10001000000100001"
#define FRAME_SIZE 100
#define TIMEOUT 1000 

using namespace std;

Sender::Sender() : allFramesSent(false), currentFrameIndex(0) {}

bool Sender::readFileAndCreateFrames(const string &fileName) {
    int i;
    ifstream file;
    frames.clear();
    string currentFrame;  
    char byte;

    file.open(fileName.c_str(), ios::binary); 

    if (!file) {
        cerr << "Dosya acilamadi!" << endl;
        return false;
    }  

    while (file.read(&byte, 1)) {
        bitset<8> bits(byte); 
        for (i = 7; i >= 0; i--) {
            currentFrame.push_back(bits[i] ? '1' : '0');
        }

        if (currentFrame.size() >= FRAME_SIZE) {
            Frame f;
            f.data = currentFrame.substr(0, FRAME_SIZE);
            f.frameSize = FRAME_SIZE;
            f.isPadded = false;
            frames.push_back(f);
            currentFrame = currentFrame.substr(FRAME_SIZE);
        }
    }
    
    if (!currentFrame.empty()) { // son frame tamamlanmamışsa
        Frame f;
        f.frameSize = currentFrame.size();
        f.isPadded = true;
        while (currentFrame.size() < FRAME_SIZE) {
            currentFrame.push_back('0'); // eksik kalan bitler 0 ile tamamlanır
        }
        
        f.data = currentFrame;
        frames.push_back(f);
    }

    cout << "Toplam " << frames.size() << " frame olusturuldu." << endl;
    for (i = 0; i < frames.size(); i++) {
        cout << "Frame " << i << " hazirlandi. Boyut: " << frames[i].frameSize << " bit" << endl;
    }

    file.close();
    return true;
}
/*
void Sender::sendFrame(int frameNumber, Receiver& receiver) {
    if (frameNumber >= frames.size()) {
        cerr << "Hata: Geçersiz frame numarası!" << endl;
        return;
    }
    
    ProtocolUtils pr;
    string crcKodu;
    string frameData = frames[frameNumber].data;
    
    // CRC kodunu hesapla ve frame'e ekle
    if(frames[frameNumber].isPadded == true) {
        crcKodu = pr.crcHesapla(frameData.substr(0, frames[frameNumber].frameSize), CRC_POLY);
        cout << "Padded frame boyutu: " << frames[frameNumber].frameSize << " | CRC kodu: " << crcKodu << endl;
    } else {
        crcKodu = pr.crcHesapla(frameData, CRC_POLY);
    }

    frames[frameNumber].crc = crcKodu;
    frames[frameNumber].frameNumber = frameNumber;
    
    cout << "\n--- Frame " << frameNumber << " icin gonderim islemi basladi ---" << endl;
    cout << "CRC kodu hesaplandi: " << crcKodu << endl;
    
    bool ackReceived = false;
    int gonderimCount = 1;
    
    while (!ackReceived) { //ack gelene kadar devam eder gönderim
        cout << "\nFrame " << frameNumber << " gonderim denemesi: " << gonderimCount << endl;
        
        cout << "Frame " << frameNumber << " gonderiliyor..." << endl;
        this_thread::sleep_for(chrono::milliseconds(200));
        
        // %10 olasılıkla frame iletim sırasında kaybolabilir
        int random = rand() % 100;
        bool frameLost = (random < 10);
        
        bool receiverGotFrame = false;
        
        if (frameLost) {
            cout << "HATA: Frame " << frameNumber << " iletimde KAYBOLDU" << endl;
        } else {
            cout << "Frame " << frameNumber << " basariyla karsi tarafa iletildi." << endl;
            
            // frame başarıyla iletildi, receiver'dan cevap al
            receiverGotFrame = receiver.receiveFrame(frames[frameNumber]);
        }
        
        // ACK/NACK alınması simülasyonu
        if (!frameLost) {
            cout << "ACK/NACK bekleniyor... (Timeout: " << TIMEOUT << "ms)" << endl;
            
            // bekleme animasyonu
            for (int i = 0; i < 3; i++) {
                cout << ".";
                cout.flush();
                this_thread::sleep_for(chrono::milliseconds(200));
            }
            cout << endl;
            
            // %15 olasılıkla ACK/NACK yolda kaybolur (simülasyon)
            random = rand() % 100;
            bool ackLost = (random < 15);
            
            if (ackLost) {
                cout << "TIMEOUT: ACK/NACK yolda KAYBOLDU!" << endl;
                ackReceived = false;
            } else {
                // ACK/NACK başarıyla alındı
                if (receiverGotFrame) {
                    cout << "ACK basariyla alindi!" << endl;
                    ackReceived = true;
                } else {
                    cout << "NACK alindi! Frame CRC hatasi iceriyor." << endl;
                    ackReceived = false;
                }
            }
        } else {
            // Frame kaybolduysa zaten ACK gelmeyecek
            ackReceived = false;
        }
        
        if (!ackReceived) {
            cout << "Frame " << frameNumber << " yeniden gonderilecek..." << endl;
            this_thread::sleep_for(chrono::milliseconds(500)); // Tekrar gönderim öncesi bekleme
            gonderimCount++;
        }
    }
    
    cout << "Frame " << frameNumber << " basariyla gonderildi ve onaylandi." << endl;
    cout << "Toplam gonderim denemesi: " << gonderimCount << endl;
}*/

void Sender::sendFrame(int frameNumber, Receiver& receiver) {
    if (frameNumber >= frames.size()) {
        cerr << "Hata: Geçersiz frame numarası!" << endl;
        return;
    }

    ProtocolUtils pr;
    string originalData = frames[frameNumber].data;
    bool isPadded = frames[frameNumber].isPadded;
    int actualSize = frames[frameNumber].frameSize;

    // Hata oluşturulsun mu? (örnek: %20 ihtimal)
    //bool bilincliHata = (rand() % 100) < 20;
    bool bilincliHata = true;
    bool ackReceived = false;
    int gonderimCount = 1;
    bool ilkDeneme = true;

    while (!ackReceived) {
        // Her denemede gönderilecek veriyi hazırla (ilk denemede bozuk olabilir)
        string gonderilecekVeri = originalData;
        cout << "Original veri: " << originalData << endl;
        if (ilkDeneme && bilincliHata) {
            cout << "UYARI: Frame " << frameNumber << " BILEREK HATALI GONDERILIYOR (bit bozma)" << endl;
            //int pos = rand() % gonderilecekVeri.size()-1;
            int pos=25;
            cout << "pos: " << pos << endl;
            gonderilecekVeri[pos] = (gonderilecekVeri[pos] == '0') ? '1' : '0';
            cout << "Gonderilecek bozulmus veri: " << gonderilecekVeri << endl;
        }

        // CRC hesapla
        string crcKodu;
        if (isPadded) {
            crcKodu = pr.crcHesapla(originalData.substr(0, actualSize), CRC_POLY);
        } else {
            crcKodu = pr.crcHesapla(originalData, CRC_POLY);
        }

        // Geçici frame oluştur
        Frame tempFrame = frames[frameNumber];
        tempFrame.data = gonderilecekVeri;
        tempFrame.crc = crcKodu;
        tempFrame.frameNumber = frameNumber;

        // Gönderim logu
        cout << "\n--- Frame " << frameNumber << " gonderim denemesi: " << gonderimCount << " ---" << endl;
        cout << "CRC kodu: " << crcKodu << endl;

        // %10 olasılıkla frame kaybolabilir
        int random = rand() % 100;
        bool frameLost = (random < 10);
        bool receiverGotFrame = false;

        if (frameLost) {
            cout << "HATA: Frame " << frameNumber << " iletimde KAYBOLDU" << endl;
        } else {
            cout << "Frame " << frameNumber << " karsiya iletildi." << endl;
            receiverGotFrame = receiver.receiveFrame(tempFrame);
        }

        // ACK/NACK simülasyonu
        if (!frameLost) {
            cout << "ACK/NACK bekleniyor... (Timeout: " << TIMEOUT << "ms)" << endl;
            for (int i = 0; i < 3; i++) {
                cout << ".";
                cout.flush();
                //std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            cout << endl;

            // %15 olasılıkla ACK/NACK kaybolabilir
            random = rand() % 100;
            bool ackLost = (random < 15);

            if (ackLost) {
                cout << "TIMEOUT: ACK/NACK yolda kayboldu!" << endl;
                ackReceived = false;
            } else {
                if (receiverGotFrame) {
                    cout << "ACK alindi." << endl;
                    ackReceived = true;
                } else {
                    cout << "NACK alindi. CRC hatasi!" << endl;
                    ackReceived = false;
                }
            }
        } else {
            ackReceived = false;
        }

        if (!ackReceived) {
            cout << "Frame " << frameNumber << " yeniden gonderilecek..." << endl;
            //std::this_thread::sleep_for(std::chrono::milliseconds(500));
            gonderimCount++;
            ilkDeneme = false;  // sonraki denemelerde hata yapma
        }
    }

    cout << "Frame " << frameNumber << " basariyla gonderildi ve onaylandi." << endl;
    cout << "Toplam gonderim denemesi: " << gonderimCount << endl;
}

// stop-and-Wait protokolü ile tüm frameleri gönderen metod -> tum simulasyonu buradan yonetebiliriz
void Sender::sendAllFrames(Receiver& receiver) {
    if (frames.empty()) {
        cerr << "Gonderilecek frame yok!" << endl;
        return;
    }
    
    cout << "\n=== STOP-AND-WAIT PROTOKOLU BASLATILIYOR ===\n" << endl;
    cout << "Toplam gonderilecek frame sayisi: " << frames.size() << endl;
    cout << "ACK timeout: " << TIMEOUT << "ms\n" << endl;
    
    for (size_t i = 0; i < frames.size(); i++) {
        cout << "\n>>>>> Frame " << i << "/" << (frames.size()-1) << " gonderiliyor <<<<<" << endl;
        
        sendFrame(i, receiver); //burada cagirdim
        
        // Bir sonraki frame'e geçmeden önce kısa bir bekleme
        if (i < frames.size() - 1) {
            cout << "\nBir sonraki frame'e geciliyor..." << endl;
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
    
    allFramesSent = true;
    cout << "\nTum frameler basariyla gonderildi!" << endl;
    
    // Alıcı tarafta dosya oluştur
    receiver.finalizeReception();
}

int main() {
    string filename;
    Sender sender;
    ProtocolUtils pr;
    Receiver r;

    cout << "Dosya adini girin: ";
    cin >> filename;

    if(sender.readFileAndCreateFrames(filename)){
        int i;
        for(i=0;i<sender.frames.size();i++){
            //sender.sendFrame(i, r);
        }

        sender.sendAllFrames(r);
    }

    string chksm;
    chksm = pr.calculateChecksum(sender.frames, sender.frames.size());

    cout <<"deneme crc: "<<sender.frames[0].crc<<endl;
    
    cout<<"deneme checksum sonucu: "<<chksm<<endl;

    return 0;
}
