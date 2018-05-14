# 论文

## 绪论

## 相关技术介绍

### MQTT

MQTT全称是Message Queue Telemetry Transport(消息队列遥测传输), 是一个基于订阅/发布范式的消息协议，最早由IBM提出，是为了使物联网设备在复杂网络环境下的进行稳定通信而设计的协议。MQTT协议目前已经成为ISO标准(ISO / IEC 20922 PRF)。由于MQTT协议开放的特点，已经有多种语言实现了MQTT协议，其中最为著名的就是Eclipse Paho Project，提供了C/C++、Java、Python、JavaScript、Go等主流语言的实现。MQTT应用场景十分广泛，包括遥感数据、汽车、智能家居、智慧城市等。运用MQTT协议，物联网设备可以非常方便将数据发往云端，在服务器完成数据处理的工作。

#### MQTT协议的特点

##### 发布/订阅模式

在Machine-to-Machine(机器之间)的通讯中，传统的请求/响应模式不再适用，取而代之的是发布/订阅模式。在请求/响应模式中，发送请求的客户与发送响应的客户是一种同步关系，就好像打电话一样，需要一直等到对方接电话了才能够开始交流；在发布/订阅模式中，消息的发布者与订阅者之间是异步的关系，这意味着发布者和订阅者之间不需要建立直接的联系，就好像发邮件一样，什么时候发邮件都行，等收件人有空去查看邮件就行了。

##### 主题

MQTT通过主题Topic对消息进行分类，消息的发布者和订阅者必须指定感兴趣的主题。主题本质就是一个字符串，类似文件路径，通过反斜杠表示多个层级关系。
消息的发布者和订阅者可以使用通配符来订阅某一类消息，其中+表示过滤一个层级，#只能出现在主题的最后表示过滤任意级别的层级。例如：

- building-a/floor-1：代表A楼1层的设备
- +/floor-1：代表任何楼的1层的设备
- building-a/#：代表A楼所有的设备

##### 服务质量

MQTT被设计能够工作多种网络环境下，为了在不同网络环境下提供消息的可靠性，MQTT提供了三种不同级别的服务质量(Quality of Service, QoS)：

- Level 0：消息的发布者会尽量发送消息，但是如果消息在中途丢失，发布者不会重新发送丢失的消息。因此不能确保消息的订阅者能够收到消息。
- Level 1：消息的发布者会等待代理服务器的确认，如果发布者没有在一定时间内收到消息的确认，就会至少再发送一次未被确认的消息，直到接受到确认。因此这种方式很可能造成多次重复消息。
- Level 2：与Level 1的至少一次重发不同，如果消息的发布者没有收到对消息的确认，发布者只会再重发一次未被确认的消息，不管这次重发的消息有没有收到确认。

##### 遗嘱机制

当MQTT客户端因为网络因素等其它非正常原因导致连接断开时，用户可以启用客户端的遗嘱机制，这样在客户端断开连接时，代理服务器将以发布话题的形式告知对该客户端感兴趣的其它客户端。

#### Mosquitto代理服务器介绍

当设备之间通过MQTT协议进行通讯时，需要有中间服务器来管理不同的话题、转发消息、进行权限认证、加密消息等工作，完成这部分工作的中间件称其为MQTT代理服务器，常见的MQTT代理服务器有Mosquitto、HiveMQ、EMQ等，因为开源的Mosquitto非常方便进行二次开发，因此本课题采用的是Mosquitto作为实验的MQTT代理服务器。

##### 使用SSL/TLS加密消息

MQTT协议使用TCP协议进行传输，默认TCP协议明文传输数据，将数据直接暴露在复杂的网络环境中会带来安全隐患，数据可能会被窃取、篡改，因此对数据进行加密显得十分重要。  
SSL(Secure Sockets Layer)最初由Netscape公司设计，为网络通信提供一种数据安全以及数据完整性的协议，在传输层为网络链接进行加密。TLS(Transport Layer Security)是SSL的继任者，TLS 1.0建立在SSL 3.0的基础上。目前应用最广泛的是TLS 1.0，但是主流浏览器已经支持TLS1.2了。  
在SSL/TLS协议中：所有信息都是加密传输的，第三方无法窃听；通信双方会对信息进行校验，一旦被篡改，通信双方会立即发现；配备身份证书，防止身份被他人冒充。
在SSL/TLS握手阶段，客户端先向服务器请求公钥，然后使用公钥加密信息，服务器收到密文后，使用私钥解密。为了防止公钥被篡改，将公钥存在数字证书中，只要证书是可信的，公钥就是可信的。  
但是使用非对称加密开销太大，因此在握手阶段完成后，客户端和服务器协商生成了会话密钥，之后的通信使用会话密钥进行对称加密，非对称加密只用于握手阶段，减少了后续通信加密运算的时间。因此，SSL/TLS协议的过程大致为：

