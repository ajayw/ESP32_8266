//项目基于ESP32主控，用来测试加减量变化，蓝牙无线连接到手机蓝牙调试APP，在APP上输入1来增加值，输入0来减小值
#include "BluetoothSerial.h"
// 创建一个 BluetoothSerial 实例
BluetoothSerial SerialBT;
#define LED_PIN 3             // LED连接到数字输出1
int number=0;
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  // 初始化蓝牙串口并设置设备名称
  if (!SerialBT.begin("ESP32_Bluetooth")) {
    Serial.println("蓝牙初始化失败！");
    while (1);
  }
  Serial.println("蓝牙已开启。请连接到 ESP32_Bluetooth");
}

void loop() {
    // 从蓝牙接收数据并发送到串口监视器

  if (SerialBT.available()) {
    int receivedByte = SerialBT.read();

    if((receivedByte=='1')&&(number<255)){
      number = number + 1;
    }else if((receivedByte=='0')&&(number>0)){
      number = number - 1;
    }
    analogWrite(LED_PIN, number);
    Serial.println(number);
    SerialBT.println(number);
  }
}
