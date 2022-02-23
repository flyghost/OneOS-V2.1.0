# 版本历史：

### 版本：MPY_V2.1

#### 新增功能：
1.标准库
thread:
math:
ubinascii:
ucollections:
uarray:
uhashlib:
ussl:
usocket:
uselect:
uheapq:
ujson:
ure:
uzlib:
utime:
uctypes:
urandom:
2.特定库：
ucryptolib:

3.Machine库：
I2C:
SPI：

其他功能：
1.文件系统挂载支持FLASH分区挂载文件系统。
2.支持repl开关和占用内存配置。


### 版本：MPY_V2.0

#### 新增功能：

## Machine库

片上资源硬件控制模块库。

#### class machine.Pin
machine.Pin 类是 machine 模块下面的一个硬件类，用于对 GPIO 引脚的配置和控制，并提供对 Pin 设备的操作方法。
Pin 对象用于控制GPIO引脚。通常与一个物理引脚相关联，可以驱动输出电平和读取输入电平。Pin 类中有设置引脚模式（输入/输出）的方法，也有获取和设置引脚电平的方法。
​Pin对象不用于引脚的复用，只用于GPIO输入与输出模式，对于复用模式可直接使用对应功能的类对象进行使用。

**常量**

| 引脚模式  |说明                                 |
| ---------------------- | ---------------------------------- |
| Pin.IN     | 设置为输入模式。     |
| Pin.OUT_PP | 设置为推挽输出模式。 |
| Pin.OUT_OD | 设置为开漏输出模式。 |

| 中断触发类型           |说明                                 |
| ---------------------- | ---------------------------------- |
| Pin.IRQ_RISING         | 外部中断模式：上升沿触发中断。       |
| Pin.IRQ_FALLING        | 外部中断模式：下降沿触发中断。       |
| Pin.IRQ_RISING_FALLING | 外部中断模式：上升和下降沿触发中断。 |
| Pin.IRQ_LOW_LEVE       | 外部中断模式：低电平触发。           |
| Pin.IRQ_HIGH_LEVEL     | 外部中断模式：高电平触发。           |

| 上/下拉模式   | 说明 |
| ------------- | ----------------------------- |
| Pin.PULL_DOWN | 选择上/下拉模式：使能下拉模式。 |
| Pin.PULL_UP   | 使能上拉模式。                  |

**函数**

| 函数名称 | Pin.index(base，port)                                        |
| -------- | ------------------------------------------------------------ |
| 功能     | 获取操作系统底层的GPIO索引号（OneOS自有的一套GPIO-Pin的映射关系）。 |
| 参数说明 | base： 传入Pin的组号，如‘A’ ，‘B’等。<br />port： 传入Pin的端口号。 |
| 返回值   | 返回索引号。                                                   |
​	注：该函数用于pin脚的索引，如操作"PA_3"，则传入Pin.index('A', 3)，然后使用返回值进行Pin对象的构造。

**构造函数**

​		Pin对象的构造函数说明如下：

| 构造函数 | class machine.Pin(id, mode = -1, value=-1,  pull = -1)        |
| -------- | ------------------------------------------------------------ |
| 功能     | 构造一个Pin对象。                                              |
| 参数说明 | id：可采用元组形式(name,pin)传入参数或者只传pin参数，其中name为自定义的引脚名，pin为设备驱动的引脚号（Pin.index计算得到），如传入：(“led1”,22)，表示自定义引脚名为“led1”，其引脚号为22；若只传入22，则表示引脚名为NULL，引脚号为22；<br />  mode: 指定引脚模式，可选常量见Pin常量中引脚模式部分相关常量。 <br />value: 如果配置成输出形式，可以通过这个参数设定初始输出值，可传入逻辑0或者逻辑1。<br />pull: 如果引脚可以软件配置上下拉电阻，通过这个参数指定上下拉模式，参数可选Pin常量中上/下拉模式部分相关常量。<br />若mode、pull、value未传入，则此构造函数不对pin引脚进行相关配置，即按照芯片默认设置或已有配置。 |
| 返回值   | 返回创建的machine.Pin类对象                                  |

 注：1、mode 目前暂不需支持 ALT_PP、ALT_OD、ANALOG。

​         2、value 只对模式为 OUT_PP、OUT_OD 有效。

​           3、id中传入的引脚号为Pin.index()计算得到。

​           4、并不是所有引脚都可设置为指定的模式，用户需参照datasheet配合使用，避免设置不生效或返回异常。
**实例方法**
​		machine.Pin 类包括如下实例方法

| 方法名       | 说明                         |
| ------------ | ---------------------------- |
| Pin.init()   | 初始化Pin对象。                |
| Pin.deinit() | 关闭Pin对象。                  |
| Pin.value()  | 获取/设置引脚逻辑电平。        |
| Pin.name()   | 获取构造时用户自定义的引脚名。 |
| Pin.pin()    | 获取构造时设置的pin引脚号。    |
| Pin.port()   | 获取pin引脚对应的端口号。      |
| Pin.mode()   | 获取引脚模式。                 |
| Pin.irq()    | 设置引脚的中断模式和回调函数。 |

​     详细说明如下：

| 方法名称 | Pin.init(mode = -1, value=-1, pull = -1)                     |
| -------- | ------------------------------------------------------------ |
| 功能     | 根据参数重新初始化引脚配置。                                   |
| 参数说明 | mode：指定引脚模式，可选常量见引脚模式部分相关常量。 <br />value：如果配置成输出形式，可以通过这个参数设定初始输出值，可传入逻辑0或者逻辑1。 <br /> pull：如果引脚可以软件配置上下拉电阻，通过这个参数指定上下拉模式，参数<br />可选Pin常量中上/下拉模式部分相关常量。 |
| 返回值   | 固定返回None。                                                 |


