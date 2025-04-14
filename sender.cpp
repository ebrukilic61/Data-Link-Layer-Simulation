#include "sender.h"
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

Sender::Sender() : allFramesSent(false) {}

bool Sender::readFileAndCreateFrames(const string &fileName) { // dosyadan veri okuma ve çerçevelere bölme işlemi
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
    
    if (!currentFrame.empty()) { //son frame tamamlanmamışsa
        Frame f;
        f.frameSize = currentFrame.size();
        f.isPadded = true;
        while (currentFrame.size() < FRAME_SIZE) {
            currentFrame.push_back('0'); // eksik kalan bitler 0 ile tamamlanır
        }
        
        f.data = currentFrame;
        frames.push_back(f);
    }

    for (i = 0; i < frames.size(); i++) {
        cout << "Frame " << i << ": " << frames[i].data << endl; //frame i+1 diyordu ama biz bunu i olarak kaydediyoruz bu nedenle değiştirdim
    }

    file.close();
    return true;
}

/*
void Sender::sendFrame(const string& frameData, int frameNumber){  //crc kod dönüşümü
    cout << "Frame gonderimi basliyor..." << endl;
    ProtocolUtils pr;
    string crcKodu = pr.crcHesapla(frameData, CRC_POLY);
    
    Frame gonderilecekFrame;
    gonderilecekFrame.data = frameData;
    gonderilecekFrame.crc = crcKodu;
    gonderilecekFrame.frameNumber = frameNumber;

    cout<<"frame "<<frameNumber<<" icin crc kodu olusturuldu: "<<crcKodu<<endl;
}
*/

void Sender::sendFrame(int frameNumber) {
    if (frameNumber >= frames.size()) {
        cerr << "Hata: Geçersiz frame numarası!" << endl;
    }
    
    ProtocolUtils pr;
    string crcKodu;
    string frameData = frames[frameNumber].data;
    
    if(frames[frameNumber].isPadded == true){
        crcKodu = pr.crcHesapla(frameData.substr(0,frames[frameNumber].frameSize),CRC_POLY); //datayı tuttuğumuz orijinal frame sizeına göre alıp crc kodunu hesaplıyoruz
        //cntrl için cout:
        cout << "padded frame boyutu: "<<frames[frameNumber].frameSize << " | crc kodu: "<<crcKodu<<endl;
    }else{
        crcKodu = pr.crcHesapla(frameData, CRC_POLY);
    }

    frames[frameNumber].crc = crcKodu;
    frames[frameNumber].frameNumber = frameNumber;
    
    cout << "Frame " << frameNumber << " gonderiliyor..." << endl;
    cout << "Frame " << frameNumber << " icin CRC kodu: " << crcKodu << endl;
    
    /*
    int retryCount = 0;
    bool ackReceived = false;
    
    while (!ackReceived && retryCount < MAX_RETRANSMISSION) {
        // frame gönderme simülasyonu
        if (!simulateSendFrame(frames[frameNumber])) {
            cout << "Frame " << frameNumber << " gönderiminde hata oluştu." << endl;
            retryCount++;
            continue;
        }
        
        // ACK almayı bekle
        ackReceived = waitForAck(frameNumber);
        
        if (!ackReceived) {
            cout << "Timeout: Frame " << frameNumber << " için ACK alınamadı." << endl;
            cout << "Frame " << frameNumber << " yeniden gönderiliyor... (Deneme: " << retryCount + 1 << ")" << endl;
            retryCount++;
        }
    }
    
    if (ackReceived) {
        cout << "Frame " << frameNumber << " başarıyla gönderildi ve ACK alındı." << endl;
        return true;
    } else {
        cout << "Frame " << frameNumber << " için maksimum yeniden gönderim sayısına ulaşıldı!" << endl;
        return false;
    }
    */
}

bool Sender::waitForACK(int frameNumber) {
    // burada ACK alma simülasyonu yapıyoruz
    cout << "ACK bekleniyor..." << endl;
    
    // timeout
    this_thread::sleep_for(chrono::milliseconds(rand() % TIMEOUT));
    
    // %85 ACK alma başarı oranı, %15 karşı tarafa ulaşmaz
    int random = rand() % 100;
    return (random < 85);
}

/*
bool Sender::simulateSendFrame(const Frame& frame) {

}
*/

int main() {
    string filename;
    Sender sender;

    cout << "Dosya adini girin: ";
    cin >> filename;

    if(sender.readFileAndCreateFrames(filename)){
        int i;
        for(i=0;i<sender.frames.size();i++){
            sender.sendFrame(i);
        }
    }
    
    return 0;
}