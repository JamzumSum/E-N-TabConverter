# E-N-TabConverter
## 将图片形式的六线谱转换为MusicXML

由我[原来的项目](https://github.com/EscapeLand/ELand-chordConverter)迁移而来，这次完全由我自己开发:P

>我希望能实现识别任何一张照片中的吉他谱<br>
但现在连机器生成的正置图都费劲hhhh<br>
至于这个项目到底该何去何从，我也不知道。。我只是单纯想把它做下去而已2333

### 文件列表：

#### 头文件

|头文件名      |简介                         |
|:-----------:|-----------------------------|
|Cuckoo.h     |小节内部识别                  |
|global.h     |全局变量声明                  |
|GUI.h        |win32界面库                   |
|myheader.h   |函数声明汇总                  |
|swan.h       |xml输出模块                   |
|type.h       |类 自定义类型声明              |

#### 源文件

|源文件名      |简介                          |
|:-----------:|------------------------------|
|Dodo.cpp     |图像处理函数汇集                |
|eagle.cpp    |机器学习函数汇集                |
|framework.cpp|较大规模图像处理模块            |
|frmain.cpp   |界面                           |
|maincpp.cpp  |算法实现                       |

#### 杂项

|文件（夹）    |简介                          |
|:-----------:|------------------------------|
|icon         |存放图标                       |
|OpenCV       |OpenCV函数库                   |
|sample_classified|已分类样本                 |
|sample_picture|测试用图                      |
|tData.csv    |机器学习训练数据                |
|tinyxml2     |tinyxml2库                     |

### 引用

  * [OpenCV](https://github.com/opencv/opencv)
  * [tinyxml2](https://github.com/leethomason/tinyxml2)

Thanks sincerely.