| 方法名称 | Pin.deinit() |
| -------- | ------------ |
| 功能     | 关闭Pin对象。  |
| 参数说明 | 无参数       |
| 返回值   | 固定返回None。 |

 注：关闭 Pin 对象后，需重新 init 才能正常使用 Pin 对象。

| 方法名称 | Pin.value([x])                                               |
| -------- | ------------------------------------------------------------ |
| 功能     | 获取/设置引脚逻辑电平。                                        |
| 参数说明 | [x]为可选参数，若未给定参数则为读取当前引脚的值，若给定参数则为设定引脚的值，可传入逻辑0或逻辑1。 |
| 返回值   | 没有参数时返回引脚的逻辑电平，有参数时返回None。               |

| 方法名称 | Pin.name()                   |
| -------- | ---------------------------- |
| 功能     | 获取构造时用户自定义的引脚名。|
| 参数说明 | 无参数                       |
| 返回值   | 返回用户自定义的引脚名。       |

| 方法名称 | Pin.pin()                 |
| -------- | ------------------------- |
| 功能     | 获取构造时设置的pin引脚号。 |
| 参数说明 | 无参数                    |
| 返回值   | 返回构造时设置的pin引脚号。 |

| 方法名称 | Pin.port()                  |
| -------- | --------------------------- |
| 功能     | 获取pin引脚对应的port端口号。 |
| 参数说明 | 无参数                      |
| 返回值   | 返回pin引脚对应的port端口号。 |

| 方法名称 | Pin.mode()                                                  |
| -------- | ----------------------------------------------------------- |
| 功能     | 获取当前引脚的模式。                                          |
| 参数说明 | 无参数                                                      |
| 返回值   | 返回当前引脚模式，形式为int类型，其值与对应模式常量之一匹配。 |

| 方法名称 | Pin.irq(irqmode，funcb)                                      |
| -------- | ------------------------------------------------------------ |
| 功能     | 获取/设置引脚的中断模式。                                      |
| 参数说明 | irqmode：传入参数见Pin常量中中断触发类型部分相关常量，模式可按位进行或操作。  <br /> funcb：设置中断回调函数。 |
| 返回值   | 固定返回None                                                 |

**示例**

\>>>from machine import Pin
\>>>Pin.index('E', 7) 
\>>>Pin.index('E', 8)
\>>>Pin.index('E', 9)  
\>>>pin1=Pin(("led_r",71), Pin.OUT_PP)
\>>>pin2=Pin(("led_g",72), Pin.OUT_PP)
\>>>pin3=Pin(("led_b",73), Pin.OUT_PP)
\>>>pin1.init(mode=Pin.OUT_OD,value=1)
\>>>pin2.init(mode=Pin.OUT_OD,value=1)
\>>>pin3.init(mode=Pin.OUT_OD,value=1)
\>>>pin1.pin()
\>>>pin3.port()
\>>>pin1.value(1) #红灯亮
\>>>pin1.value(0) #红灯灭
\>>>pin1.value()
\>>>pin1.deinit()


#### class machine.UART

​		machine.UART 类是machine模块下面的一个硬件类，用于对 UART 的配置和控制，提供对 UART 设备的操作方法。

**常量**

| 数据长度    |                |
| ----------- | -------------- |
| UART.BITS_7 | 数据长度为7bit。 |
| UART.BITS_8 | 数据长度为8bit。 |
| UART.BITS_9 | 数据长度为9bit。 |

| 校验位           |            |
| ---------------- | ---------- |
| UART.PARITY_NONE | 没有校验位。 |
| UART.PARITY_EVEN | 偶数位校验。 |
| UART.PARITY_ODD  | 奇数位校验。 |

| 停止位      |               |
| ----------- | ------------- |
| UART.STOP_1 | 停止位长度为1。 |
| UART.STOP_2 | 停止位长度为2。 |

| 流控类型     |                 |
| ------------ | --------------- |
| UART.FC_RTS  | 硬件流控使用RTS。 |
| UART.FC_CTS  | 硬件流控使用CTS。 |
| UART.FC_NONE | 无流控。          |

**构造函数**

 UART对象的构造函数说明如下：

| 构造函数 | class machine.UART(id, …)                                     |
| -------- | ------------------------------------------------------------ |
| 功能     | 构造一个UART对象。                                             |
| 参数说明 | id：UART编号，如传入1表示uart1设备。<br />  省略部分与Init函数参数一致，可对创建的uart对象进行初始化。|
| 返回值   | 创建成功，返回创建的machine.UART类对象。                           |

**实例方法**

​		machine. UART 类包括如下实例方法：

| 方法名           | 说明                     |
| ---------------- | ------------------------ |
| UART.init()      | 初始化UART总线。           |
| UART.deinit()    | 关闭UART总线。             |
| UART.read()      | 读取字符。                 |
| UART.readline()  | 读一行数据，以换行符结尾。 |
| UART.readchar()  | 读取一个字节数据。         |
| UART.write()     | 将 buf 中的数据写入总线。  |
| UART.writechar() | 向总线写入一个字节数据。   |

详细说明如下：

| 方法名称 | UART.init(baudrate=115200, <br />[bits=8], <br /> [parity=UART.PARITY_NONE],<br />[stop=UART.STOP_1],<br /> [flow= UART.FC_NONE],<br />[timeout=0],<br />[timeout_char=0],<br />[read_buf_len=64]) |
| -------- | ------------------------------------------------------------ |
| 功能     | 根据参数初始化UART配置。                                       |
| 参数说明 | baudrate：必选，设置UART波特率，默认115200。 <br />bits：可选，设置每次发送数据的长度，UART常量中数据长度部分相关常量，默认设置为8。<br /> parity：可选，设置校验方式，UART常量中校验位部分相关常量，默认设置为无校验。 <br /> stop：可选, 设置停止位，UART常量中停止位部分相关常量，默认为1位停止位。  <br /> flow：可选，设置流控类型，UART常量中流控类型部分相关常量，可采用或进行多种选择，默认选择无流控。  <br /> timeout：可选, 设置等待读写一个字符时的超时时间（单位ms），默认为0。<br /> timeout_char：可选，设置写入或读取字符之间等待的超时时间（单位ms）。  <br /> read_buf_len：可选，设置读取缓冲区的字符长度（禁用为0），默认64字节。 |
| 返回值   | 固定返回None                                                 |

