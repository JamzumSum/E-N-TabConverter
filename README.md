# E-N-TabConverter
## 将图片形式的六线谱转换为MusicXML

由我[原来的项目](https://github.com/EscapeLand/ELand-chordConverter)迁移而来，这次完全由我自己开发:P

>2019-2-4：重写了很多，在我开学之前应该还有一次 Release

## 目录

  * [简介](#背景及原理)
  * [文件列表](#文件列表)

### 背景及原理

  篇幅有点长，移步[wiki](https://github.com/YuanWangZhe/E-N-TabConverter/wiki/原理)

### 文件列表：

#### 头文件

|头文件名      |简介                         |
|:-----------:|-----------------------------|
|Cuckoo.h     |小节内部识别                  |
|global.h     |全局变量声明                  |
|GUI.h        |win32界面库                   |
|Dodo.h       |图像处理函数声明                |
|swan.h       |XML输出模块                   |
|type.h       |类 自定义类型声明              |

#### 源文件

|源文件名      |简介                          |
|:-----------:|------------------------------|
|Cuckoo.cpp   |图像处理类实现                  |
|Dodo.cpp     |图像处理函数实现                |
|eagle.cpp    |机器学习函数实现                |
|framework.cpp|较大规模图像处理函数实现          |
|frmain.cpp   |界面                           |
|func.cpp     |算法实现                       |

#### 杂项

|文件（夹）         |简介                          |
|:---------------:|------------------------------|
|icon             |存放图标                     |
|OpenCV           |OpenCV函数库                 |
|sample_classified|已分类样本                 |
|sample_picture   |测试用图                      |
|tData.csv        |机器学习训练数据                |
|tinyxml2         |tinyxml2库                     |

### 引用

  * [OpenCV](https://github.com/opencv/opencv)
  * [tinyxml2](https://github.com/leethomason/tinyxml2)

Thanks sincerely.
