#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
// 定义服务和特性 UUID
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef1"
#define CHARACTERISTIC_UUID "abcdef01-2345-6789-1234-56789abcdef0"

//舵机
#define MOTOR 2
#define DOWN_SERVO 5

// 循迹控制引脚
#define LEFT_LINE_TRACKING      34
#define CENTER_LINE_TRACKING    35
#define RIGHT_LINE_TRACKING     14

//超声波
#define Trigpin 13
#define Echopin 12

//右
#define RightFrontFWD 27
#define RightFrontBWD 23
#define RightBackFWD  33
#define RightBackBWD  32

//左
#define LeftFrontFWD 18
#define LeftFrontBWD 19
#define LeftBackFWD  26
#define LeftBackBWD  25

Servo myUpServo;  // 创建一个Servo对象
Servo myDownServo;  // 创建一个Servo1对象

int model_var = 1;
int Left_Tra_Value;
int Center_Tra_Value;
int Right_Tra_Value;
int Black_Line = 1500;
unsigned long mid_distance;
int upServoMidAngle = 90;
int downServoAngle = 90;
int downServoMidAngle = 90;
int DownServoMark = 0;
// 全局变量
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false; // 记录 BLE 连接状态

// 小车控制函数声明
void moveForward();
void moveBackward();
void rotateLeft();
void rotateRight();
void stopMoving();
void TrackLeft();
void TrackRight();
unsigned long getDistance();

// 回调类：处理 BLE 连接和断开事件
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected!");
    stopMoving();
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected!");
    pServer->startAdvertising(); // 重新开始广播
  }
};

// 回调类：处理 BLE 数据写入事件
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.length() > 0) {
      char cmd = value[0];
      Serial.print("Received value: ");
      Serial.println(cmd);

      // 执行对应的指令
      switch (cmd) {
        case 'W':
          if(model_var == 1) moveForward();
          pCharacteristic->setValue("W");
          break;
        case 'S':
          if(model_var == 1) moveBackward();
          pCharacteristic->setValue("S");
          break;
        case 'A':
          // if(model_var == 1) rotateLeft();
          DownServoMark = 8;
          pCharacteristic->setValue("A");
          break;
        case 'D':
          // if(model_var == 1) rotateRight();
          DownServoMark = 9;
          pCharacteristic->setValue("D");
          break;
        case 'B':
          if(model_var == 1) {
            stopMoving();
            DownServoMark = 0;
          }
          pCharacteristic->setValue("B");
          break;
        case '1':
          downServoAngle = downServoMidAngle;
          myUpServo.write(upServoMidAngle);
          myDownServo.write(downServoMidAngle);
          model_var = 1;
          stopMoving();
          pCharacteristic->setValue("model_var = 1");
          break;
        case '2':
          myUpServo.write(upServoMidAngle);
          myDownServo.write(downServoMidAngle);
          model_var = 2;
          pCharacteristic->setValue("model_var = 2");
          break;
        case '3':
          myUpServo.write(upServoMidAngle);
          myDownServo.write(downServoMidAngle);
          model_var = 3;
          pCharacteristic->setValue("model_var = 3");
          break;
        case '4':
          myUpServo.write(upServoMidAngle);
          myDownServo.write(downServoMidAngle);
          model_var = 4;
          pCharacteristic->setValue("model_var = 4");
          break;
        case 'P':
          downServoMidAngle = downServoAngle;
          pCharacteristic->setValue("MidAngle!");
          break;
        default:
          model_var = 1;
          stopMoving();
          // pCharacteristic->setValue("Invalid command, stopped.");
          break;
      }
      // delay(100);
    }
  }
};

void moveForward() {
  digitalWrite(RightFrontFWD, HIGH);
  digitalWrite(RightFrontBWD, LOW);       
  digitalWrite(RightBackFWD, HIGH); 
  digitalWrite(RightBackBWD, LOW);  
  
  digitalWrite(LeftFrontFWD, HIGH); 
  digitalWrite(LeftFrontBWD, LOW);       
  digitalWrite(LeftBackFWD, HIGH); 
  digitalWrite(LeftBackBWD, LOW);   
}

