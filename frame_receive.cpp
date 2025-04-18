#include <iostream>
#define CRC_POLY "10001000000100001"
#define FRAME_FLAG "01111110"

using namespace std;

bool receiveFrame(string fullFrame) {
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
}

int main() {
    string fullFrame = "101010100110011001110010011111010011111101100000110110101"; // örnek frame, CRC hesaplandı 25 bitlik stuffed veri için

    bool result = receiveFrame(fullFrame);

    if(result) {
        cout << "Frame başarıyla alındı ve CRC doğrulandı." << endl;
    } else {
        cout << "Frame alınamadı veya CRC hatalı!" << endl;
    }

    return 0;
}