#include <queue>
#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// 定义服务和特性 UUID
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef1"
#define CHARACTERISTIC_UUID "abcdef01-2345-6789-1234-56789abcdef0"

//右
#define RightFrontFWD 27  //右前轮前进  
#define RightFrontBWD 23  //右前轮后退
#define RightBackFWD  33  //右后轮前进  
#define RightBackBWD  32  //右后轮后退

//左
#define LeftFrontFWD 18   //左前轮前进  
#define LeftFrontBWD 19   //左前轮后退
#define LeftBackFWD  26   //左后轮前进  
#define LeftBackBWD  25   //左后轮后退

// 超声波模块引脚
#define Trigpin 13
#define Echopin 12

//舵机
#define MOTOR 2

// 循迹控制引脚
#define LEFT_LINE_TRACJING      34
#define CENTER_LINE_TRACJING    35
#define right_LINE_TRACJING     14

//控制太阳能马达 光敏1234通道
const int analogPin1 = 4;  // 第一个模拟通道连接到4引脚
const int analogPin2 = 15;  // 第三个模拟通道连接到15引脚

#define motorPin1 16 // 第二个马达控制引脚16，用于PWM控制
#define motorPin2 17  // 第四个马达控制引脚17，用于PWM控制

Servo myServo;  // 创建一个Servo对象
int model_var = 1;
int Left_Tra_Value;
int Center_Tra_Value;
int Right_Tra_Value;
int Black_Line = 1500;
int midAngle = 90;
unsigned long Time_Echo_us;
// unsigned long Len_mm;
unsigned long mid_distance;
// 全局变量
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false; // 记录 BLE 连接状态

std::queue<char> commandQueue;//全局指令缓冲区    

// 小车控制函数声明
void moveForward();
void moveBackward();
void rotateLeft();
void rotateRight();
void stopMoving();
void strafeLeft();
void strafeRight();
long getDistance();

void ObstacleAvoidance(){
  mid_distance = getDistance();
  Serial.print("Present Distance is: "); 
  Serial.print(mid_distance, DEC); //output result to Serial monitor
  Serial.println("mm"); //output result to Serial monitor
  if(mid_distance < 150 && mid_distance > 0)
  {
    stopMoving();
    Serial.print("mid_distance: ");
    Serial.println(mid_distance);

    myServo.write(140); //左
    delay(1000);
    unsigned long leftDistance = getDistance(); // 测量并存储距离
    Serial.print("leftDistance: ");
    Serial.println(leftDistance);
    delay(10);

    myServo.write(40);//右
    delay(1000);
    unsigned long rightDistance = getDistance(); // 测量并存储距离
    Serial.print("rightDistance: ");
    Serial.println(rightDistance);
    delay(10);

    myServo.write(midAngle);

    moveBackward();//后退
    delay(200);

    if(leftDistance > rightDistance)
    {
      rotateLeft();//左移
      delay(800);
      moveForward();
    }
    if(rightDistance > leftDistance)
    {
      rotateRight();//右移
      delay(800);
      moveForward();
    }
    if(mid_distance > leftDistance && mid_distance > rightDistance)
    {
      moveBackward();//后退
      delay(200);
    }
  }else if(mid_distance > 150){
    moveForward();
  }else{
    Serial.println("dis=0");
  }
}

void Follow(){
  mid_distance = getDistance();
  Serial.print("Present Distance is: "); 
  Serial.print(mid_distance, DEC); //output result to Serial monitor
  Serial.println("mm"); //output result to Serial monitor
  if(mid_distance < 150 && mid_distance>0)
  {
    moveBackward();
  }

  if(150 <= mid_distance && mid_distance  <= 200)
  {
    stopMoving();
  }

  if(mid_distance  >= 200)
  {
    moveForward();
  }
}

