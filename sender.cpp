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
    ProtocolUtils pr;
    ifstream file(fileName.c_str(), ios::binary);
    if (!file) {
        cerr << "Dosya acilamadi!" << endl;
        return false;
    }

    string bitStream;
    char byte;

    while (file.read(&byte, 1)) {
        bitset<8> bits(byte);
        for (int i = 7; i >= 0; i--) {
            bitStream.push_back(bits[i] ? '1' : '0');
        }
    }

    int frameCounter = 0;
    string carryOverBits;

    while (bitStream.length() >= 100) {
        string rawData = carryOverBits + bitStream.substr(0, 100 - carryOverBits.size());
        bitStream = bitStream.substr(100 - carryOverBits.size());

        Frame f;
        f.data = rawData;
        f.frameSize = 100;
        f.isPadded = false;
        f.frameNumber = to_string(frameCounter % 2);
        //cout<<"frame num: "<<frameCounter<<endl;

        // Bit stuffing
        string stuffedData = pr.applyBitStuffing(rawData);
        f.result = stuffedData;

        // Eğer stuffedData 100'den büyükse taşan bitleri al
        if (stuffedData.length() > 100) {
            carryOverBits = stuffedData.substr(100);
            f.result = stuffedData.substr(0, 100);  // Sadece 100 bitlik kısmı bu frame’e koy
        } else {
            carryOverBits = "";
        }

        f.addressSend = "0000001"; // örnek sender adresi
        f.addressRec = "0000010";  // örnek receiver adresi

        f.crc = pr.crcHesapla(f.result, CRC_POLY);

        cout<<"raw data: "<<f.data<<endl;
        cout<<"bit stuffed data: "<<f.result<<endl;
        cout<<"frame size"<<f.frameSize<<endl;

        frames.push_back(f);
        f.fNum = frameCounter;
        frameCounter++;
    }

    // kalan bit varsa son frame oluşturulur
    if (!bitStream.empty()) { // || !carryOverBits.empty()
        string stuffedData;
        //string rawData = carryOverBits + bitStream;
        Frame f;
        f.isPadded = true;
        //while (bitStream.size() < 100) bitStream.push_back('0');
        f.data = bitStream;
        f.addressSend = "0000001";
        f.addressRec = "0000010";
        f.frameNumber = to_string(frameCounter % 2);
        f.result = pr.applyBitStuffing(f.data);
        if (f.result.length() > 100) {
            carryOverBits = f.result.substr(100);
            f.result = f.result.substr(0, 100);
        } else {
            carryOverBits = "";
        }
        stuffedData = carryOverBits + bitStream;
        f.frameSize = f.result.size();
        while (stuffedData.size() < FRAME_SIZE) {
            stuffedData.push_back('0'); // eksik kalan bitler 0 ile tamamlanır
        }
        cout <<"frame_data:"<<f.result.substr(0, f.frameSize)<<endl;
        f.crc = pr.crcHesapla(f.result.substr(0, f.frameSize), CRC_POLY);
        cout<<"raw data: "<<f.data<<endl;
        cout<<"bit stuffed data: "<<f.result<<endl;
        cout<<"frame size: "<<f.result.size() <<endl;
        cout<<"carry over bits: "<<carryOverBits<<endl;
        f.fNum = frameCounter;
        frames.push_back(f);
    }

    file.close();
    cout << "Toplam " << frames.size() << " frame olusturuldu." << endl;
    return true;
}

string Sender::transformFrame(const Frame & fr){
    Sender sender;
    string frame = fr.addressSend+fr.frameNumber+fr.result+FRAME_FLAG+fr.crc; //sender kendi adresini gönderecek
    cout<<"RESULT:"<<fr.result<<endl;
    cout<<"FRAME_FLAG:"<<FRAME_FLAG<<endl;
    cout<<"CRC:"<<fr.crc<<endl;
    return frame;
}