| 方法名称 | UART.deinit() |
| -------- | ------------- |
| 功能     | 关闭UART总线  |
| 参数说明 | 无参数        |
| 返回值   | 固定返回None  |


| 方法名称 | UART.read([nbytes])                                          |
| -------- | ------------------------------------------------------------ |
| 功能     | 读取字符                                                     |
| 参数说明 | [nbytes]为可选参数，若传入nbytes，则最多读取nbytes个字节数据；若未传入，则会以阻塞的方式读取尽可能多的数据，大概阻塞20秒左右。 |
| 返回值   | 返回一个包含读入数据的字节对象。如果超时则返回 None。          |

| 方法名称 | UART.readline()                                                                                                |
| -------- | -------------------------------------------------------------------------------------------------------------- |
| 功能     | 读一行数据，以换行符结尾。如果存在这样的行，则立即返回。如果超时，则无论是否存在换行符，都会返回所有可用数据。 |
| 参数说明 | 无参数                                                       |
| 返回值   | 返回行读取数据，如果超时或无可用数据则返回None。               |

注：若数据接收缓存区满但未超时也会返回所有可用数据

| 方法名称 | UART.readchar()                            |
| -------- | ------------------------------------------ |
| 功能     | 读取一个字节数据。                          |
| 参数说明 | 无参数                                     |
| 返回值   | 返回读取到字符ASCII码，如果超时则返回 None。 |



| 方法名称 | UART.write(buf)                       |
| -------- | ------------------------------------- |
| 功能     | 将 buf 中的数据写入总线。              |
| 参数说明 | buf：欲写入的字节数据。                 |
| 返回值   | 返回写入的字节数，如果超时则返回 None。|

| 方法名称 | UART.writechar(buf)              |
| -------- | -------------------------------- |
| 功能     | 将 buf 中的一字节数据写入总线。   |
| 参数说明 | buf：欲写入的一字节数据的ASCII码。 |
| 返回值   | 返回 None。                        |

**示例**

\>>>from machine import UART
\>>>uart2=UART(2)
\>>>uart2.init(115200)
\>>>uart2.write("hello")
\>>>uart2.writechar(49)
(串口输入“hello world\r\n”)
\>>>uart2.read(2)
b'he'
\>>>uart2.readchar()
108
\>>>uart2.read()
b'lo world\r\n'
(串口输入“hello \n world\r\n”)
\>>>uart2.readline()
(串口输入“hello \n world”)
b'hello\r\n'
\>>>uart2.read()
b'world\r\n'
\>>>uart2.deinit()

#### class machine.RTC

​		machine.RTC类是machine模块下面的一个硬件类，用于计时使用。

**构造函数**

​		RTC对象的构造函数说明如下：

| 构造函数 | class machine.RTC() |
| -------- | ------------------- |
| 功能     | 构造一个RTC对象。     |
| 参数说明 | 无                  |
| 返回值   | 返回创建的RTC对象。   |

**实例方法**

​		RTC类包括如下实例方法：

| 方法名        | 说明               |
| ------------- | ------------------ |
| RTC.setdate() | 设置RTC日期。        |
| RTC.settime() | 设置RTC时间。        |
| RTC.now()     | 读取当前日期与时间。 |

​     详细说明如下：

| 方法名称 | RTC.setdate([year], [month], [day])                        |
| -------- | ------------------------------------ |
| 功能     | 设置RTC日期。                          |
| 参数说明 | 传入三个整型参数，分别代表年、月、日。 |
| 返回值   | 固定返回None。                         |

| 方法名称 | RTC.settime([hour], [minute], [second])                        |
| -------- | ------------------------------------ |
| 功能     | 设置RTC时间。                         |
| 参数说明 | 传入三个整型参数，分别代表时、分、秒。 |
| 返回值   | 固定返回None。                         |

| 方法名称 | RTC.now()                     |
| -------- | ----------------------------- |
| 功能     | 读取当前日期与时间。            |
| 参数说明 | 无参数                        |
| 返回值   | 以str的形式返回当前日期与时间。 |

**示例**

\>>> from machine import RTC

\>>> rtc=RTC()  

\>>> rtc.settime(14,43,14)

\>>> rtc.setdate(2020,8,12)

\>>> rtc.now()

'Wed Apr 21 14:43:17 2021'


#### class machine.PWM

​		machine.PWM 类是machine模块下的一个硬件类，用于指定PWM设备的配置和控制，提供对PWM设备的操作方法。

**构造函数**

​		PWM对象的构造函数说明如下：

| 构造函数 | class machine.PWM(id, channel, freq, duty)                   |
| -------- | ------------------------------------------------------------ |
| 功能     | 构造一个PWM对象。                                              |
| 参数说明 | id：PWM设备编号。<br />channel：选择PWM通道号。<br />freq：设置初始化频率。<br />duty：设置占空比。 |
| 返回值   | 返回创建的machine.PWM类对象。                                  |

**实例方法**

​		machine.PWM 类包括如下实例方法

| 方法名           | 说明                           |
| ---------------- | ------------------------------ |
| PWM.init()       | 初始化PWM对象通道              |
| PWM.deinit()     | 关闭PWM对象通道                |
| PWM.freq([freq]) | 获取/设置 PWM 对象的频率       |
| PWM.duty([duty]) | 获取/设置 PWM 对象的占空比数值 |

​     详细说明如下：