- 客户端向服务器索取公钥并进行验证
- 客户端和服务器协商生成会话密钥
- 客户端和服务器使用会话密钥对通信进行加密

Mosquitto支持对消息进行SSL/TLS加密，本课题使用的是TLS 1.0版本，对消息进行加密后，无法再传输过程中窃取篡改消息，提高了数据传输的安全性

##### 基于用户名和密码的认证

Mosquitto支持使用X509证书来进行认证，不过使用证书进行认证会带来一定的额外开销，例如需要关系证书的生命周期，并且本课题实现的系统还需要对用户进行认证，为每个用户颁发证书进行认证登录显然是不合理的。因此本课题使用的是基于用户名和密码的认证。因为Mosquitto只对用户名和密码认证提供有限的支持，不能满足本课题实现系统的要求，因此采用开源的插件mosquitto-auth-plug来实现用户名和密码认证。该插件提供对各种主流数据库的支持，并且与本课题的后台框架Django也能实现较好的融合。

### Docker

软件开发过程中环境配置是一件非常麻烦的工作，不同机器有不同的环境，如何让软件在不同机器上运行起来是一件非常有挑战的事情。

#### 虚拟机技术介绍

为了实现软件带环境发布的目标，可以使用虚拟机技术。虚拟机技术通过模拟一个完整的操作系统来还原软件的运行环境，但是该方案存在以下不足之处：

- 资源占用多。虚拟机模拟的不仅仅是软件所依赖的环境配置，还有整个操作系统，因此即使真正运行的程序占用的内存只有1M，虚拟机也需要几百M的内存才能运行
- 冗余步骤多。因为虚拟机模拟的是整个操作系统，因此会带来一些与程序运行无关的额外操作，比如登录操作系统
- 启动慢。虚拟机不仅仅启动我们需要运行的程序，还会启动一系列与操作系统相关的服务，导致启动较

#### Docker技术介绍

Docker属于Linux容器的一种封装，提供简单易用的外部接口，是目前最流行的Linux容器解决方案。与虚拟机不同，Linux容器模拟的不是一个完整的操作系统，而是对不同的进程进行隔离。对于运行在容器内的进程来说，它所见的各种资源都是虚拟的，从而实现与底层系统的隔离。因为容器实现的是进程级别的虚拟化，相比传统的虚拟机技术而言，有很多优势：

- 启动速度快。一个容器对于底层操作来说就是一个进程，因此启动一个容器就是启动本机的一个进程，而不是启动一整个操作系统，因此启动速度非常快
- 资源占用少。一方面容器提供的进程级别的虚拟化，相比于虚拟机来说，不会占用不需要使用的资源；另一方面容器之间可以共享资源，但是虚拟机是独享资源
- 打包体积小。打包的容器只包含需要使用的组件，而虚拟机需要打包一整个操作系统，所以容器文件要比虚拟机占用的空间小

### Django

#### 框架介绍

Django是一个高级的Python网络框架，使用Django可以快速开发安全和可维护的网站。Django负责处理网络开发中麻烦的部分，开发者可以专注于上层应用程序的开发，无需关心底层网络通讯的细节。使用Django开发可以带来以下优点：

- 通用性。Django可以适用于开发几乎任何类型的网站，包括维基、社交网站等等。Django也可以与目前任何客户端框架一起工作，并且可以提供几乎任何格式的内容，例如HTML, RSS, JSON等等
- 安全性。默认情况下，Django可以防范许多攻击，包括SQL注入、跨网站伪造请求和点击劫持
- 可维护性。Django代码的编写遵循一定的设计原则和模式，鼓励创建可重复使用和可维护的代码。Django代码遵循不要重复自己的DRY原则，减少了不必要的重复，使得代码数量大大减少，增加了可维护性

