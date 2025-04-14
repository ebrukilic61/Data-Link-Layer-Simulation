#include "sender.h"
#include "protocol_utils.h"
#include <iostream>
#include <fstream>
#include <bitset>
#define CRC_POLY "10001000000100001"

using namespace std;

#define FRAME_SIZE 100

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
            frames.push_back(f);
            currentFrame = currentFrame.substr(FRAME_SIZE);
        }
    }
    
    if (!currentFrame.empty()) { //son frame tamamlanmamışsa
        while (currentFrame.size() < FRAME_SIZE) {
            currentFrame.push_back('0'); // eksik kalan bitler 0 ile tamamlanır
        }
        Frame f;
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
string Sender::xorHesapla(string a, string b)
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

string Sender::crcHesapla(string data, string key)
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
string Sender::checksum(string crcCode, int n) { //doğruluğunu kontrol etmem lazım, önceden yazmıştım bu kodu
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
*/

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

int main() {
    string filename;
    Sender sender;

    cout << "Dosya adini girin: ";
    cin >> filename;

    if(sender.readFileAndCreateFrames(filename)){
        int i;
        for(i=0;i<sender.frames.size();i++){
            sender.sendFrame(sender.frames[i].data, i);
        }
    }
    
    return 0;
}