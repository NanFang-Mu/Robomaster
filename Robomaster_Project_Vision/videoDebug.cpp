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
    std::cout << "Debug模式已启用，正在处理视频..." << std::endl;
    video_mark(cap, 1);

    // 资源释放
    cap.release();    
    cv::destroyAllWindows();

    // 处理结束
    std::cout << "视频处理完毕，结果已保存至./output/result_video.avi" << std::endl;
    return 0;
}