#### 架构设计

Django采用MVT的软件设计模式，即模型Model、视图View和模板Template。三者的关系如下表：
层次|职责
---|---
Model|处理与数据有关的所有事务，包括数据的存取、数据有效性的验证
View|处理模型的存取，以及实现页面跳转的逻辑，是模板与模板之间的桥梁
Template|处理如何在页面上展示数据的问题

从上表可以看出，与传统的MVC设计模式相比，Django的视图层仅仅决定要展示哪些数据，而模板层决定要如何展示数据，可以理解为Django将传统MVC设计模式中的视图层划分为新的视图层和模板层，这样划分更具灵活性，使得可以随时替换模板层。至于传统MVC设计模式的控制器部分，在Django中由URLconf实现，URLconf使用正则表达式来匹配URL，然后调用相应的Python函数进行处理。

### LEP

LEP全称是Linux Easy Profiling(Linux易用剖析器)，是由国内嵌入式专家宋宝华和国内Linux内核专家陈莉君等人致力打造的开源项目，目的是为了便利Linux程序员，使其更加容易监控CPU的运行状态，发现程的性能瓶颈，优化程序的执行效率。Linux有很多现成的调试和剖析工具，比如top，iotop等，LEP不仅在功能上是这些传统工具的超集，而且在数据可视化，人机交互等方面对这些工具做了进一步的增强。

#### 架构设计

LEP将数据采集和数据显示分离，数据采集端对应的是LEPD(LEP Daemon)程序，数据显示端对应的是LEPV(LEP Viewer)程序。LEPD完全使用C语言程序编写，使得LEPD不仅可以部署在服务器上，也可以部署在性能相对匮乏的嵌入式板上，采用C语言编写的另一个好处是可以最小化对宿主机性能的影响，保证数据采集的准确性，能够准确反映宿主机的负载情况。LEPV使用Django作为后台网络框架，基于Docker部署，能够非常方便地进行安装。LEPD采集被监控端的数据，WEB服务端通过JSONRPC的形式获得这些原始数据，LEPV使用Python对原始数据进行加工处理，再发送给浏览器，浏览器使用JavaScript以丰富图表形式展示数据。

### mjpg-streamer

mjpg-streamer是一个命令行工具，它能够从多种不同的输入组件获取JPEG帧，并将其复制到多个不同的输出组件。比较常用的输入组件包括input_opencv、input_raspicam和input_uvc；输出组件包括output_http和output_viewer。一种常见的使用方式是使用input_uvc组件在Linux下获取支持UVC标准摄像头的JPEG图像，使用output_http组件进行输出，这样就实现了一个基本的IP Camera的功能，通过访问摄像头的IP地址就可以获取JPEG图像。mjpg-streamer只能运行在可信的网络内，因为在同一网络中的所有设备都可以访问获取到摄像头的图像信息，考虑到安全性问题，本课题实现的系统在互联网传输图像时，不是采用直接暴露摄像头地址的方式，而是利用MQTT代理服务器的加密功能，使得图像信息不被窃取。

### 本章小结

## 基于客户端开发设计与实现

基于客户端的机器人管理系统能够实现在局域网下，通过机器人传输回来的视频数据，利用手柄控制机器人运动。如果采用服务器设计，机器人需要将视频数据传输到服务器，服务器再将视频数据传输给用户，用户将控制指令发送给服务器，服务器再转发控制指令，大大增加了延迟，非常不适合在实时性要求较高的场景使用。因此本课题专门针对机器人控制的场景提出了基于客户端的机器人管理系统的设计。

### 系统设计

通过基于客户端的机器人管理系统，用户可以根据机器人传输回来的视频数据，获取机器人周围环境的情况，通过手柄控制机器人运动。因此，基于客户端的机器人管理系统主要实现接收来自机器人的视频信息、读取手柄控制信息以及将手柄控制信息发送给机器人的功能。  
为了降低延迟，客户端需要和机器人处于同一局域网下，客户端通过设置机器人的IP地址与机器人建立连接，机器人使用mjpg-streamer传输视频信息，客户端使用Joystick Library获取手柄控制信息，通过socket将手柄控制信息发送给机器人，机器人通过socket接收手柄的控制信息，机器人的运动控制由ROS实现。此处需要图