| 方法名称 | PWM.init(freq, duty)                                         |
| -------- | ------------------------------------------------------------ |
| 功能     | 初始化PWM对象通道。                                            |
| 参数说明 | freq：设置初始化频率，频率范围视硬件而定。 <br />duty：设置占空比，设置范围为0~100。 |
| 返回值   | 返回None                                                     |

| 方法名称 | PWM.deinit()    |
| -------- | --------------- |
| 功能     | 关闭PWM对象通道。 |
| 参数说明 | 无参数          |
| 返回值   | 返回None。        |

| 方法名称 | PWM.freq([freq])                                             |
| -------- | ------------------------------------------------------------ |
| 功能     | 设置/查看 PWM 对象的频率。                                     |
| 参数说明 | [freq]为可选参数，若传入[freq]，则根据传入参数设置PWM对象频率。设置范围视硬件而定，否则返回当前频率。 |
| 返回值   | 传入参数时，返回None；  未传入参数时返回当前PWM频率。        |

| 方法名称 | PWM.duty([duty])                                             |
| -------- | ------------------------------------------------------------ |
| 功能     | 设置/查看 PWM 对象的占空比数值。                               |
| 参数说明 | [duty]为可选参数，若传入[duty]，则根据传入参数设置PWM对象占空比，否则返回当前占空比。设置范围为0~100。|
| 返回值   | 传入参数时，返回None；  未传入参数时返回当前的占空比。       |

**示例**

\>>> from machine import PWM
\>>> pwm = PWM(4, 1, 1000, 50)
\>>> pwm.init(1000, 50)
\>>> pwm.freq()
1000
\>>> pwm.freq(2000)
\>>> pwm.duty()
50
\>>> pwm.duty(20)
\>>> pwm.deinit()

#### class machine.ADC

​		ADC类是machine模块下面的ADC类，例如在万耦开发板上，对应的就是其ADC设备。

**构造函数**

​		ADC对象的构造函数说明如下：

| 构造函数 | class machine.ADC(id, channel)                  |
| -------- | ----------------------------------------------- |
| 功能     | 构造一个ADC对象。                                 |
| 参数说明 | id: ADC设备的设备号, 如果设-1, 则表示设备号不存在，adc设备名字就是“adc”。 <br />channel: 通道号。|
| 返回值   | 返回创建的ADC对象。                               |
注：因为adc设备支持的通道号各不相同，因此暂不支持在参数输入阶段拦截不支持的通道号。不支持的通道号会在read方法中打印错误。

**实例方法**

​     machine.ADC类包括如下实例方法：

| 方法名       | 说明          |
| ------------ | ------------- |
| ADC.init()   | 初始化ADC对象。 |
| ADC.read()   | 读取电压。      |
| ADC.deinit() | 关闭对象。      |

​     详细说明如下：

| 方法名称 | ADC.init([channel])                  |
| -------- | ------------------------------------ |
| 功能     | 初始化ADC。                              |
| 参数说明 | channel: 可选参数，通道号。不输入时，使用创建对象时的通道。|
| 返回值   | 成功返回None，失败触发ValueError异常。 |

| 方法名称 | ADC.read([channel])                                        |
| -------- | ---------------------------------------------------------- |
| 功能     | 读取ADC。                                                    |
| 参数说明 | channel：可选参数，通道号。不传参数时，读取init通道数据，否则读取创建对象时的通道数据。|
| 返回值   | 成功返回None，失败触发ValueError异常。                       |

| 方法名称 | ADC.deinit()                         |
| -------- | ------------------------------------ |
| 功能     | 关闭ADC。                              |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None，失败触发ValueError异常。 |

**示例**
\>>>from machine import ADC
\>>>temp= ADC(-1, 1)
\>>>temp.init()
\>>>temp.read()
260
\>>>temp.read(2)
323
\>>>temp.deinit()



#### class machine.WDT

WDT类是machine模块下面的WDT类，例如在万耦开发板上，对应的是其mcu的wdt。

**构造函数**

  WDT对象的构造函数说明如下：

| 构造函数 | class machine.WDT() |
| -------- | ------------------- |
| 功能     | 构造一个WDT对象。     |
| 参数说明 | 无                  |
| 返回值   | 返回创建的WDT对象。   |

**实例方法**

​		machine.WDT类包括如下实例方法：

| 方法名       | 说明                       |
| ------------ | -------------------------- |
| WDT.init()   | 初始化看门狗对象。           |
| WDT.start()  | 启动看门狗。                 |
| WDT.feed()   | 喂狗，刷新看门狗。           |
| WDT.deinit() | 关闭看门狗(mpy方法已实现，硬件驱动未实现)。 |

​     详细说明如下：

| 方法名称 | WDT.init()                           |
| -------- | ------------------------------------ |
| 功能     | 初始WDT。                              |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | WDT.start()                          |
| -------- | ------------------------------------ |
| 功能     | 启动看门狗。                           |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | WDT.feed ()                          |
| -------- | ------------------------------------ |
| 功能     | 喂狗，刷新看门狗。                     |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | WDT.deinit ()                        |
| -------- | ------------------------------------ |
| 功能     | 关闭看门狗。                              |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

**示例**
```python
import utime
from machine import WDT

def wdt_test():
    wdt = WDT(1)
    wdt.init() 
    wdt.start()
    wdt.feed()
    count= 0
    while count < 11:
        utime.sleep(1)
        count += 1
        print("watch dog keep alive for :%ds\n" % count)
        wdt.feed()
    count = 0
    while True:
        print("watch dog stop feed for :%d.%ds\n" % (count / 10, count % 10))
        utime.sleep_ms(100)
        count += 1

wdt_test()
```


#### class machine.Timer

​		machine.Timer用于处理周期性和定时性事件。

**常量**

| 定时类型       |                                  |
| -------------- | -------------------------------- |
| Timer.PERIODIC | 定时器会周期性执行设置的回调函数。 |
| Timer.ONE_SHOT | 定时器只执行一次设置的回调函数。   |

