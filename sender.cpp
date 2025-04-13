#include "sender.h"
#include "protocol_utils.h"

class Sender {
private:
    vector<Frame> frames;
    string checksumValue;
    string checksumHex;
    bool allFramesSent;
    
public:
    Sender() : allFramesSent(false) {}
    
    // dosyadan veri okuma ve çerçevelere bölme işlemi
    bool readDataFile(const string& filename) {
        
    }
    
    // frame gönderme, crc-checksum işlemleri ve simülasyon işlemleri (yüzdeliklerle belirteceğimiz şeyler)
    void sendFrames(Receiver& receiver) {
        
};

int main() {
    srand(time(nullptr));
    
    // gui eklenir
    Sender sender;
    Receiver receiver;
    
    string filename = "test.dat"; // GUI'de kullanıcı seçecek
    
    if(sender.readDataFile(filename)) {
        sender.sendFrames(receiver);
    }
    
    return 0;
}