### 系统实现

#### 客户端获取视频信息

在机器人上运行mjpg-streamer程序，通过使用input_uvc组件获取USB摄像头的JPEG格式图像，使用output_http组件在机器人上开启HTTP服务器，外部可以通过访问HTTP服务器获取JPEG格式图像。在编译好的mjpg-streamer项目下找到可执行文件mjpg_streamer，在机器人端执行以下命令开启视频传输：

```
./mjpg_streamer -i "./input_uvc.so" -o "./output_http.so -w ./www"
```

执行上述命令后，默认情况下，会在机器人端的8080端口开启HTTP服务器，通过访问HTTP服务器相应的URL就可以获取视频数据。  
客户端使用socket连接机器人端的HTTP服务器。socket可以使用HTTP协议与HTTP服务器进行通信。HTTP协议是纯文本协议，意味着使用socket与服务器建立连接后，直接传递纯文本就可以了。客户端的MjpegClient负责与机器人进行视频传输。通过调用MjpegClient的run方法向机器人端的HTTP服务器发送请求，为了获取机器人采集到的视频数据，需要向HTTP服务器发送GET请求，服务器通过GET请求携带的action参数来进行不同的处理，将action参数设置为stream可以获得视频流数据。

```
void MjpegClient::run()
{
    imageBuffer.clear();                //imageBuffer存放一张JPEG图片
    imageBuffer.append(0xFF);           //0xFFD8是JPEG图片的SOI(Start Of Image)标志
    imageBuffer.append(0xD8);
    QString str("GET /?action=stream"); //GET请求
    QByteArray qba = str.toLocal8Bit();
    qba += 0x0A;                        //HTTP请求头的结束标志为两个换行符
    qba += 0x0A;
    sock.write(qba);                    //发送请求
}
```

客户端发送请求之后，利用Qt的信号机制，当接收到HTTP服务器的响应后，会调用MjpegClient的recvMjpeg方法。recvMjpeg方法主要完成接收一张完整的JPEG图像的任务。使用socket接收JPEG图像的关键在于判断图像的起始标记和结束标记。JPEG文件内容分为两个部分：标记码和压缩数据。标记码由两个字节组成，每种标记码的第一个字节都是0xFF，后一个字节用来区分不同的标记码，SOI(Start of Image)标记码为0xFFD8，EOI(End of Image)标记码为0xFFD9。因此在socket接收到的数据中，从0xFFD8到0xFFD9之间的数据就构成一张完整JPEG图像。需要注意的是每次socket接收的数据不一定就是一张完整的JPEG图像，可能一张图像被分成多次发送，这样需要保留每次socket接收的数据，直到在接收缓冲区中检测到EOI标记码才可以确定接收到一张完整的图像。

```
void MjpegClient::recvMjpeg()
{
    char lastByte='\0';
    QByteArray buffer;
    buffer = sock.readAll();                          //获取接收到全部数据，注意不一定是一张完整的图像
    for(int i = 0;i<buffer.count();i++)
    {
        if(enRecv) imageBuffer.append(buffer[i]);
        if(lastByte == (char)0xFF)                    //所有的标记码都是以0xFF开头
        {
            if(buffer[i] == (char)0xD8)               //0xFFD8为SOI标记
            {
                enRecv = true;
            }else if(buffer[i] == (char)0xD9)         //0xFFD9为EOI标记
            {
                enRecv = false;
                QPixmap map;
                map.loadFromData(imageBuffer,"JPEG");
                if(!map.isNull())
                    emit getImage(map);
                imageBuffer.clear();
                imageBuffer.append(0xFF);
                imageBuffer.append(0xD8);             //为下一张图初始化SOI标记
            }
        }
        lastByte = buffer[i];
    }
}
```

#### 客户端获取手柄信息

