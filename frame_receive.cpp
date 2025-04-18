/*
#include <iostream>
#define CRC_POLY "10001000000100001"
#define FRAME_FLAG "01111110"

using namespace std;

bool receiveFrame(string fullFrame) {
    size_t startFlagPos = fullFrame.find(FRAME_FLAG);
    size_t endFlagPos = fullFrame.rfind(FRAME_FLAG);

    if (startFlagPos == string::npos || endFlagPos == string::npos || startFlagPos == endFlagPos) {
        cout << "Geçerli iki flag bulunamadı!" << endl;
        return false;
    }

    // Flag'ler arası veri (asıl frame içeriği)
    string frameContent = fullFrame.substr(startFlagPos + 8, endFlagPos - (startFlagPos + 8));

    // Header: 7 bit adres + 1 bit frame numarası
    string senderAddr = frameContent.substr(0, 7);
    string frameNum = frameContent.substr(7, 1);
    string dataWithChecksum = frameContent.substr(8);

    cout << "sender address: " << senderAddr << endl;
    cout << "frame number: " << frameNum << endl;
    cout << "checksum: " << dataWithChecksum << endl;

    return true;
}

int main() {
    string fullFrame = "01111110101010100111011101111110";
    bool result = receiveFrame(fullFrame);

    if (result) {
        cout << "Frame basariyla alindi ve CRC ayrildi." << endl;
    } else {
        cout << "Frame alinamadi veya CRC hatali!" << endl;
    }

    return 0;
}
*/

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
    bool bilincliHata = (rand() % 100) < 20;
    //bool bilincliHata = true;
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