**构造函数**

​		Timer对象的构造函数说明如下：

| 构造函数 | class machine.Timer(name)    |
| -------- | ---------------------------- |
| 功能     | 构造一个Timer对象。            |
| 参数说明 | name:以str格式传入定时器名称。 |
| 返回值   | 返回创建的Timer对象。          |

**实例方法**

Timer类包括如下实例方法：

| 方法名         | 说明       |
| -------------- | ---------- |
| Timer.init()   | 启动定时器。 |
| Timer.deinit() | 关闭定时器。 |
| Timer.tick_per_second() | 获取每秒有多少个tick，即系统 1 tick对应的时间。 |

详细说明如下：

| 方法名称 | Timer.init(mode, period,  callback)                          |
| -------- | ------------------------------------------------------------ |
| 功能     | 启动定时器。                                                   |
| 参数说明 | mode：选择定时类型。 <br /> period：选择定时时间，单位ms。  <br /> callback：设置定时回调函数。 |
| 返回值   | 成功返回None，失败触发ValueError异常。                         |

| 方法名称 | Timer.deinit() |
| -------- | -------------- |
| 功能     | 关闭定时器     |
| 参数说明 | 无             |
| 返回值   | 固定返回None   |

**示例**

\>>>def cb_test(device):

...   print("Timer callback test")

... 

\>>> from machine import Timer
\>>> Timer.tick_per_second()
1000
\>>> timer = Timer('test')
\>>> timer.init(timer.ONE_SHOT, 1000, cb_test)

Timer callback test

\>>>timer.deinit()

\>>> timer.init(timer.PERIODIC, 1000, cb_test)

Timer callback test

Timer callback test

Timer callback test

Timer callback test

Timer callback test

Timer callback test

Timer callback test

\>>>timer.deinit()


## Device库

外设资源硬件控制库。

#### class device.ALS_PS
ALS_PS类是device模块下面的一个硬件类，用于读取光强模块与距离感应模块的参数。
万耦开发板上板载了一块光强+距离传感器，型号为AP3216C，通过I2C3进行进行通信。

**常量**

| 采样类型          |  说明                     |
| ----------------- | ---------------------- |
| ALS_PS.LIGHT      | 光照强度采样。 |
| ALS_PS.PROXIMITYS | 距离采样。     |

**构造函数**
ALS_PS对象的构造函数说明如下：

| 构造函数 | class device.ALS_PS()                                   |
| -------- | ------------------------------------------------------------ |
| 功能     | 构造一个ALS_PS对象。    |
| 参数说明 | 无。                    |
| 返回值   | 返回创建的ALS_PS对象 。 |

**实例方法**
device.ALS_PS类包括如下实例方法：

| 方法名          | 说明             |
| --------------- | ---------------- |
| ALS_PS.init()   | 初始化ALS_PS对象。 |
| ALS_PS.read()   | 读取光强/距离值。  |
| ALS_PS.deinit() | 关闭ALS_PS传感器。 |

​ 详细说明如下：

| 方法名称 | ALS_PS.init()              |
| -------- | -------------------------- |
| 功能     | 初始化光强传感器。           |
| 参数说明 | 无。                         |
| 返回值   | 成功返回None，失败触发异常。 |

| 方法名称 | ALS_PS.read([type])                                          |
| -------- | ------------------------------------------------------------ |
| 功能     | 读取光强、距离值。                                             |
| 参数说明 | [type]：采样类型，该参数为可选参数，值为常量中的“采样类型”；传入该参数时，功能为获取指定类型的采样值；未传入该参数时，功能为同时获取光强及距离的采样值。 |
| 返回值   | 未传入参数时，以字典的形式返回光强及距离，如{'PROXIMITYS':  43, 'LIGHT': 474.0}；  传入参数时，返回指定类型的采样值。 |

​     注：光照越强，光强采样值越大，距离越近，距离采样值越大。 

| 方法名称 | ALS_PS.deinit()                      |
| -------- | ------------------------------------ |
| 功能     | 关闭ALS_PS传感器。                     |
| 参数说明 | 无参数                               |
| 返回值   | 成功返回None，失败触发ValueError异常。 |

​     注：关闭ALS_PS传感器后，需重新init才能正常使用ALS_PS对象。

**示例**
\>>>from device import  ALS_PS
\>>> light=ALS_PS()        \# 创建ALS_PS设备对象
\>>> light.init()            \#对象初始化
\>>> light.read(ALS_PS.LIGHT)   \#读取光强
113.0
\>>> light.read(ALS_PS.PROXIMITYS) \#读取距离
1023
\>>> light.read()             \#读取光强及距离
{'PROXIMITYS': 1023, 'LIGHT': 7.0}
\>>> light.deinit()


#### class device.AUDIO

​		AUDIO类是device模块下面的AUDIO类，例如在万耦开发板上，对应音频解码芯片，其麦耳接孔为3.5mm无麦耳机接孔。

**构造函数**

​		AUDIO对象的构造函数说明如下：

| 构造函数 | class device.audio() |
| -------- | -------------------- |
| 功能     | 构造一个AUDIO对象。    |
| 参数说明 | 无                   |
| 返回值   | 返回创建的AUDIO对象。  |

**实例方法**

​		device. AUDIO类包括如下实例方法：

| 方法名         | 说明            |
| -------------- | --------------- |
| AUDIO.init()   | 初始化AUDIO对象。 |
| AUDIO.volume() | 设置AUDIO音量。   |
| AUDIO.player() | 播放wav音频文件。 |
| AUDIO.stop()   | 暂停播放。        |
| AUDIO.start()  | 继续开始播放。    |
| AUDIO.deinit() | 注销AUDIO对象。   |

​     详细说明如下：