本课题使用的手柄型号为Logitech Extreme 3D Pro，Linux内核自带手柄驱动joystick，因此可以通过读设备文件来获得手柄的输入信息，获取手柄的输入信息非常容易，但是每款手柄的按键都有自己的编码方式，加上本课题使用的手柄官方并没有给出按键的编码表，因此本课题使用的是Wisconsin Robotics公司开源的Joystick Library。Joystick Library是一个跨平台的解决方案，支持获取多款手柄的控制信息，使用起来非常方便。客户端的JoystickClient负责读取手柄的控制信息，并将手柄控制信息通过socket发送给机器人端。JoystickClient继承自QThread，每个实例都是一条独立的线程，这样在读取手柄控制信息的同时不会阻塞主界面的运行。JoystickClient的run方法是主循环，主要负责初始化连接。

```
void JoystickClient::run(){
    while (es.GetNumberConnected() < 1);        //等待手柄连接

    if(init_connect(host, port)==false) return; //连接机器人

    while(true){
        auto& a = es.GetIDs();
        if (a.size() <= 0)
            continue;
        check(a[0]);                            //读取手柄的状态
        usleep(10000);
    }
}
```

es对象的类型是Extreme3DProService，由开源库Joystick Library提供，可以用于读取Logitech Extreme 3D Pro手柄所有按键的输入，本课题实现的客户端只需要获取手柄x轴和y轴的输入，x轴和y轴的输入范围为[-100, 100]之间。获取x轴和y轴的输入之后，将其发送给机器人，机器人获取到输入后，传递给底层控制模块，控制机器人运动。有关机器人端底层控制模块由团队里的另一个同学开发。

```
void JoystickClient::check(int id){
    int x, y;
    if(!es.GetX(id, x)) //获取x轴输入
        x=0;
    if(!es.GetY(id, y)) //获取y轴输入
        y=0;
    send(x, y);         //发送给机器人
}
```

### 本章小结

## 基于服务器开发设计与实现

### 系统设计

基于服务器的机器人后台管理系统实现了以下功能：

- 用户管理功能。包括用户注册、登录、注销
- 机器人管理功能。包括添加删除机器人、静态地为机器人添加或删除传感器以及创建或删除传感器类型
- 数据管理功能。机器人将各种传感器信息上传到服务器后台，既可以实时可视化数据，也可以将数据保存到服务器，方便以后查看
- 远程控制机器人功能。远程通过服务器控制机器人移动

### 系统实现

#### 使用Django作为WEB服务器

Django提供与用户管理、机器人管理相关的功能。

##### 数据库设计

本课题使用的是Mysql数据库，使用Django自带的Models模板创建数据库。包括以下数据表：

###### Users表

键|说明
--|--
id|主键，标识每个用户
password|密码，不存储明文密码
last_login|记录最后登录的时间
is_superuser|超级用户标志位，超级用户可以进入Django管理后台，并且可以绕开Mosquitto的权限检查
username|用户名，用于登录WEB服务器和Mosquitto服务器
first_name|用户名字
last_name|用户姓氏
email|用户邮件
is_staff|设置为True表示用户可以进入Django后台管理
is_active|设置为True表示用户可以登录
date_joined|记录用户注册时间
nickname|用户昵称

###### Devices表

键|说明
--|--
id|主键，标识每个设备
devicename|设备用户名，用于设备登录Mosquitto服务器
password|密码，不存储明文密码
nickname|设备昵称

###### Sensors表

键|说明
--|--
id|主键，标识一种传感器
sensorname|一种传感器的名称
sensorurl|访问一种传感器的url模板

###### DevicesSensors表

键|说明
--|--
id|主键，标识设备和传感器的所属关系
sensorcnt|某个设备拥有某种传感器的数量
device_id|外键，标识某个设备
sensor_id|外键，标识某种传感器

###### ACLs表

ACLs表具有两个功能，一方面记录了用户和设备的所属关系，另一方面作为Mosquitto服务器的访问控制列表。

键|说明
--|--
id|主键，标识一种访问权限或者设备所属关系
clientname|登录Mosquitto服务器的账号，可以是Users表的username或者是Devices表的devicename
topic|某个Mosquitto主题
rw|对某个主题的访问权限，1表示具有订阅权限，2表示具有订阅和发布权限
device_id|Devices表的外键，表示某个设备
user_id|Users表的外键，表示某个用户

##### RESTfull接口设计