void moveBackward() {
  digitalWrite(RightFrontFWD, LOW);  
  digitalWrite(RightFrontBWD, HIGH);
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, HIGH); 
  
  digitalWrite(LeftFrontFWD, LOW);  
  digitalWrite(LeftFrontBWD, HIGH);
  digitalWrite(LeftBackFWD, LOW);  
  digitalWrite(LeftBackBWD, HIGH);
}

void TrackRight() {//
  digitalWrite(RightFrontFWD, LOW);
  digitalWrite(RightFrontBWD, LOW);       
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, LOW);  
  
  digitalWrite(LeftFrontFWD, HIGH); 
  digitalWrite(LeftFrontBWD, LOW);       
  digitalWrite(LeftBackFWD, HIGH); 
  digitalWrite(LeftBackBWD, LOW);   
}

void TrackLeft() {
  digitalWrite(RightFrontFWD, HIGH);
  digitalWrite(RightFrontBWD, LOW);       
  digitalWrite(RightBackFWD, HIGH); 
  digitalWrite(RightBackBWD, LOW);  
  
  digitalWrite(LeftFrontFWD, LOW); 
  digitalWrite(LeftFrontBWD, LOW);       
  digitalWrite(LeftBackFWD, LOW); 
  digitalWrite(LeftBackBWD, LOW);   
}

void rotateRight() {
  if(downServoAngle > (downServoMidAngle - 40)){
    downServoAngle = downServoAngle - 5;
    delay(50);
  }
  myDownServo.write(downServoAngle);
  Serial.print("downServoAngle: ");
  Serial.println(downServoAngle);
}

void rotateLeft() {
  if(downServoAngle < (downServoMidAngle + 40)){
    downServoAngle = downServoAngle + 5;
    delay(50);
  }
  myDownServo.write(downServoAngle);
  Serial.print("downServoAngle: ");
  Serial.println(downServoAngle);
}

void stopMoving() { // 停止所有轮子的转动
  digitalWrite(RightFrontFWD, LOW);
  digitalWrite(RightFrontBWD, LOW);
  digitalWrite(RightBackFWD, LOW);
  digitalWrite(RightBackBWD, LOW);
  digitalWrite(LeftFrontFWD, LOW);
  digitalWrite(LeftFrontBWD, LOW);
  digitalWrite(LeftBackFWD, LOW);
  digitalWrite(LeftBackBWD, LOW);
}

// 获取超声波传感器距离
unsigned long getDistance() {
    digitalWrite(Trigpin, LOW);
    delayMicroseconds(2);
    digitalWrite(Trigpin, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigpin, LOW);

    unsigned long duration = pulseIn(Echopin, HIGH, 60000)/58.0; // 设置超时时间
    delay(50);
    if (duration > 0) {
        return duration;
    } else {
        return 0; // 返回 0 表示测量失败
    }
}

void ObstacleAvoidance(){
  mid_distance = getDistance();
  Serial.print("Present Distance is: "); 
  Serial.print(mid_distance, DEC); //output result to Serial monitor
  Serial.println("cm"); //output result to Serial monitor
  if(mid_distance < 15 && mid_distance > 0)
  {
    stopMoving();
    Serial.print("mid_distance: ");
    Serial.println(mid_distance);
    delay(500);

    myUpServo.write(140); //左
    delay(1000);
    unsigned long leftDistance = getDistance(); // 测量并存储距离
    Serial.print("leftDistance: ");
    Serial.println(leftDistance);
    delay(10);

    myUpServo.write(40);//右
    delay(1000);
    unsigned long rightDistance = getDistance(); // 测量并存储距离
    Serial.print("rightDistance: ");
    Serial.println(rightDistance);
    delay(10);

    myUpServo.write(upServoMidAngle);

    moveBackward();//后退
    delay(800);

    if(leftDistance > rightDistance)
    {
      myDownServo.write(downServoMidAngle+40);//左移
      moveForward();
      delay(1400);
      myDownServo.write(downServoMidAngle);
    }
    if(rightDistance > leftDistance)
    {
      myDownServo.write(downServoMidAngle-40);//左移
      moveForward();
      delay(1400);
      myDownServo.write(downServoMidAngle);
    }
    if(mid_distance > leftDistance && mid_distance > rightDistance)
    {
      myDownServo.write(40);
      moveBackward();//后退
      delay(1400);
      myDownServo.write(downServoMidAngle);
    }
  }else if(mid_distance > 15){
    moveForward();
  }else{
    Serial.println("dis=0");
  }
}

