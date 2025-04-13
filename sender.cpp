#include "sender.h"
#include "protocol_utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>

#define FRAME_SIZE 100

using namespace std;

class Sender {
private:
    string checksumValue;
    string checksumHex;
    bool allFramesSent;
    
public:
    Sender() : allFramesSent(false) {}
    
    // dosyadan veri okuma ve çerçevelere bölme işlemi
    bool readFileAndCreateFrames(const string &fileName) {
        int i;
        ifstream file;
        vector<string> frames;  
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
                if(bits[i]){
                    currentFrame.push_back('1');
                }
                else{
                    currentFrame.push_back('0');
                }
            }
    
            if (currentFrame.size() >= FRAME_SIZE) {
                frames.push_back(currentFrame.substr(0, FRAME_SIZE)); 
                currentFrame = currentFrame.substr(FRAME_SIZE); 
            }
        }
    
        if (!currentFrame.empty()) {  //son frame tamamlanmamışsa
            while (currentFrame.size() < FRAME_SIZE) {
                currentFrame.push_back('0');  // eksik kalan bitler 0 ile tamamlanır
            }
            frames.push_back(currentFrame);
            
        }
    
        for (i = 0; i < frames.size(); i++) {
            cout << "Frame " << i + 1 << ": " << frames[i] << endl;
        }
    
        file.close();
        return true;  
    }
    
    // frame gönderme, crc-checksum işlemleri ve simülasyon işlemleri (yüzdeliklerle belirteceğimiz şeyler)
    void sendFrames(Receiver& receiver) {
    }
};

int main() {
    srand(time(nullptr));
    
    // gui eklenir
    Sender sender;
    Receiver receiver;

    string filename;
    cout << "Dosya adini girin: ";
    cin >> filename;
    
    if(sender.readFileAndCreateFrames(filename)) {
        sender.sendFrames(receiver);
    }
    
    return 0;
}