void Tracking(){
  Left_Tra_Value = analogRead(LEFT_LINE_TRACJING);
  Center_Tra_Value = analogRead(CENTER_LINE_TRACJING);
  Right_Tra_Value = analogRead(right_LINE_TRACJING);
  Serial.print("Left: ");
  Serial.print(Left_Tra_Value);
  Serial.print("\tCenter: ");
  Serial.print(Center_Tra_Value);
  Serial.print("\tRight: ");
  Serial.println(Right_Tra_Value);

  if (Left_Tra_Value < Black_Line && Center_Tra_Value >= Black_Line && Right_Tra_Value < Black_Line)
  {
    moveForward();
  }

  if (Left_Tra_Value >= Black_Line && Left_Tra_Value > Right_Tra_Value)
  {
    TrackLeft();//左移
  }

  if (Right_Tra_Value >= Black_Line && Right_Tra_Value > Left_Tra_Value)
  {
    TrackRight();//右移
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

void Light_val() {      //比较光敏值
  // 读取第一个模拟通道的电压值
  int sensorValue1 = analogRead(analogPin1);
  Serial.print("Received Light1: ");
  Serial.println(sensorValue1);
  // 读取第二个模拟通道的电压值
  int sensorValue2 = analogRead(analogPin2);
  Serial.print("Received Light2: ");
  Serial.println(sensorValue2);

  if((sensorValue1 >= 200) && (sensorValue1-sensorValue2 >= 100)){
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    delay(30);
    digitalWrite(motorPin2, LOW);
    delay(50);
  }else if((sensorValue2 >= 200) && (sensorValue2-sensorValue1 >= 100)){
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    delay(30);
    digitalWrite(motorPin1, LOW);
    delay(50);
  }else{
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  }
}

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
      commandQueue.push(cmd);
      pCharacteristic->setValue(cmd);
    }
  }
};

void processCommand(){
  if(!commandQueue.empty()){
    char cmd = commandQueue.front();
    commandQueue.pop();
          // 执行对应的指令
      switch (cmd) {
        case 'W':
          if(model_var == 1) moveForward();
          break;
        case 'S':
          if(model_var == 1) moveBackward();
          break;
        case 'A':
          if(model_var == 1) rotateLeft();
          break;
        case 'D':
          if(model_var == 1) rotateRight();
          break;
        case 'B':
          if(model_var == 1) stopMoving();
          break;
        case 'R':
          if(model_var == 1) leftWard();
          break;
        case 'T':
          if(model_var == 1) rightWard();
          break;
        case 'Y':
          if(model_var == 1) leftDown();
          break;
        case 'U':
          if(model_var == 1) rightDown();
          break;
        case 'Q':
          if(model_var == 1) strafeLeft();
          break;
        case 'E':
          if(model_var == 1) strafeRight();
          break;
        case '1':
          model_var = 1;
          stopMoving();
          pCharacteristic->setValue("model_var = 1");
          break;
        case '2':
          model_var = 2;
          pCharacteristic->setValue("model_var = 2");
          break;
        case '3':
          model_var = 3;
          pCharacteristic->setValue("model_var = 3");
          break;
        case '4':
          model_var = 4;
          pCharacteristic->setValue("model_var = 4");
          break;
        case '7': {
          model_var = 7;
          // pCharacteristic->setValue(distStr.c_str());
          break;
        }
        default:
          model_var = 1;
          stopMoving();
          pCharacteristic->setValue("Invalid command, stopped.");
          break;
      }
    // delay(100);
  }
}

