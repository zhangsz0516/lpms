## 低功耗管理系统

 lpms : Low Power Management System, 低功耗管理系统

# 特点

- 轻量级，占用较少的ROM与RAM资源。

- 方便移植与适配，提供平台无关的框架，提供平台适配例程，提供移植适配文档

- 把功耗管理系统做成一个软件包，方便更新迭代与使用

- 接口使用简单灵活，框架运行高效


## 技术点

- 增加用户配置的灵活性，提供功耗模块配置文件

- 睡眠管理

- 变频管理

- 模块id，易于与功耗的调试


## 使用要点

- 请求睡眠接口，工作时请求，工作结束释放，向【高功耗】方向请求，默认无任何请求，进入【深睡眠】

- 请求与释放，成对出现，无引用计数

- 功耗模块划分，可以根据外设、线程、任务等等细分。

- 睡眠运行在空闲线程（idle）

- 变频：低频转高频-立即执行，高频转低频-idle执行，频率不变-不执行变频


## 系统组成

- lpms_config.h : 配置用户的模块id
- lpms.c lpms.h : 低功耗框架主文件
- lpms_drv.c lpms_drv.h : 平台适配文件
- lpms_tim.c lpms_tim.h : 平台低功耗定时器，用于tickless、时钟补偿等

## 宏开关

- PM框架使能

- 睡眠管理使能

- 变频管理使能

- 框架配置

- 框架调试开关

## 使用方法

- TODO

## 示例

- TODO