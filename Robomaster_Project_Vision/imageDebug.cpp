#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <iostream>

#include <opencv2/opencv.hpp>

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
    std::cout << "Debug模式已启用，正在处理图片..." << std::endl;
    imageMark(image, 1);

    // 对图片对象进行保存
    if (!(cv::imwrite("./output/image_result.png", image)))
    {
        std::cerr << "图片保存失败，请检查输出路径 ./output/image_result.png 是否正确" << std::endl;
        return -1;
    }

    // 处理结束
    std::cout << "图片处理完毕，结果已保存至./output/image_result.png" << std::endl;
    return 0;
}