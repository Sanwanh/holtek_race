#include <SoftwareSerial.h>
#include "line_notify.h"
#include <BMS56M605.h>  // 陀螺儀
#include <BMH12M105.h>  // 秤重
#include <BMH08002-4.h> //  血氧


Line_notify lineNotify;
BMH12M105 weight(21, 20); // 使用軟體 UART 通訊，RX 腳位連接 D6，TX 腳位連接 D

int readData, readData_1; // 創建變數，用於存儲重量值
int Threshold = 1000;

BMS56M605 Mpu(8); // INT Pin 連接 D8

BMH08002_4 mySpo2(2, &Serial4); // 軟體 UART,EN=D2,RX=D5,TX=D4
uint8_t Mode = 0;               // 工作模式
uint8_t rBuf[15] = {0};         // 存放測量數據
uint8_t Status = 0;             // 測量狀態
uint8_t flag = 0;               // 是否有手指

int LEDBOT = 3;
int Botton2 = 7;

void setup()
{
  pinMode(LEDBOT , OUTPUT);
  digitalWrite(LEDBOT,1);
  pinMode(Botton2 , INPUT_PULLUP);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial4.begin(38400);
  lineNotify.init("SAN", "0966856720");
  lineNotify.set_token("lKxdwgslXOi7fIN2tVhNeYG5HIBGvqIQk9G136pA4az");
  pinMode(9,OUTPUT);
  Mpu.begin();    // 三軸模組初始化
  mySpo2.begin(); // 血氧初始化模組
  weight.begin(); // 重量模組初始化
  Serial.println("start");
  mySpo2.setModeConfig(0x01); // 定时传输模式，检测到手指时红光亮起
  mySpo2.setTimeInterval(300);
  // Serial.println("Please place your finger"); // 提示放置手指
  mySpo2.beginMeasure();         // 進入開始測量狀態
  Mode = mySpo2.getModeConfig(); // 查詢工作模式
  // lineNotify.send_msg("TEST");
  if (Mode == 0x02 || Mode == 0x03)
  {
    Mode = 1;
  }
  else
    Mode = 0;
  Serial.println(Mode);
}
bool start = false;
unsigned long last_use_time = 0;
void loop()
{

  // sendLineNotify(msg);         // 發送訊息
  Mpu.getEvent();                  // 獲取三軸加速度、角速度以及環境溫度
  readData = weight.readWeight();  // 讀取重量值
  readData_1 = readData - 37;      // 重量值校正
  Serial.println(readData_1);
  if (abs(readData_1) > Threshold) // Threshold閾值 500
  {
    if (start == false)
    {
      start = true;
      lineNotify.send_msg("拐杖啟動");
    }
    last_use_time = millis();
  }
  if (start == true && millis() - last_use_time > 5000)
  {
    start = false;
    lineNotify.send_msg("拐杖關閉");
  }

  if (start == true && (abs(Mpu.gyroX) > 60 || abs(Mpu.gyroY) > 60 || abs(Mpu.gyroZ) > 60))
  {
    lineNotify.send_msg("可能摔倒了");
    delay(100);
    for(int a =0 ;a<3 ; a++){
      digitalWrite(9,HIGH);
      digitalWrite(3,0);
      delay(500);
      digitalWrite(9,LOW);
      digitalWrite(3,1);
      delay(500);
    }
  }
  if (start == true)
  {
    Mode_ask();
  }
  // if (Botton2 == 0)
  // {
  //   for(int a =0 ;a<3 ; a++){
  //     digitalWrite(9,HIGH);
  //     digitalWrite(2,0);
  //     delay(500);
  //     digitalWrite(9,LOW);
  //     digitalWrite(2,1);
  //     delay(500);
  //   }
  // }
}

void Mode_ask()
{

  Status = mySpo2.requestInfoPackage(rBuf);
  if (Status == 0x02)
  {
    // Serial.println("测量完成，请拿开手指");
    displayMeasurementResult(); // 顯示測量結果
    mySpo2.endMeasure();        // 停止测量
    mySpo2.sleep();             // 进入休眠
    Mode = 0;
    uint8_t rBuf[15] = {0};
    uint8_t Status = 0;
    uint8_t flag = 0;
    Serial.println(Mode);
    mySpo2.setModeConfig(0x01);
    // mySpo2.beginMeasure(); // 重新开始测量
  }
  if (Status == 0x01 && flag != 1)
  {
    lineNotify.send_msg("請勿移動手指");
    // Serial.println("請勿移動手指");
    flag = 1;
  }
  if (Status == 0x00 && flag != 0)
  {
    lineNotify.send_msg("請重新放置手指");
    // Serial.println("請重新放置手指");
    flag = 0;
  }
}

void displayMeasurementResult()
{
  String output = (String) "\n" + "SpO2: " + String(rBuf[0], DEC) + " percent" + "\n" +
                  "心率: " + String(rBuf[1], DEC) + " bpm/min" + "\n" +
                  "PI數據: " + String((float)rBuf[2] / 10) + " percent";
  // 然后您可以将这个字符串发送出去，例如通过串口发送
  lineNotify.send_msg(output);
  Serial.println(output);
}
