# 这是一个闲着没事用于学习的项目

​	该项目基于STM32H7(反客科技)开发板硬件做为运行平台，vscode + edie + arm-gnu-gcc + Ozone + OepnOCD 做为开发、调试的环境的学习项目。



## 待实现的一些基础功能

- 串口输出
- qspi flash读写
- qspi flash xip执行
- freeRTOS bring up
- fatFS + SDIO
- slave-usb
  - custom hid
  - USB Mass Storage Class
- lvgl



## 待实现的一些平台内功能

- 集成和实现NanoPlantform(自己搞的一个实验性的跨平台软件框架)
- bootloader
  - 支持USB Mass Storage Class
  - 支持bootloader自更新
  - 支持固件版本查询、校验
  - 支持配置表增删改查
- 纯软件树形通信协议
  - 动态路由表
- 基于qspi flash的一个非容失存储库，类似esp-idf的nvs，但是要更加简洁、减少耦合
- 动态库编译、加载
- 应用模块化，允许模块化编译、升级和调试