void Sender::sendFrame(int fNum, Receiver& receiver) {
    if (fNum >= frames.size()) {
        cerr << "Hata: Gecersiz frame numarasi!" << endl;
        return;
    }
    
    ProtocolUtils pr;
    string originalData = frames[fNum].result;
    bool isPadded = frames[fNum].isPadded;
    int actualSize = frames[fNum].frameSize;

    // %20 ihtimal
    bool bilincliHata = (rand() % 100) < 20;
    bool ackReceived = false;
    int gonderimCount = 1;
    bool ilkDeneme = true;
    string crcKodu;

    if (isPadded) {
        crcKodu = pr.crcHesapla(originalData.substr(0, actualSize), CRC_POLY);
    } else {
        crcKodu = pr.crcHesapla(originalData, CRC_POLY);
    }
    
    cout << "\n--- Frame " << fNum << " icin gonderim islemi basladi ---" << endl;
    cout << "CRC kodu hesaplandi: " << crcKodu << endl;
    cout << "fnum: "<<fNum<<endl;

    while (!ackReceived) { //ack gelene kadar devam eder gönderim
        cout << "\nFrame " << fNum << " gonderim denemesi: " << gonderimCount << endl;
        
        cout << "Frame " << fNum << " gonderiliyor..." << endl;
        this_thread::sleep_for(chrono::milliseconds(200));

        //%20'lik hata olusturma görevi:
        string gonderilecekVeri = originalData;
        cout << "Original veri: " << originalData << endl;
        if (ilkDeneme && bilincliHata) {
            cout << "UYARI: Frame " << fNum << " BILEREK HATALI GONDERILIYOR (bit bozma)" << endl;
            //int pos = rand() % gonderilecekVeri.size()-1;
            int pos=25;
            cout << "pos: " << pos << endl;
            gonderilecekVeri[pos] = (gonderilecekVeri[pos] == '0') ? '1' : '0';
            cout << "Gonderilecek bozulmus veri: " << gonderilecekVeri << endl;
        }

        Frame tempFrame = frames[fNum];
        tempFrame.data = gonderilecekVeri;
        tempFrame.crc = crcKodu;
        tempFrame.fNum = fNum;

        // %10 olasılıkla frame iletim sırasında kaybolabilir
        int random = rand() % 100;
        bool frameLost = (random < 10);
        
        bool receiverGotFrame = false;
        
        if (frameLost) {
            cout << "HATA: Frame " << fNum << " iletimde KAYBOLDU" << endl;
        } else {
            string transformedData = transformFrame(frames[fNum]);
            cout << "Frame " << fNum << " basariyla karsi tarafa iletildi." << endl;            
            receiverGotFrame = receiver.receiveFrame(transformedData); //sadece data ->transformFrame
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
                cout << "ACK/NACK yolda KAYBOLDU!" << endl;
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
            cout << "Frame " << fNum << " yeniden gonderilecek..." << endl;
            this_thread::sleep_for(chrono::milliseconds(500)); // Tekrar gönderim öncesi bekleme
            gonderimCount++;
        }
    }
    
    cout << "Frame " << fNum << " basariyla gonderildi ve onaylandi." << endl;
    cout << "Toplam gonderim denemesi: " << gonderimCount << endl;
}

// stop-and-wait protokolü ile tüm frameleri gönderen metod -> tum simulasyonu buradan yonetebiliriz
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
        
        sendFrame(i, receiver); //frame gönderim işlemini (simülasyonlar vs) burada cagirdim
        
        // Bir sonraki frame'e geçmeden önce kısa bir bekleme
        if (i < frames.size() - 1) {
            cout << "\nBir sonraki frame'e geciliyor..." << endl;
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
    
    allFramesSent = true;
    cout << "\nTum frameler basariyla gonderildi!" << endl;

    ProtocolUtils pr;
    string checksum = pr.calculateChecksum(frames, frames.size());
    string chk = pr.applyBitStuffing(checksum);
    Frame checksumFrame;
    checksumFrame.checksum = chk;
    //checksumFrame.result = checksum;  // checksum için bit stuffing yapmadım
    checksumFrame.frameSize = checksum.length();
    checksumFrame.addressSend = "0000001";
    //checksumFrame.addressRec = "0000010";
    checksumFrame.frameNumber = "0";
    checksumFrame.fNum = frames.size();

    //checksuma bit stuffing uygulanmalı
    string stuffedChk = pr.applyBitStuffing(checksumFrame.checksum);
    cout<<"bit stuffed checksum: "<<stuffedChk<<endl;

    string recChecksumFrame = FRAME_FLAG + checksumFrame.addressSend + checksumFrame.frameNumber + stuffedChk + FRAME_FLAG;
    bool recChkTrue = receiver.receiveChecksumFrame(recChecksumFrame);

    if(recChkTrue){
        cout << "Checksum dogru, veri gonderimi basariyla sona erdi." << endl;
    }else{
        cout<<"Cheksum yanlis, veri gonderimi bastan yapilacak"<<endl;
    }
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

    return 0;
}