| 方法名称 | AUDIO.init()                         |
| -------- | ------------------------------------ |
| 功能     | 初始AUDIO。                            |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | AUDIO.volume(volume)                 |
| -------- | ------------------------------------ |
| 功能     | 设置音量参数。                         |
| 参数说明 | volume:音量。                          |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | AUDIO.player(file) |
| -------- | ------------------ |
| 功能     | 播放wav音频文件。    |
| 参数说明 | file：文件名。       |
| 返回值   | 成功返回None。       |

| 方法名称 | AUDIO.stop() |
| -------- | ------------ |
| 功能     | 暂停播放。     |
| 参数说明 | 无           |
| 返回值   | 成功返回None。 |

| 方法名称 | AUDIO.start() |
| -------- | ------------- |
| 功能     | 继续开始播放。  |
| 参数说明 | 无            |
| 返回值   | 成功返回None。  |

| 方法名称 | AUDIO.deinit()                       |
| -------- | ------------------------------------ |
| 功能     | 关闭AUDIO。                            |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

**示例**

\>>>from device import AUDIO
\>>>audio= AUDIO()
\>>>audio.init()
\>>>audio.volume(35)
\>>>audio.player('3.wav')
\>>>audio.stop()
\>>>audio.start()
\>>>audio.deinit()

#### class device.BEEP

BEEP类是device模块下面的BEEP类，对应开发板上的蜂鸣器

**构造函数**

| 构造函数 | class device.BEEP([pin])                                      |
| -------- | ------------------------------------------------------------ |
| 功能     | 构造一个BEEP对象                                              |
| 参数说明 | 无                                                           |
| 返回值   | 返回创建的BEEP对象                                            |

**实例方法**

device.BEEP类包括如下实例方法：

| 方法名        | 说明                |
| ------------- | ------------------- |
| BEEP.init()   | 初始化BEEP对象      |
| BEEP.on()     | 打开BEEP,  BEEP鸣响 |
| BEEP.off()    | 关闭BEEP, BEEP无声  |
| BEEP.deinit() | 注销BEEP            |

​
详细说明如下：

| 方法名称 | BEEP.init([pin])                                             |
| -------- | ------------------------------------------------------------ |
| 功能     | 初始BEEP                                                     |
| 参数说明 | pin: beep的pin索引值；可以缺省; 当与实例化时同时缺省会报 pin为空的错误 |
| 返回值   | 成功返回None，失败触发ValueError异常                         |

| 方法名称 | BEEP.on()                            |
| -------- | ------------------------------------ |
| 功能     | 打开BEEP                             |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None，失败触发ValueError异常 |

| 方法名称 | BEEP.off()                           |
| -------- | ------------------------------------ |
| 功能     | 关闭BEEP                             |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None，失败触发ValueError异常 |

| 方法名称 | BEEP.deinit()                        |
| -------- | ------------------------------------ |
| 功能     | 注销BEEP                             |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None，失败触发ValueError异常 |

**示例**
\>>> from device import BEEP
\>>> beep = BEEP(18)
\>>> beep.init()
\>>> beep.on()
\>>> beep.off()
\>>> beep.deinit()


#### class device.Humiture

​		Humiture类是device模块下面的Humiture类，例如在万耦开发板上，对应的其温湿度传感器，型号是aht10。

**常量**  

| 采样类型             |          |
| -------------------- | -------- |
| Humiture.TEMPERATURE | 温度采样 |
| Humiture.HUMIDITY    | 湿度采样 |

**构造函数**

Humiture对象的构造函数说明如下：

| 构造函数 | class device.Humiture () |
| -------- | ------------------------ |
| 功能     | 构造一个Humiture对象。     |
| 参数说明 | 无                       |
| 返回值   | 返回创建的温湿度对象。     |

**实例方法**

device.Humiture类包括如下实例方法：

| 方法名            | 说明                     |
| ----------------- | ------------------------ |
| Humiture.init()   | 初始化Humiture对象。       |
| Humiture.read()   | 读取温湿度传感器对象的值。 |
| Humiture.deinit() | 关闭Humiture传感器。       |

详细说明如下：

| 方法名称 | Humiture.init()                      |
| -------- | ------------------------------------ |
| 功能     | 初始Humiture。                         |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | Humiture.read([type])                                        |
| -------- | ------------------------------------------------------------ |
| 功能     | 获取Humiture的温度计和湿度计的值或者只获取其中之一的值。       |
| 参数说明 | [type]：采样类型，该参数为可选参数。                           |
| 返回值   | 1：当无参数时返回值为温度和湿度的字典组合值。 <br />2：当参数为Humiture.TEMPERATURE获取的值是温度值。 <br />3：当参数Humiture.HUMIDITY获取的值是湿度值。  <br />4：失败触发ValueError异常。 |

| 方法名称 | Humiture.deinit()                    |
| -------- | ------------------------------------ |
| 功能     | 关闭Humiture传感器。                   |
| 参数说明 | 无参数                               |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

**示例**

\>>>from device import Humiture
\>>> hum = Humiture()
\>>> hum.init()
\>>> hum.read()
{'HUMIDITY': 43.0, 'TEMPERATURE': 28.0}
\>>> hum.read(hum.TEMPERATURE)
28.0
\>>> hum.read(hum.HUMIDITY)
42.0
\>>> hum.deinit()

#### class device.KEY

​		KEY类是device模块下面的按键类，例如在万耦开发板上，则对应其4个按键。

**构造函数**

KEY对象的构造函数说明如下：

| 构造函数 | class device.KEY(key_num, pin_num) |
| -------- | ---------------------------------- |
| 功能     | 构造一个key对象。                    |
| 参数说明 | key_num : key编号                  |
|          | pin_num：key的pin索引              |
| 返回值   | 返回创建的对应的按键key对象。        |

**实例方法**

device.KEY类包括如下实例方法：

| 方法名         | 说明                         |
| -------------- | ---------------------------- |
| KEY.init()     | 初始化KEY对象。                |
| KEY.callback() | 将一个函数设置为按键回调函数。 |
| KEY.deinit()   | 关闭KEY。                      |

