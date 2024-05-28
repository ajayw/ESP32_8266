/**
 * 程序功能：使用MQTT协议，与阿里云物联网平台进行数据通信。适用的平台为ESP32和ESP8266
 * 使用教程：https://blog.yyzt.site/
 * 注意事项：无
 * 参考资料：https://github.com/0xYootou/arduino-aliyun-iot-sdk
 *          https://github.com/legenddcr/aliyun-mqtt-arduino
 *          https://help.aliyun.com/document_detail/89301.html
 * 阿里云物联网平台网址：https://iot.console.aliyun.com/lk/summary/new
 */

#ifndef _ALIYUNMQTT_H
#define _ALIYUNMQTT_H

#include <ArduinoJson.h>      // 序列化以及反序列化JSON字符串数据
#include <AsyncMqttClient.h>  // 负责整体MQTT协议，以及实现MQTT客户端的功能（包括设置Client端来源、连接服务器、发送数据等）
#include <SHA256.h>           // SHA256加密,Crypto.h

#define YYZT_AliyunMqttLibraryVersion 0.0    // 库版本
#define sha256HmacSize 32  // 哈希值的大小
//#define mqttPort 1883      // MQTT服务器的端口

class AliyunMqtt{
  public:
    /**
   * 函数功能：设置连接阿里云物联网平台的设备证书（也叫三元组）以及地域，即ProductKey、DeviceName、DeviceSecret
   * 参数1：[_productKey] [String] 产品密钥
   * 参数2：[_deviceName] [String] 设备名称
   * 参数3：[_deviceSecret] [String] 设备密钥
   * 参数4：[_region] [String] 地区，默认为上海，即"cn-shanghai"
   * 返回值：无
   * 注意事项：无
   */
  // void AliyunMqtt();
  void setDeviceCertificate(String _productKey, String _deviceName, String _deviceSecret, String _region = "cn-shanghai",uint16_t _mqttPort = 1883);
  /**
   * 函数功能：SHA256加密
   * 参数1：[_signContent] [const String&] 签名内容
   * 参数2：[_deviceSecret] [const String&] 设备密钥
   * 返回值：[String] 加密后的字符串
   * 注意事项：无
   */

  // AsyncMqttClient mqttClient;  // 实例化mqttClient
  private:
    String hmac256(String _signContent, String _deviceSecret);
    char clientId[256] = {0};      // 客户端id
    char mqttUsername[128] = {0};  // MQTT用户名
    char mqttPassword[128] = {0};  // MQTT密码
    char domain[128] = {0};        // 云平台域名
};
#endif

