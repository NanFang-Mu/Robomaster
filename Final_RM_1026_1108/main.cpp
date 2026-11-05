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

#include <opencv2/opencv.hpp>
#include "function.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <windows.h>

// 单帧接口函数
std::vector<Armor> detectArmor(const cv::Mat& image)
{
    // 变量声明
    cv::Mat target_mask, light_candidates, armor_result; // 新矩阵       
    std::vector<Armor> armor_list;                       // 声明有效装甲板容器
    bool colorFlag;                                      // 颜色标志位

    colorFlag = true; // 红色灯条标志位
    // 识别红色灯条的代码
    // 用输入./images/outpost_frame.png图的克隆来初始化四个新矩阵
    target_mask = image.clone();
    light_candidates = image.clone();
    armor_result = image.clone();

    // 调用功能函数
    armor_list = detectArmor(target_mask, light_candidates, armor_result, colorFlag);

    // 尝试保存图像并检测图像能否正常保存，如果不能则提示，如果可以也提示
    if (cv::imwrite("./output/armor_result.png", armor_result))
    {
        std::cout << "图像armor_result.png保存成功！" << std::endl;
    }
    else
    {
        // 保存失败的提示
        std::cout << "图像armor_result.png保存失败！" << std::endl;
    }
    if (cv::imwrite("./output/light_candidates.png", light_candidates))
    {
        std::cout << "图像light_candidates.png保存成功！" << std::endl;
    }
    else
    {
        std::cout << "图像light_candidates.png保存失败！" << std::endl;
    }
    if (cv::imwrite("./output/target_mask.png", target_mask))
    {
        std::cout << "图像target_mask.png保存成功！" << std::endl;
    }
    else
    {
        std::cout << "图像target_mask.png保存失败！" << std::endl;
    }
    std::cout << std::endl;

    return armor_list;
}


int main(int argc, char** argv)
{
    // 输入输出编码统一,使用utf-8编码
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    // 设置默认路径
    std::string image_path =  "./images/outpost_frame.png";
    
    // 设置命令行参数路径
    if (argc > 1)
    {
        image_path = argv[1];
        std::cout << "使用命令行参数路径读取输入图: " << image_path << std::endl;        
    }
    else
    {
        std::cout << "使用默认路径读取输入图: " << image_path << std::endl;        
    }
    
    // 检测文件夹./images和./output是否存在，不存在则创建
    if (!std::filesystem::exists("./images"))
    {
        std::filesystem::create_directory("./images");
        std::cout << "创建文件夹./images" << std::endl;
    }
    if (!std::filesystem::exists("./output"))
    {
        std::filesystem::create_directory("./output");
        std::cout << "创建文件夹./output" << std::endl;
    }

    // 清空./output文件夹中的所有文件
    for (const auto &entry : std::filesystem::directory_iterator("./output"))
    {
        std::filesystem::remove(entry.path());
        std::cout << "正在清空./output文件夹中的所有文件" << std::endl;
    }
    std::cout << std::endl;

    //  检测./images/outpost_frame.png能否正常读取，不能读取则提示，能读取也提示，并输出图片的宽度，高度，通道数
    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);   //以IMREAD_COLOR形式读取
    if (image.empty())
    {
        std::cout << "无法读取" << image_path << "，程序自动结束" << std::endl;
        std::cout << std::endl;
        return -1;
    }
    else
    {
        std::cout << "成功读取" << image_path  << std::endl;
        std::cout << "图片宽度: " << image.cols << std::endl;
        std::cout << "图片高度: " << image.rows << std::endl;
        std::cout << "通道数: " << image.channels() << std::endl;
        std::cout << std::endl;
    }

    // 对image进行缩放，1280*1024
    cv::resize(image, image, cv::Size(1280, 1024), cv::INTER_LINEAR);

    // 声明有效装甲板容器
    std::vector<Armor> armor_list;
    
    // 调用单帧接口函数
    armor_list = detectArmor(image);

    // 输出有效装甲板信息
    // 如果armor_list为空，则提示没有找到装甲板
    if (armor_list.empty())
    {
        std::cout << "未找到有效装甲板！" << std::endl;
    }
    else
    {
        std::cout << "-------------------有效装甲板信息-------------------" << std::endl;
        for (const auto &armor : armor_list)
        {
            std::cout << "编号：" << armor.id << std::endl;
            std::cout << "颜色：" << (armor.colorFlag == true ? "红色" : "蓝色") << std::endl;
            std::cout << "中心点：" << armor.center << std::endl;
            std::cout << "四角点：";
            for (const auto &point : armor.vertices)
            {
                std::cout << "(" << point.x << ", " << point.y << ") ";
            }
            std::cout << std::endl;
            std::cout << "是否有效：" << (armor.isValid ? "是" : "否") << std::endl;
            std::cout << "----------------------------------------------------" << std::endl;
        }
    }
    std::cout << std::endl;

    return 0;
}