void Follow(){
  mid_distance = getDistance();
  Serial.print("Present Distance is: ");
  Serial.print(mid_distance, DEC); //output result to Serial monitor
  Serial.println("cm"); //output result to Serial monitor
  if(mid_distance < 15 && mid_distance>0)
  {
    moveBackward();
  }

  if(15 <= mid_distance && mid_distance  <= 20)
  {
    stopMoving();
  }

  if(mid_distance  >= 20)
  {
    moveForward();
  }
}

void Tracking(){
  Left_Tra_Value = analogRead(LEFT_LINE_TRACKING);
  Center_Tra_Value = analogRead(CENTER_LINE_TRACKING);
  Right_Tra_Value = analogRead(RIGHT_LINE_TRACKING);
  Serial.print("Left: ");
  Serial.print(Left_Tra_Value);
  Serial.print("\tCenter: ");
  Serial.print(Center_Tra_Value);
  Serial.print("\tRight: ");
  Serial.println(Right_Tra_Value);

  if (Left_Tra_Value < Black_Line && Center_Tra_Value >= Black_Line && Right_Tra_Value < Black_Line)
  {
    myUpServo.write(downServoMidAngle);
    moveForward();
  }

  if (Left_Tra_Value >= Black_Line && Left_Tra_Value > Right_Tra_Value)
  {
    myDownServo.write(downServoMidAngle + 20);
    TrackLeft();//左循迹
  }

  if (Right_Tra_Value >= Black_Line && Right_Tra_Value > Left_Tra_Value)
  {
    myDownServo.write(downServoMidAngle - 20);
    TrackRight();//右循迹
  }

  if (Left_Tra_Value >= Black_Line && Center_Tra_Value >= Black_Line && Right_Tra_Value >= Black_Line)
  {
    stopMoving();
  }

  if (Left_Tra_Value < Black_Line && Center_Tra_Value < Black_Line && Right_Tra_Value < Black_Line)
  {
    stopMoving();
  }
}

void setup() 
{
  Serial.begin(115200);

  // 初始化 BLE
  BLEDevice::init("ESP32_Car");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建服务和特性
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->setValue("Ready for commands");

  // 启动服务
  pService->start();

  // 开始广播
  pServer->getAdvertising()->start();
  Serial.println("BLE advertising started...");

  myUpServo.attach(MOTOR);  // 将舵机的信号线连接到ESP32的GPIO 
  myDownServo.attach(DOWN_SERVO);  // 将舵机的信号线连接到ESP32的GPIO 
  myUpServo.write(upServoMidAngle);
  myDownServo.write(downServoMidAngle);
  
  pinMode(Trigpin,OUTPUT);
  pinMode(Echopin,INPUT);
  
  pinMode(LEFT_LINE_TRACKING, INPUT);
  pinMode(CENTER_LINE_TRACKING, INPUT);
  pinMode(RIGHT_LINE_TRACKING, INPUT);

  pinMode(RightFrontFWD, OUTPUT);
  pinMode(RightFrontBWD, OUTPUT);
  pinMode(RightBackFWD, OUTPUT);
  pinMode(RightBackBWD, OUTPUT);
  pinMode(LeftFrontFWD, OUTPUT);
  pinMode(LeftFrontBWD, OUTPUT);
  pinMode(LeftBackFWD, OUTPUT);
  pinMode(LeftBackBWD, OUTPUT);
}

void loop()
{
  // 如果设备未连接，进入低功耗模式
  if (!deviceConnected) {
    Serial.println("Device not connected, entering light sleep...");
    delay(1000); // 模拟低功耗等待
  }
  if(model_var == 2){
    ObstacleAvoidance();
  }else if(model_var == 3){
    Follow();
  }else if(model_var == 4){
    Tracking();
  }else{
    model_var = 1;
    if(DownServoMark == 8) rotateLeft();
    if(DownServoMark == 9) rotateRight();
  }
}