void setup() {
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

  // 初始化引脚
  myServo.attach(MOTOR);  // 将舵机的信号线连接到ESP32的GPIO 
  myServo.write(midAngle);

  pinMode(Trigpin,OUTPUT);
  pinMode(Echopin,INPUT);
  
  pinMode(LEFT_LINE_TRACJING, INPUT);
  pinMode(CENTER_LINE_TRACJING, INPUT);
  pinMode(right_LINE_TRACJING, INPUT);

  pinMode(RightFrontFWD, OUTPUT);
  pinMode(RightFrontBWD, OUTPUT);
  pinMode(RightBackFWD, OUTPUT);
  pinMode(RightBackBWD, OUTPUT);
  pinMode(LeftFrontFWD, OUTPUT);
  pinMode(LeftFrontBWD, OUTPUT);
  pinMode(LeftBackFWD, OUTPUT);
  pinMode(LeftBackBWD, OUTPUT);

  pinMode(motorPin1, OUTPUT);  // 将马达控制引脚1设置为输出
  pinMode(motorPin2, OUTPUT);  // 将马达控制引脚2设置为输出
}

void loop() {
  // 如果设备未连接，进入低功耗模式
  if (deviceConnected) {
    processCommand();
    if(model_var == 2){
      ObstacleAvoidance();
    }else if(model_var == 3){
      Follow();
    }else if(model_var == 4){
      Tracking();
    }else if(model_var == 7){
      Light_val();
    }else{
      model_var = 1;
    }
  }else{
    Serial.println("Device not connected, entering light sleep...");
    stopMoving();
    delay(1000); // 模拟低功耗等待
  }
}

// 小车运动控制函数

void moveForward() {  //前进
  digitalWrite(RightFrontFWD, HIGH);
  digitalWrite(RightFrontBWD, LOW);       
  digitalWrite(RightBackFWD, HIGH); 
  digitalWrite(RightBackBWD, LOW);  
  
  digitalWrite(LeftFrontFWD, HIGH); 
  digitalWrite(LeftFrontBWD, LOW);       
  digitalWrite(LeftBackFWD, HIGH); 
  digitalWrite(LeftBackBWD, LOW);   
}

void moveBackward() {//后退
  digitalWrite(RightFrontFWD, LOW);  
  digitalWrite(RightFrontBWD, HIGH);
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, HIGH); 
  
  digitalWrite(LeftFrontFWD, LOW);  
  digitalWrite(LeftFrontBWD, HIGH);
  digitalWrite(LeftBackFWD, LOW);  
  digitalWrite(LeftBackBWD, HIGH);
}

void TrackLeft() { //左转
  digitalWrite(RightFrontFWD, HIGH);
  digitalWrite(RightFrontBWD, LOW); 
  digitalWrite(RightBackFWD, HIGH); 
  digitalWrite(RightBackBWD, LOW); 
  
  digitalWrite(LeftFrontFWD, LOW); 
  digitalWrite(LeftFrontBWD, LOW); 
  digitalWrite(LeftBackFWD, LOW); 
  digitalWrite(LeftBackBWD, LOW);
}

void TrackRight() { //右转
  digitalWrite(RightFrontFWD, LOW);
  digitalWrite(RightFrontBWD, LOW); 
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, LOW);
  
  digitalWrite(LeftFrontFWD, HIGH);
  digitalWrite(LeftFrontBWD, LOW);
  digitalWrite(LeftBackFWD, HIGH);
  digitalWrite(LeftBackBWD, LOW); 
}

void rotateLeft() { //左转
  digitalWrite(RightFrontFWD, HIGH);
  digitalWrite(RightFrontBWD, LOW); 
  digitalWrite(RightBackFWD, HIGH); 
  digitalWrite(RightBackBWD, LOW); 
  
  digitalWrite(LeftFrontFWD, LOW); 
  digitalWrite(LeftFrontBWD, HIGH); 
  digitalWrite(LeftBackFWD, LOW); 
  digitalWrite(LeftBackBWD, HIGH);
}

void rotateRight() { //右转
  digitalWrite(RightFrontFWD, LOW);
  digitalWrite(RightFrontBWD, HIGH); 
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, HIGH);
  
  digitalWrite(LeftFrontFWD, HIGH);
  digitalWrite(LeftFrontBWD, LOW);
  digitalWrite(LeftBackFWD, HIGH);
  digitalWrite(LeftBackBWD, LOW); 
}

