# E_Paper-HoloCubic
ESP32 墨水屏天气时钟股市信息

移植了部分适合墨水屏的HoloCubic_AIO代码，原作者链接如下：
https://github.com/ClimbSnail/HoloCubic_AIO

添加了墨水屏支持库，原作者链接如下：
https://github.com/HalfSweetStudio/EPaperDrive

本项目中修改点：
1，重新编写了GUI界面，删除动画类效果
2，保留了AIO固件中的天气和bilibili粉丝数应用程序
3，删除了交互逻辑，所有信息都在主页显示
4，添加了股市信息显示

如何使用：
1，将main.cpp中WiFi的ssid和key修改中需要连接的路由器
...
2，修改天气api中的密钥信息，需要自己去天气官网申请
...
3，股市信息的api也需要替换成自己申请的key
...
目前还在完善代码阶段...代码还不能使用，只移植了墨水屏驱动!