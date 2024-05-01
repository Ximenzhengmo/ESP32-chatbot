# AI-smart-assistant

BUAA  电子信息工程学院  第三届电子信息智能硬件创新大赛  校园网范围内的AI语音助手  帕主任的鹰

> ESP32-software by 张尚谋

## 开发环境()
* **esp-idf**: v4.4.6
* **esp-adf**: v2.6  

## 文件介绍(files introduction)
* **main/** : source and header file
* **python_script/** : python scripts for debug or better use
* **esp_tts_voice_data_xiaole.dat** : esp-tts file to write to Flash `voice_data` part in offset `0x310000`
* **partitions.csv**: partition chart for flash use
* **sdkconfig-template**: a template for sdkconfig, you can remove `-template` to use it in your project
  