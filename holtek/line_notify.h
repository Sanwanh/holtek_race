#ifndef __LINE_NOTIFY_H__
#define __LINE_NOTIFY_H__

#include <SoftwareSerial.h>

//++hardware
#define ESP8266 Serial3    //hardware // arduino mega 2560 tx1 rx1(18,19)

//++software
//SoftwareSerial ESP8266(19,18);


class Line_notify
{
private:
    void sendAtCommand(String,String,unsigned long);
    String token = "";
public:
    void init(String ssid, String password);
    void set_token(String token);
    void send_msg(String);

};


void Line_notify::sendAtCommand(String cmd , String waitWord , unsigned long timeout = 5000){
    unsigned long startTime;
    while(ESP8266.available()){
        ESP8266.read();
    }
    int len_waitWord = waitWord.length();
    int check_cnt = 0;
    String waitWord1 = "ERROR";
    int len_waitWord1 = waitWord1.length();
    int check_cnt1 = 0;

    ESP8266.print(cmd + "\r\n");

    startTime = millis();
    while( (millis() - startTime) < timeout){
        while(ESP8266.available()){
            //Serial.println("[][][][]FUCK");
            char c = ESP8266.read();
            if(c == waitWord.charAt(check_cnt)){
                check_cnt++;
            }
            else{
                check_cnt = 0;
            }

            if(c == waitWord1.charAt(check_cnt1)){
                check_cnt1++;
            }
            else{
                check_cnt1 = 0;
            }

            if(check_cnt == len_waitWord || check_cnt1 == len_waitWord1){
                return;
            }
        }
    }
    
    
}

void Line_notify::init(String ssid, String password){
    ESP8266.begin(115200);
    sendAtCommand("AT" , "OK");
    sendAtCommand("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"" , "OK" );
}
void Line_notify::set_token(String token){
    this->token = token;
}


void Line_notify::send_msg(String msg){
    int len_msg = msg.length();
    String post = (String)  "POST /api/notify HTTP/1.1\r\n" 
                        +   "Host: notify-api.line.me\r\n"
                        +   "Authorization: Bearer " + token + "\r\n"
                        +   "Content-Type: application/x-www-form-urlencoded\r\n"
                        +   "Content-Length: "
                        +   String(len_msg + 8) + "\r\n\r\n" 
                        +   "message=" + msg;


    sendAtCommand("AT+CIPSTART=\"SSL\",\"notify-api.line.me\",443" , "OK");

    sendAtCommand("AT+CIPSEND=" + String(post.length()) , ">");

    sendAtCommand(post , "OK");

    //sendAtCommand("AT+CIPCLOSE" , "OK");

}


#endif