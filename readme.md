# axi_spi_tf-card
## 0. 前言
这个项目是一个axi4的ip，cpu核（zynq）能够通过axi4总线控制这个ip，发出spi总线的控制信号，控制信号通过GPIO/PMOD上的管脚连接到tf卡/microsd卡的卡托，从而能够读取tf卡中的数据。

这个项目是个学习项目，没啥实用性，只支持下述基本的指令：
- CMD0:重置卡片进入`idle`mode
- CMD8:设置卡片电压模式
- CMD55:下一个调用的命令是ACMD指令
- CMD41:激活tf卡，从`idle`模式进入`activate`模式
- CMD17:读取一个block

也就是说，这个ip唯二的作用：初始化卡片，读取任一block的数据。

这个项目中的一些细节参考了[这个项目](https://github.com/WangXuan95/FPGA-SDcard-Reader)。但他是sdio的，和spi模式还有一些不一样，不能直接用。我索性自己写了一个。

**我用的vivado版本是22.2**

## 1. 各文件简介
- block_design/[design_1.pdf](/block_design/design_1.pdf) 是vivado中blockdesign的截图，描述了这个ip如何与zynq、ila、外界的tf卡引脚连接。
- doc/[Part1PhysicalLayerSimplifiedSpecificationVer9.10Fin_20231201.pdf](doc/Part1PhysicalLayerSimplifiedSpecificationVer9.10Fin_20231201.pdf)是tf卡的文档，其中第七章讲述的是spi模式。
- ip/*.v是ip的rtl代码。
- vitis/[helloworld.c](/vitis/helloworld.c)是vitis的zynq核代码，作用是操作内存地址，控制ip。

## 2. 如何运行
### 2.1 硬件上
最好的情况是fpga上有一个pl端的tf卡卡槽。但这个可能性不大。我用到过的板卡都没有pl端的tf卡卡槽。因此需要购买sd模块，这个几块钱就行。我买的sd模块是下图这样的，两三块包邮。别忘了买杜邦线。
![1.jpg](/mdpic/1.jpg)
**注意：** 购买sd卡模块的时候有两个注意事项：
- 支持的sd卡种类和容量。我买的这款支持的sdhc卡容量小于32G。和tf卡配合使用的时候要注意容量。
- VCC的供电电压范围。贵的sd卡模块会支持3.3v和5v的供电。但我买的sd卡模块只支持5v的供电，且通过内部的电压转换芯片转化成3.3v的作为内部供电。直接给3.3v可能会出错。买的时候要注意，这些引脚的电压都要适配fpga的gpio的电压。

不过如果真的买错了不要着急，也是有办法的。我就买错了，我的fpga是ZCU104，他的PMOD引脚都是3.3v的。这个时候就需要查阅板卡文档。一般情况下，FPGA的电源管理模块都会输出各种各样的VDD电压，找到相应的电压后飞个线出来就可以解决了。  
![2.jpg](/mdpic/2.jpg)
上图就是ZCU104的板卡文档，再这张图的中间偏右位置，可以发现5v信号`UTIL_5V0`被接到了一个洞`J171`上。只要找个杜邦线焊上去就行了（由于我的焊丝和电烙铁年龄都比我还大，化不开，挂不上，导致最终的成品和痔疮一样，敬请见谅）这样就可以将FPGA内部的电压信号飞线飞出来了。下图分别是正面和反面。

![3.jpg](/mdpic/3.jpg)
![4.jpg](/mdpic/4.jpg)

### 2.2 vivado上
vivado上没啥好说的，参考[design_1.pdf](/block_design/design_1.pdf)，连接好各个模块，设置`SCK` `MISO` `MOSI` `CS` 各个模块的引脚就行。注意设置ip的axi总线地址。我设置的是`0xa0000000`.

关于地址分配：axi模块中的地址offset从0x0-0x3FF
其中，0x0-0x1FC是在CMD17的时候存储一个block的数据用的
0x200存储的是命令参数，32bit
0x204存储的是命令编号，6bit
0x208存储的是开关，这个开关一按下，就发送上述的指令，同时开关自动复位。


### 2.3 vitis上
有些版本的vivado中，自制的axi4ip放到vitis中`Platform Project`很可能会报一些奇怪的错误。请参考[这个文档](https://support.xilinx.com/s/question/0D52E00006hpOx5SAE/drivers-and-makefiles-problems-in-vitis-20202?language=en_US),也就是将`Platform Project`中的三个`Makefile`中的
```makefile
INCLUDEFILES=*.h
LIBSOURCES=*.c
OUTS = *.o
```
改成
```makefile
INCLUDEFILES=$(wildcard *.h)
LIBSOURCES=$(wildcard *.c)
OUTS=$(wildcard *.o)
```
就行。一般情况下，`Application Project`可能也会出奇怪的bug。这个看报错信息就行，在对应路径新增文件。
```
Error intializing SD boot data : Software platform XML error, sdx:qemuArguments value "**/qemu/pmu_args.txt" path does not exist E:/********/sw/**/qemu/pmu_args.txt, platform path E:/********/, sdx:configuration **, sdx:image standard
```

### 2.4 最终vitis程序运行
vitis程序怎么用直接看代码就行。之前要修改一下`helloworld.c`中的`SPI_BASE_ADDR`,将他改成之前在vivado中设计的ip地址。正确运行的界面如下
![1722514387572](/mdpic/5.jpg)  
每一轮给tf卡命令需要输入两个数字，一个是cmd的命令号（十进制），一个是cmd的参数（十六进制）。我暂时还没有让sd卡的返回信号通过vitis显示。想看每一轮sd卡怎么返回的就通过ila在vivado中抓波形就好。过两天我就改一下代码，让寄存器存储tf卡给的回应，然后通过vitis输出。

由于这是个学习项目，因此我们需要在串口中手动实现tf卡的初始化。步骤如下
1. cmd:0 arg:0   重置tf卡，让他处于idle状态。这个时候如果抓波形，可以看到`MISO`给出的回应类型是R1类型，每一个bit的具体含义如下图
![7.jpg](/mdpic/7.jpg)
正常响应是`8'b00000001`,表示tf卡正在idle状态。
2. cmd:8 arg:1aa  设置工作电压类型。参数1aa中，1表示电压类型是2.7v-3.6v，aa可以当成是一个防伪flag，可以改成任何一个8bit数字。最后给的40bit回应会原模原样的把这个防伪flag显示出来。具体的含义参考[tf卡文档](/doc/Part1PhysicalLayerSimplifiedSpecificationVer9.10Fin_20231201.pdf)中的下面这张表。
![6.jpg](/mdpic/6.jpg).
正确的回应中，R1和第一步的回应一样都是`8'b00000001`,表示依旧在`idle`mode，`[31:12]`位不重要，`[11:8]`正常值应该是1，表示参数1aa中的电压类型是可以接受的。`echo-back`就是我之前说的防伪flag，应该返回aa。
3. cmd:55 arg:0 这句命令是一个修饰命令，表示下一条命令不是普通的cmd，是acmd。回应也是R1类型的回应，由于依旧是idle模式，所以回应值依旧是`8'b00000001`
4. cmd:41 arg:40000000 由于第三步是CMD55，所以这一步并不是CMD41，而是ACMD41.参数也是确定工作电压的。这一个命令的响应也是一个R1类型的响应。
5. 上一步的响应有两种情况。如果tf卡返回了`0x00000001`,说明初始化仍未完成，所以我们还需要不断地发送ACMD41，也就是重复34两部步骤。一直到这个返回值变成了`0x00000000`，说明tf卡进入了activate模式，初始化完成
6. cmd:17 arg:800 这句话的意思是读取tf卡的第一个block。当然，前提是第五部中，tf卡已经进入了activate模式。这个800具体是多少因人而异。需要下载一个[winhex](http://www.winhex.com/winhex/),这个软件的作用是查看tf卡各个扇区的信息。注意看下图
![8.jpg](/mdpic/8.jpg)
我当前选中的是tf卡第一个扇区。最右边那一栏中，显示逻辑扇区号是0，物理扇区号是2048，也就是0x800.也就是说，tf卡的扇区物理编号是从0x800开始的。我们要读tf卡的第一个扇区，就应该读800号扇区。输入vitis之后，对应vitis的返回值和winhex中的第一个扇区的数据，发现是一样的。

## 3. 一些学习笔记
这一章记录了我写verilog代码/vitis代码时遇到的一些问题。
- crc7的计算：这部分可以参考`/ip/spi2tf1KB_v1_0_S00_AXI.v`中的函数`CalcCrc7`。这个函数我是参考[这个项目](https://github.com/WangXuan95/FPGA-SDcard-Reader)的。
- 关于时钟频率：我查阅的文档上都说初始化的时候频率不要高于400k，但经过我的实验，25M的SCK也是可以正常运行的。可以看到我的代码里有个四分频的构造。这是为了抓取ila波形。之后纯用vitis调试了就会去掉这个四分频。


