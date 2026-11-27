# Robomaster 视觉组任务仓库

## 简短说明
- 1.`docs/`还在写，里面有一些未完成的旧内容，请忽略
- 2.算法分析部分正在加工，后续会更新
- 3.文件保存防御工程的控制台输出内容可能有误，尽管不影响主要功能，但后续也会更新
- 4.请先按照CMake文件进行编译，后面会补充相关指令与“快速开始”
- 5.一些不宜Git管理的大型文件和不同模式下产生的文件将存放于Releases中，包含了用于输入输出的图片、视频文件和ROI可疑裁剪图集、CSV逐帧数据文件等

## 更新内容
- 1.重构了大部分代码，整理了文件和代码结构，优化了一些函数接口
- 2.重写了CMake构建文件，编译将产生四个可执行文件，分别对应”图片模式““图片调试模式”“视频模式”“视频调试模式”，差异之处见目录结构说明
- 3.新增了逐帧数据文件的CSV记录
- 4.新增了“可疑重投影“的处理和ROI裁剪记录

## 目录结构说明
- `Robomaster_Project_Vision/`：项目总文件夹
	- `docs/`：文档（尚未完成，正在完善）
		- `Issue_Record.txt`：问题记录
		- `Operating_Instructions.txt`：运行说明
		- `Process_Record.txt`：过程记录
	- `videos/`：输入的视频文件，见Releases
	- `images/`：输入的图片文件，见Releases
	- `output/`：程序运行生成的结果，见Releases
		- `armor_suspicious/`：视频调试模式下保存的可疑ROI裁剪记录
		- `frame_data.csv`：逐帧数据文件
		- `video_result.avi`：视频处理结果
		- `image_result.png`：图片处理结果
	- `function.h`：头文件，负责声明单帧图像处理所需的函数、结构体、全局变量等
	- `function.cpp`：源代码，负责实现单帧图像处理、标注等核心功能
	- `tool.h`：头文件，负责声明用于辅助的工具函数
	- `tool.cpp`：源代码，负责实现用于辅助的工具函数功能
	- `config.h`：头文件，负责声明和存放固定的参数配置等
	- `videoMode.cpp`：源代码，负责执行视频处理的主流程
	- `videoDebug.cpp`：源代码，相比视频处理主流程增加了重投影标注和可疑ROI裁剪记录
	- `imageMode.cpp`：源代码，负责执行单张图片处理的主流程
	- `imageDebug.cpp`：源代码，相比单证图片处理主流程增加了重投影标注    
	- `CMakeLists.txt`：CMake构建文件