详细说明如下：

| 方法名称 | KEY.init()                           |
| -------- | ------------------------------------ |
| 功能     | 初始化key。                            |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None，失败触发ValueError异常。 |

| 方法名称 | KEY.callback(func)                   |
| -------- | ------------------------------------ |
| 功能     | 设置回调函数。                         |
| 参数说明 | func：回调函数名                     |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | KEY.deinit()                         |
| -------- | ------------------------------------ |
| 功能     | 关闭key。                              |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None，失败触发ValueError异常。 |

**示例**

\>>> def f0(x):
...   print("key0 is pressed")
... 
\>>> def f1(x):
...   print("key1 is pressed")
...

\>>> def f2(x):
...   print("key2 is pressed")
... 

\>>> def f3(x):
...   print("key3 is pressed")
\>>> from device import KEY
\>>> key0 = KEY(0, 45) #C13
\>>> key0.init()
\>>> key0.callback(f0)
\>>> key1 = KEY(1, 58) #D10
\>>> key1.init()
\>>> key1.callback(f1)
\>>> key2 = KEY(2, 57)  #D9
\>>> key2.init()
\>>> key2.callback(f2)
\>>> key3 = KEY(3, 56)  #D8
\>>> key3.init()
\>>> key3.callback(f3)


#### class device.LCD

​LCD类是device模块下面的LCD类，例如在万耦开发板上的LCD显示屏，对应的型号是：st7789vw。

**常量** 

| 屏幕状态 |        |
| -------- | ------ |
| LCD.ON   | 屏幕亮 |
| LCD.OFF  | 屏幕灭 |

| 颜色        |        |
| ----------- | ------ |
| LCD.WHITE   | 白色   |
| LCD.BLACK   | 黑色   |
| LCD.BLUE    | 蓝色   |
| LCD.BRED    | 黑红色       |
| LCD.GRED    | 绿红色       |
| LCD.GBLUE   | 绿蓝色       |
| LCD.RED     | 红色   |
| LCD.MAGENTA | 紫红色 |
| LCD.GREEN   | 绿色   |
| LCD.CYAN    | 青色   |
| LCD.YELLOW  | 黄色   |
| LCD.BROWN   | 棕色   |
| LCD.BRRED   | 棕红色 |
| LCD.GRAY    | 灰色   |
| LCD.GRAY175 | 灰色175       |
| LCD.GRAY151 | 灰色151       |
| LCD.GRAY187 | 灰色187       |
| LCD.GRAY240 | 灰色240       |
**构造函数**

​LCD对象的构造函数说明如下：

| 构造函数 | class device.LCD()      |
| -------- | ----------------------- |
| 功能     | 构造一个LCD对象。         |
| 参数说明 | 无                      |
| 返回值   | 返回创建的对应的LCD对象。 |
 **实例方法**

​device.LCD类包括如下实例方法：

| 方法名       | 说明                                   |
| ------------ | -------------------------------------- |
| LCD.init()   | 初始化LCD对象。                          |
| LCD.light()  | 点亮或或者熄灭LCD。                      |
| LCD.pixel()  | 对屏幕固定的矩形区域进行指定颜色的填充。 |
| LCD.clear()  | 清屏。                                   |
| LCD.color()  | 对字体的前景色和背景色进行设置。         |
| LCD.text()   | 在指定位置以指定大小显示字符串。         |
| LCD.fill()   | 屏幕背景色填充。                         |
| LCD.deinit() | 关闭LCD对象。                            |

详细说明如下：

| 方法名称 | LCD.init()                           |
| -------- | ------------------------------------ |
| 功能     | 初始LCD,此函数可不调用。               |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | LCD.light([state])                                           |
| -------- | ------------------------------------------------------------ |
| 功能     | 点亮或者熄灭LCD或者获取屏幕的状态。                            |
| 参数说明 | 1：LCD.ON或者LCD.OFF代表打开或者关闭LCD显示。<br />2：无参数代表获取屏幕的状态（此时调用必须在调用了light带参数之后方可真正获取到屏幕的状态）。 |
| 返回值   | 成功返回None或者屏幕的状态  失败触发ValueError异常。           |

| 方法名称 | LCD.pixel(x,y,w,h,color)                                     |
| -------- | ------------------------------------------------------------ |
| 功能     | 对LCD的指定区域进行指定颜色的填充                            |
| 参数说明 | x: 起始横坐标 <br />y：起始纵坐标 <br />w：宽度  <br />h：高度  <br />color：颜色 |
| 返回值   | 成功返回None  失败触发ValueError异常                         |

| 方法名称 | LCD.clear()                          |
| -------- | ------------------------------------ |
| 功能     | 清屏，清除整个屏幕。                   |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | LCD.color(fore_color,back_color)                             |
| -------- | ------------------------------------------------------------ |
| 功能     | 设置字体的前景色和背景色。                                     |
| 参数说明 | fore_color：前景色。 <br />back_color：背景色，颜色参数参考颜色常量。 |
| 返回值   | 成功返回None  失败触发ValueError异常。                         |

| 方法名称 | LCD.text(str,x,y,size)                                       |
| -------- | ------------------------------------------------------------ |
| 功能     | 字符串显示                                                   |
| 参数说明 | str：待显示字符串 <br /> x：起始横坐标  <br /> y：起始纵坐标 <br /> size：显示字体大小（驱动只支持16,24,32） |
| 返回值   | 成功返回None  失败触发ValueError异常                         |

| 方法名称 | LCD.fill(color)                      |
| -------- | ------------------------------------ |
| 功能     | 屏幕背景色填充。                       |
| 参数说明 | 参数参考颜色常量。                     |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | LCD.deinit() |
| -------- | ------------ |
| 功能     | 关闭LCD。      |
| 参数说明 | 无           |
| 返回值   | 成功返回None。 |