为了提供结构清晰、符合标准、易于理解、扩展方便的API接口，更好地实现前后端分离，本课题使用REST原则设计了API接口，提供RESTful服务。后台提供的接口如下所示：

URL|Method|说明
--|--|--|
/users/|POST|注册新用户，用户名只能由字母数字组成
/session/|POST|用于用户登录
/session/|DELETE|用于用户登出
/users/{username}/|GET|返回username用户的信息
/users/{username}/|POST|修改username用户的信息、增删用户设备列表
/users/{username}/|DELETE|删除username用户，与该用户有关的信息都会被删除
/users/{username}/{deviceid}/|POST|在username用户的设备列表中添加deviceid设备
/users/{username}/{deviceid}/|DELETE|在username用户的设备列表中移除deviceid设备
/devices/|POST|添加新的设备, 注意新设备不会添加到当前登陆用户的设备列表中
/devices/{device_id}/|GET|返回device_id设备的信息
/devices/{device_id}/|POST|修改device_id设备的信息
/devices/{device_id}/|DELETE|删除device_id设备
/sensors/|GET|返回目前支持的传感器信息,包括sensorname和sensorurl
/sensors/|POST|添加新的传感器, 注意新添加的传感器并没有添加到任何设备上

### 使用Mosquitto作为MQTT代理服务器

Mosquitto提供数据管理功能，机器人将传感器的数据通过Mosquitto服务器发布在某个话题上，服务器程序订阅感兴趣的话题，就可以获得对应的数据进行处理。

#### 话题设计

话题|说明
--|--
/{id}/cmd|用于向机器人发送命令，可选的命令包括monitor_open, monitor_close, mcamera_open, mcamera_close, bundle_open, bundle_close,用于开关各种传感器
/{id}/monitor/{sub_monitor}/raw|机器人内部运行状态的原始数据，由服务器程序monitor进行订阅处理,sub_monitor可选cpu_stat、memory_procrank等
/{id}/monitor/{sub_monitor}|此话题由服务器程序monitor进行发布，外部程序可以订阅此话题进行数据的可视化显示
/{id}/mcamera/{mcamera_id}|机器人将视频数据发送到该话题上，外部程序订阅此话题可以实时接收视频数据；mcamera_id从0开始编号，如果有多个摄像头，编号依次递增
/{id}/{sensor_type}/{sensor_id}/realtime|实时显示传感器数据，sensor_type可选sonar、laser、gesture；sensor_id从0开始编号，如果有多个同种类型的传感器，编号依次递增
/{id}/{sensor_type}/{sensor_id}/replay|向对应的timemachine发送命令replay_open，服务器程序timemachine就会读取保存的历史数据进行发送，外部程序订阅此话题可以实现接收历史数据的功能
/{id}/{sensor_type}/{sensor_id}/timemachine|由服务器程序timemachine进行订阅，主要用于接收外部程序的命令，可选的命令包括record_open, record_close, replay_open, replay_close

#### 数据采集

本课题采集的数据包括超声波测距信息、姿态传感器空间信息、激光测距信息、摄像头图像信息以及机器人内部资源使用情况。超声波传感器、姿态传感器和激光传感器的数据通过编写stm32单片机程序进行采集，摄像头图像信息借助mjpg-streamer开源项目进行采集，通过移植LEP开源项目获取机器人内部资源使用情况。

#### 数据处理

机器人将采集的数据通过MQTT协议发送到服务器，在服务器端编写服务器程序，通过订阅相关的话题获取数据进行处理，借助MQTT协议，编写服务器程序非常容易。目前实现的服务器程序有monitor和timemachine。

##### 服务器程序monitor

服务器程序monitor的主要工作是订阅/{id}/monitor/{sub_monitor}/raw话题，获取机器人内部资源使用情况的原始数据，参考LEP开源项目的数据处理方式，对原始数据进行处理以便进行可视化显示，将处理后的数据发布到话题/{id}/monitor/{sub_monitor}上，这样外部程序就可以通过订阅该话题进行数据的可视化显示。

##### 服务器程序timemachine

