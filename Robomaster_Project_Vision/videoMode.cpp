/*************************************************************************************
 该程序的输入对象：
 一张.png格式的，分辨率为1280x1024的，包含识别目标（装甲板和灯条）的三通道彩图

 默认输入路径：
 ./images/outpost_frame.png

 命令行参数输入路径：
 ./你的main.exe所在目录/main.exe ./你的图片输入路径

 该程序的输出对象：
 最终装甲板结果图：armor_result.png
 灯条候选图：light_candidates.png
 实际目标颜色mask：target_mask.png

 默认输出路径：
 ./output/armor_result.png
 ./output/light_candidates.png
 ./output/target_mask.png
**************************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>

#include "function.h"
#include "tool.h"

int main(int argc, char **argv)
{
    // 输入输出编码统一,使用utf-8编码
    #ifdef _WIN32    
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    #endif

    // 文件夹检测与管理
    folder_management();

    // 命令行参数设置
    std::string read_path;
    video_command_set(argc, argv, read_path);

    // 视频读取为对象
    cv::VideoCapture cap(read_path);
    if (!cap.isOpened())
    {
        std::cerr << "视频文件读取失败，请检查路径 " << read_path << " 是否正确！" << std::endl;
        return -1;
    }
    
    // 对视频对象进行配置获取，标注和保存
    video_mark(cap);

    // 资源释放
    cap.release();    
    cv::destroyAllWindows();

    // 处理结束
    std::cout << "视频处理完毕，结果已保存至./output/result_video.avi" << std::endl;
    return 0;
}