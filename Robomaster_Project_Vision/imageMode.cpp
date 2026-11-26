#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <opencv2/opencv.hpp>
#include <iostream>

#include "tool.h"
#include "function.h"

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
    image_command_set(argc, argv, read_path);

    // 图片读取为对象
    cv::Mat image = cv::imread(read_path, cv::IMREAD_COLOR);
    if (image.empty())
    {
        // 图片读取失败给出报错提示
        std::cerr << "图片文件读取失败，请检查路径 " << read_path << " 是否正确！" << std::endl;
        return -1;
    }  

    // 对图片对象进行标注
    imageMark(image);

    // 对图片对象进行保存
    cv::imwrite("./output/outpost_frame.png", image);

     // 处理结束
    std::cout << "图片处理完毕，结果已保存至./output/outpost_frame.png" << std::endl;
    return 0;
}