服务器程序timemachine是一个多线程程序，它实现了超声波传感器、姿态传感器和激光传感器的保存和恢复历史数据的功能，通过订阅话题/{id}/{sensor_type}/{sensor_id}/timemachine接收控制命令，控制命令包括record_open、record_close、replay_open、replay_close。当timemachine接收到record_open命令时，就会开辟一条新的线程订阅话题/{id}/{sensor_type}/{sensor_id}/realtime，将接收到的数据保存到文件中，目前对于一个传感器只能保存一次历史数据，下次保存就会清空上次的历史数据；当timemachine接收到record_close命令时，就会取消订阅相应的话题，完成一次历史数据的保存工作；当接收到replay_open时，timemachine就会读取对应话题的历史数据文件，将历史数据发布到话题/{id}/{sensor_type}/{sensor_id}/replay上，外部程序就可以像订阅realtime一样接收到历史数据；当接收到replay_close时，timemachine就会停止发布replay消息，完成一次历史数据的发送工作。

### 本章小结

## 系统测试

### 测试环境搭建

本课题实现的机器人后台管理系统分为三个部分，运行在机器人上的play-mqtt-daemon，运行在服务器端的play-mqtt-viewer以及客户端jscontroller。下面详细介绍这三个部分。

#### 客户端jscontroller

客户端jscontroller是基于Qt编写的，虽然Qt具有跨平台特性，但是程序读取游戏手柄时使用了Joystick Library, 其只提供了对Linux和Windows支持，因此需要根据具体运行环境重新编译Joystick Library。本课题的测试环境是。

##### 项目目录

##### 项目构建

#### play-mqtt-daemon

##### 项目目录

```
.
├── build.sh                    # 构建项目的脚本
├── controller                  # main程序的源代码
├── lepd                        # 开源的lepd项目
├── main                        # 可执行主程序
├── mjpg-streamer               # 开源的mjpg-streamer项目
├── paho.mqtt.c                 # 开源的mqtt C语言实现
├── paho.mqtt.cpp               # 开源的mqtt C++语言实现
├── play-mqtt-daemon.service    # 与systemd服务有关
├── play-mqtt-daemon.service.im # systemd服务模板文件
├── start.sh                    # 启动主程序
└── stop.sh                     # 停止主程序
```

##### 项目构建

本课题的play-mqtt-daemon部分的运行环境是友善之臂公司的NanoPC-T3 Plus，运行的操作系统为友善之臂公司基于Ubuntu core定制的Friendly core。运行play-mqtt-daemon的项目步骤如下：

- 执行命令`git clone https://github.com/ZevveZ/play-mqtt-daemon.git`克隆项目到本地
- 进入项目文件夹后，执行命令`./build.sh`构建项目，需要注意的是该脚本仅仅在实验环境测试成功，其他环境未进行测试，build.sh脚本主要完成以下工作：
  - 安装依赖的软件包
  - 编译项目的各个模块
  - 加入到systemd服务，下次开机自动启动
- 在构建项目成功后，执行命令`./start.sh`后台运行程序，执行命令`./stop.sh`停止程序运行

#### play-mqtt-viewer

##### 项目目录

```
.
├── build.sh            # 构建项目的脚本
├── docker-compose.yml  # 使用docker-compose管理容器
├── mosquitto           # 存放mosquitto的配置文件
├── mosquitto-auth-plug # mosquitto-auth-plug的源代码
├── mysql               # 存放mysql数据库的数据文件
├── site                # 存放django项目
├── start.sh            # 启动项目所有容器
├── stop.sh             # 停止项目所有容器
└── vue-docker-demo     # 存放前端vue项目
```

##### 项目构建

本课题的play-mqtt-viewer的运行环境为Centos 7, 但是由于使用docker进行部署，能够在多种不同环境进行部署运行，运行play-mqtt-viewer项目的步骤如下：

- 执行命令`git clone https://github.com/ZevveZ/play-mqtt-viewer.git`克隆项目到本地
- 进入项目文件夹后，执行命令`./build.sh`构建项目，需要注意的是该脚本仅仅在实验环境测试成功，其他环境未进行测试，build.sh脚本主要完成以下工作：
  - 安装依赖的软件包
  - 构建项目依赖的所有容器，包括mysql、docker、mosquitto和vue
- 在构建项目成功后，执行命令`./start.sh`启动所有容器，之后执行命令`./stop.sh`关闭所有容器

### 运行效果

### 本章小结

## 结论