void stopMoving() {//停止
  digitalWrite(RightFrontFWD, LOW);
  digitalWrite(RightFrontBWD, LOW);
  digitalWrite(RightBackFWD, LOW);
  digitalWrite(RightBackBWD, LOW);
  digitalWrite(LeftFrontFWD, LOW);
  digitalWrite(LeftFrontBWD, LOW);
  digitalWrite(LeftBackFWD, LOW);
  digitalWrite(LeftBackBWD, LOW);
}

void strafeRight() {//右移
  digitalWrite(RightFrontFWD, LOW);
  digitalWrite(RightFrontBWD, HIGH); 
  digitalWrite(RightBackFWD, HIGH);
  digitalWrite(RightBackBWD, LOW);

  digitalWrite(LeftFrontFWD, HIGH);
  digitalWrite(LeftFrontBWD, LOW);
  digitalWrite(LeftBackFWD, LOW);
  digitalWrite(LeftBackBWD, HIGH);
}

void strafeLeft() {//左移
  digitalWrite(RightFrontFWD, HIGH);
  digitalWrite(RightFrontBWD, LOW);
  digitalWrite(RightBackFWD, LOW);
  digitalWrite(RightBackBWD, HIGH);

  digitalWrite(LeftFrontFWD, LOW);
  digitalWrite(LeftFrontBWD, HIGH);
  digitalWrite(LeftBackFWD, HIGH);
  digitalWrite(LeftBackBWD, LOW);
}

void leftWard() {  //左上
  digitalWrite(RightFrontFWD, HIGH);
  digitalWrite(RightFrontBWD, LOW);       
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, LOW);  
  
  digitalWrite(LeftFrontFWD, LOW); 
  digitalWrite(LeftFrontBWD, LOW);       
  digitalWrite(LeftBackFWD, HIGH); 
  digitalWrite(LeftBackBWD, LOW);   
}

void rightWard() {  //右上
  digitalWrite(RightFrontFWD, LOW);
  digitalWrite(RightFrontBWD, LOW);       
  digitalWrite(RightBackFWD, HIGH); 
  digitalWrite(RightBackBWD, LOW);  
  
  digitalWrite(LeftFrontFWD, HIGH); 
  digitalWrite(LeftFrontBWD, LOW);       
  digitalWrite(LeftBackFWD, LOW); 
  digitalWrite(LeftBackBWD, LOW);   
}

void leftDown() {//左下
  digitalWrite(RightFrontFWD, LOW);  
  digitalWrite(RightFrontBWD, LOW);
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, HIGH); 
  
  digitalWrite(LeftFrontFWD, LOW);  
  digitalWrite(LeftFrontBWD, HIGH);
  digitalWrite(LeftBackFWD, LOW);  
  digitalWrite(LeftBackBWD, LOW);
}

void rightDown() {//右下
  digitalWrite(RightFrontFWD, LOW);  
  digitalWrite(RightFrontBWD, HIGH);
  digitalWrite(RightBackFWD, LOW); 
  digitalWrite(RightBackBWD, LOW); 
  
  digitalWrite(LeftFrontFWD, LOW);  
  digitalWrite(LeftFrontBWD, LOW);
  digitalWrite(LeftBackFWD, LOW);  
  digitalWrite(LeftBackBWD, HIGH);
}

// 获取超声波传感器距离
long getDistance() {
    digitalWrite(Trigpin, HIGH);
    delayMicroseconds(20);
    digitalWrite(Trigpin, LOW);
    delayMicroseconds(10);
    Time_Echo_us = pulseIn(Echopin, HIGH, 60000); // 设置超时时间为60000微秒
    if(Time_Echo_us > 1) {
        return (Time_Echo_us * 34 / 100) / 2;
    } else {
        return 0; // 如果读取失败，返回0
    }
}