**示例**
\>>>from device import LCD
\>>>lcd = LCD()
\>>>lcd.light(LCD.ON)
\>>>lcd.pixel(0,0, w= 20,h = 20, col = LCD.RED)
\>>>lcd.pixel(0,0, w= 20,h = 20) \#默认是蓝色BLUE
\>>>lcd.text("hello world", 60,60,16) \#参数 字符串，x,y,字体大小
\>>>lcd.text("hello world", 60,60,24)
\>>>lcd.text("hello world", 60,60,32)
\>>>lcd.light(LCD.ON)
\>>>lcd.light(LCD.OFF)
\>>>lcd.light()
\>>>lcd.light(LCD.ON)
\>>>lcd.fill(LCD.YELLOW)
\>>>lcd.clear()
\>>>lcd.color(LCD.RED,LCD.YELLOW)
\>>>lcd.text("hello world", 60,60,32)
\>>>lcd.deinit()


#### class device.LED

​LED类是device模块下面的LED类，例如在万耦开发板上，对应上面3个颜色的led灯，led的pin脚号，请参考开发板引脚图。

**常量**

| led灯类型  |           |
| --------- | --------- |
| LED.RED   | 红色LED灯 |
| LED.BLUE  | 蓝色LED灯 |
| LED.GREEN | 绿色LED灯 |

**构造函数**

LED对象的构造函数说明如下：

| 构造函数 | class device.LED (pin_num, led_kind)                         |
| -------- | ------------------------------------------------------------ |
| 功能     | 构造一个LED对象。                                              |
| 参数说明 | pin_num: pin的序号，可以通过machine.Pin.Index('X', y)得到。<br />led_kind是上述led灯类型中的一个。 |
| 返回值   | 返回创建的对应的颜色的led对象。                                |

**实例方法**

device.LED类包括如下实例方法：

| 方法名       | 说明                                     |
| ------------ | ---------------------------------------- |
| LED.init()   | 初始化LED对象。                            |
| LED.open()   | 打开对应的led，开发板中对应的颜色的led亮。 |
| LED.close()  | 关闭对应的led, 开发板中对应的颜色的led灭。 |
| LED.deinit() | 关闭LED对象。                              |

详细说明如下：

| 方法名称 | LED.init()                           |
| -------- | ------------------------------------ |
| 功能     | 初始led。                              |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | LED.open()                           |
| -------- | ------------------------------------ |
| 功能     | 打开led灯。                            |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | LED.close()                          |
| -------- | ------------------------------------ |
| 功能     | 关闭led灯。                            |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | LED.deinit()                         |
| -------- | ------------------------------------ |
| 功能     | 关闭led对象。                          |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |
**示例**
\>>>from device import LED
\>>>led0 = LED(71, LED.RED)
\>>>led1 = LED(72, LED.GREEN)
\>>>led2 = LED(73, LED.BLUE)
\>>>led0.init()
\>>>led0.open()
\>>>led0.close()
\>>>led1.init()
\>>>led1.open()
\>>>led1.close()
\>>>led2.init()
\>>>led2.open()
\>>>led2.close()

#### class device.SIX_AXIS

SIX_AXIS类是device模块下面的SIX_AXIS类，例如万耦开发板上的六轴传感器，对应的型号是icm20602。

**常量**  

| 采样类型       |              |
| -------------- | ------------ |
| SIX_AXIS. GYRO | 三轴陀螺仪   |
| SIX_AXIS. ACCE | 三轴加速度计 |

**构造函数**

​SIX_AXIS对象的构造函数说明如下：

| 构造函数 | class device.SIX_AXIS()        |
| -------- | ------------------------------ |
| 功能     | 构造一个SIX_AXIS对象。           |
| 参数说明 | 无                             |
| 返回值   | 返回创建的对应的六轴传感器对象。 |

**实例方法**

device. SIX_AXIS类包括如下实例方法：

| 方法名            | 说明                   |
| ----------------- | ---------------------- |
| SIX_AXIS.init()   | 初始化SIX_AXIS对象。     |
| SIX_AXIS.read()   | 读取六轴传感器对象的值。 |
| SIX_AXIS.deinit() | 关闭SIX_AXIS传感器。     |

详细说明如下：

| 方法名称 | SIX_AXIS.init()                      |
| -------- | ------------------------------------ |
| 功能     | 初始化SIX_AXIS对象。                         |
| 参数说明 | 无                                   |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

| 方法名称 | SIX_AXIS.read([type])                                        |
| -------- | ------------------------------------------------------------ |
| 功能     | 获取SIX_AXIS的陀螺仪和加速度计的值或者只获取其中之一的值。     |
| 参数说明 | [type]：采样类型，参见前述“常量”，该参数为可选参数。                           |
| 返回值   | 1：当无参数时返回值为陀螺仪和加速度计的字典组合值。<br />2：当参数为SIX_AXIS. GYRO获取的值是陀螺仪的字典值。<br />3：当参数SIX_AXIS. ACCE获取的值是加速度计的字典值。<br />4：失败触发ValueError异常。|

| 方法名称 | SIX_AXIS.deinit()                    |
| -------- | ------------------------------------ |
| 功能     | 关闭SIX_AXIS传感器。                |
| 参数说明 | 无参数                               |
| 返回值   | 成功返回None  失败触发ValueError异常。 |

**示例**
\>>> from device import SIX_AXIS
\>>> axis = SIX_AXIS()
\>>> axis.init()
\>>> axis.read()
{'GYRO': {'Y': 600, 'X': 0, 'Z': 0}, 'ACCE': {'Y': 26, 'X': -26, 'Z': 1014}}
\>>> axis.read(SIX_AXIS.GYRO)
{'Y': 500, 'X': -200, 'Z': 0}
\>>> axis.read(SIX_AXIS.ACCE)
{'Y': 26, 'X': -29, 'Z': 1015}
\>>> axis.deinit()