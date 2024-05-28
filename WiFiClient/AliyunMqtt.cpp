#include "AliyunMqtt.h"
AsyncMqttClient mqttClient;  // 实例化mqttClient
// AliyunMqtt::AliyunMqtt(){}
/**
 * 函数功能：SHA256加密
 * 参数1：[_signContent] [const String&] 签名内容
 * 参数2：[_deviceSecret] [const String&] 设备密钥
 * 返回值：[String] 加密后的字符串
 * 注意事项：无
 */
String AliyunMqtt::hmac256(String _signContent, String _deviceSecret) {
  uint8_t hashCode[sha256HmacSize] = {0};
  SHA256 sha256;
  const char* key = _deviceSecret.c_str();
  size_t keySize = _deviceSecret.length();

  sha256.resetHMAC(key, keySize);
  sha256.update(_signContent.c_str(), _signContent.length());
  sha256.finalizeHMAC(key, keySize, hashCode, sizeof(hashCode));

  String sign = "";
  for (uint8_t i = 0; i < sha256HmacSize; i++) {
    sign += "0123456789ABCDEF"[hashCode[i] >> 4];
    sign += "0123456789ABCDEF"[hashCode[i] & 0xf];
  }

  return sign;
}
void AliyunMqtt::setDeviceCertificate(String _productKey, String _deviceName, String _deviceSecret, String _region,uint16_t _mqttPort) {
  String timestamp = String(millis());                                                                                                                      // 获取设备上电的时间戳
  char signContent[512] = {0};                                                                                                                              // 定义签名字符串变量
  sprintf(clientId, "%s|securemode=3,signmethod=hmacsha256,timestamp=%s|", _deviceName.c_str(), timestamp.c_str());                                         // 拼接客户端id
  sprintf(signContent, "clientId%sdeviceName%sproductKey%stimestamp%s", _deviceName.c_str(), _deviceName.c_str(), _productKey.c_str(), timestamp.c_str());  // 拼接签名内容

  hmac256(signContent, _deviceSecret).toCharArray(mqttPassword, sizeof(mqttPassword));  // MQTT密码进行SHA256加密
  sprintf(mqttUsername, "%s&%s", _deviceName.c_str(), _productKey.c_str());
  sprintf(domain, "%s.iot-as-mqtt.%s.aliyuncs.com", _productKey.c_str(), _region.c_str());

  mqttClient.setServer(domain, _mqttPort);
  mqttClient.setClientId(clientId);                                                                                                               // 设置clientId
  mqttClient.setCredentials(mqttUsername, mqttPassword);                                                                                          // 设置MQTT用户名和密码
}
