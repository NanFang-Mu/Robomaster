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
#include <opencv2/calib3d.hpp>

// 单帧接口函数
std::vector<Armor> detectArmor(const cv::Mat &image)
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

    // // 尝试保存图像并检测图像能否正常保存，如果不能则提示，如果可以也提示
    // if (cv::imwrite("./output/armor_result.png", armor_result))
    // {
    //     std::cout << "图像armor_result.png保存成功！" << std::endl;
    // }
    // else
    // {
    //     // 保存失败的提示
    //     std::cout << "图像armor_result.png保存失败！" << std::endl;
    // }
    // if (cv::imwrite("./output/light_candidates.png", light_candidates))
    // {
    //     std::cout << "图像light_candidates.png保存成功！" << std::endl;
    // }
    // else
    // {
    //     std::cout << "图像light_candidates.png保存失败！" << std::endl;
    // }
    // if (cv::imwrite("./output/target_mask.png", target_mask))
    // {
    //     std::cout << "图像target_mask.png保存成功！" << std::endl;
    // }
    // else
    // {
    //     std::cout << "图像target_mask.png保存失败！" << std::endl;
    // }
    // std::cout << std::endl;

    return armor_list;
}

int main(int argc, char **argv)
{
    // 输入输出编码统一,使用utf-8编码
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    // 设置默认路径
    std::string video_path = "./videos/outpost_video.avi";

    // 设置命令行参数路径
    if (argc > 1)
    {
        video_path = argv[1];
        std::cout << "使用命令行参数路径读取输入视频: " << video_path << std::endl;
    }
    else
    {
        std::cout << "使用默认路径读取输入视频: " << video_path << std::endl;
    }

    // 检测文件夹./videos和./output是否存在，不存在则创建
    if (!std::filesystem::exists("./videos"))
    {
        std::filesystem::create_directory("./videos");
        std::cout << "创建文件夹./videos" << std::endl;
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

    // 打开视频
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened())
    {
        std::cerr << "错误: 无法打开视频文件 " << video_path << "\n";
        return -1;
    }

    // 获取视频fps，若失败则回退至默认值120
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps <= 0 || std::isnan(fps))
    {
        std::cerr << "警告：无法读取有效FPS，默认使用120\n";
        fps = 120;
    }

    // 获取视频宽度，若失败则回退至默认值1280
    int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    if (width <= 0)
    {
        std::cerr << "警告：无法读取有效宽度，默认使用1280\n";
        width = 1280;
    }

    // 获取视频高度，若失败则回退至默认值1024
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    if (height <= 0)
    {
        std::cerr << "警告：无法读取有效高度，默认使用1024\n";
        height = 1024;
    }

    // 获取视频总帧数，若失败则回退至默认值1450
    int total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    if (total_frames <= 0)
    {
        std::cerr << "警告：无法读取有效帧数，默认使用1450\n";
        total_frames = 1450;
    }

    std::cout << "视频信息: " << std::endl
              << "宽度：" << width << "px" << std::endl
              << "高度：" << height << "px" << std::endl
              << "帧率：" << fps << "fps" << std::endl
              << "总帧数：" << total_frames << std::endl
              << std::endl;

    // 准备写入器用于保存结果视频
    cv::VideoWriter writer("./output/video_result.avi",
                           cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                           fps, cv::Size(width, height));
    if (!writer.isOpened())
    {
        std::cerr << "错误: 无法创建输出视频文件！请检查output文件夹是否存在。\n";
        return -1;
    }

    // 帧号分配
    cv::Mat frame;
    int frame_id = 0;

    // 逐帧读取循环
    while (cap.read(frame))
    {
        // 防御性检查：确保读到的不是空帧
        if (frame.empty())
        {
            std::cerr << "警告: 读取到空帧，提前结束。\n";
            break;
        }

        // 调用单帧接口函数,声明有效装甲板容器
        std::vector<Armor> armor_list = detectArmor(frame);

        // 计算相对时间
        double time_s = frame_id / fps;

        // 在画面左上角绘制当前帧信息
        cv::putText(frame, "帧号：" + std::to_string(frame_id), cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 2);
        cv::putText(frame, "时间：" + std::to_string(time_s) + "秒", cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 2);

        // 输出有效装甲板信息
        // 如果armor_list为空，则提示没有找到装甲板
        if (armor_list.empty())
        {
            std::cout << "未找到有效装甲板！" << std::endl;
            cv::putText(frame, "No Armor_valid", cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX,
                        0.5, cv::Scalar(255, 255, 0), 2);
        }
        else
        {
            for (const Armor &armor : armor_list)
            {
                // 在画面中绘制当前帧中绘制装甲板的编号，颜色，中心点，四角点
                for (int k = 0; k < 4; k++)
                {
                    cv::line(frame, armor.vertices[k], armor.vertices[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }

                // 在图像上绘制装甲板中心点
                cv::circle(frame, armor.center, 5, cv::Scalar(0, 255, 0), -1);

                // 在frame上绘制装甲板的编号
                cv::putText(frame, std::to_string(armor.id), armor.center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);

                // 在frame上绘制装甲板的四角点编号
                for (int v = 0; v < 4; v++)
                {
                    cv::circle(frame, armor.vertices[v], 5, cv::Scalar(0, 255, 0), -1);
                    std::string text = std::to_string(armor.id) + "." + std::to_string(v);
                    cv::putText(frame, text, armor.vertices[v], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
                }

                cv::Vec3d rvec;
                cv::Vec3d tvec; // 保留tvec和rvec
                double distance; // 保留distance，即armor中心到相机光心的距离
                double tz; // 保留相机z轴方向深度深度

                // 在frame上标注PnP是否成功
                if (pose_is_solved(armor.vertices, rvec, tvec, distance, tz))
                {
                    cv::putText(frame, "PnP Success", cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX,
                                0.5, cv::Scalar(255, 255, 0), 2);
                }
                else
                {
                    // 标注PnP失败
                    cv::putText(frame, "PnP Failed", cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX,
                                0.5, cv::Scalar(255, 255, 0), 2);                                
                }
            }           

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

            // // 展示当前帧
            // cv::imshow("装甲板识别", frame);
            // cv::waitKey(0); // 等待
            // cv::destroyWindow("装甲板识别");

        }
        std::cout << std::endl;

        // 把当前帧处理结果写入视频文件
        writer.write(frame);

        // 等待键盘输入，1ms延迟，如果是ESC键(27)则退出
        int key = cv::waitKey(1);
        if (key == 27)
        {
            std::cout << "用户按 ESC 键退出。\n";
            break;
        }

        // 帧编号自加
        frame_id++;
    }

    // 资源释放
    cap.release();
    writer.release();
    cv::destroyAllWindows();

    std::cout << "视频处理完毕，结果已保存至./output/result_video.avi" << std::endl;

    return 0;
}

