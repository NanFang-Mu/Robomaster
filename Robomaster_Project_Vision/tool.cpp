#include <filesystem>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "function.h"

// 文件夹检测与管理
void folder_management()
{
    // 检测文件夹./image和./videos和./output是否存在，不存在则创建
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
    if (!std::filesystem::exists("./images"))
    {
        std::filesystem::create_directory("./images");
        std::cout << "创建文件夹./images" << std::endl;
    }

    // 清空./output文件夹中的所有文件
    for (const auto &entry : std::filesystem::directory_iterator("./output"))
    {
        std::filesystem::remove(entry.path());
        std::cout << "正在清空./output文件夹中的所有文件" << std::endl;
    }    
}

// 视频模式下命令行参数设置
void video_command_set(int argc, char **argv, std::string& read_path)
{
    // 设置命令行参数路径
    if (argc == 1)
    {
        std::cout << "使用默认路径读取视频文件 ./videos/outpost_video.avi" << std::endl;
        read_path = "./videos/outpost_video.avi";
    }
    else
    {
        std::cout << "使用自定义路径读取视频文件 "  << *argv << std::endl;
        read_path = argv[1];    
    }
}

// 图片模式下命令行参数设置
void image_command_set(int argc, char **argv, std::string& read_path)
{
    // 设置命令行参数路径
    if (argc == 1)
    {
        std::cout << "使用默认路径读取图片文件 ./images/outpost_frame.png" << std::endl;
        read_path = "./images/outpost_frame.png";
    }
    else
    {
        std::cout << "使用自定义路径读取图片文件 "  << *argv << std::endl;
        read_path = argv[1];    
    }
}

// 视频标注
void video_mark(cv::VideoCapture& cap)
{   
    // 获取视频文件的配置    
    double fps;
    int width, height, total_frames;   

    // 获取视频fps，若失败则回退至默认值120
    fps = cap.get(cv::CAP_PROP_FPS);
    if (fps <= 0 || std::isnan(fps))
    {
        std::cerr << "无法读取有效FPS，默认使用120\n";
        fps = 120;
    }

    // 获取视频宽度，若失败则回退至默认值1280
    width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    if (width <= 0)
    {
        std::cerr << "无法读取有效宽度，默认使用1280\n";
        width = 1280;
    }

    // 获取视频高度，若失败则回退至默认值1024
    height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    if (height <= 0)
    {
        std::cerr << "无法读取有效高度，默认使用1024\n";
        height = 1024;
    }

    // 获取视频总帧数，若失败则回退至默认值1450
    total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    if (total_frames <= 0)
    {
        std::cerr << "无法读取有效帧数，默认使用1450\n";
        total_frames = 1450;
    }

    // 输出信息
    std::cout << "视频信息: " << std::endl
              << "宽度：" << width << " px" << std::endl
              << "高度：" << height << " px" << std::endl
              << "帧率：" << fps << " fps" << std::endl
              << "总帧数：" << total_frames << std::endl;              

    // 帧号分配和变量声明
    cv::Mat frame;
    int frame_id = 0;

    // 准备写入器用于保存结果视频
    cv::VideoWriter writer("./output/video_result.avi",
                           cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                           fps, cv::Size(width, height));
    if (!writer.isOpened())
    {
        std::cerr << "无法创建输出视频文件，请检查output文件夹是否存在\n";
        return;
    }

    // 逐帧读取循环
    while (cap.read(frame))
    {   
        // 进度，待完善
        std::cout << "#";
        
        // 防御性检查：确保读到的不是空帧
        if (frame.empty())
        {
            std::cerr << "第 " << frame_id << " 帧读取为空" << std::endl;
            break;
        }

        // 计算相对时间
        double time_s = frame_id / fps;

        // 对单帧进行标注
        imageMark(frame);

        // 在画面左上角绘制当前帧信息
        int baseline1 = 0;
        int baseline2 = 0;
        cv::Size textSize1 = cv::getTextSize("帧号：" + std::to_string(frame_id), cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline1);
        cv::Size textSize2 = cv::getTextSize("时间：" + std::to_string(time_s) + "秒", cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline2);
        cv::rectangle(frame, cv::Point(10, 20 - textSize1.height), cv::Point(10 + textSize1.width, 20), cv::Scalar(0, 0, 0), cv::FILLED);
        cv::rectangle(frame, cv::Point(10, 40 - textSize2.height), cv::Point(10 + textSize2.width, 40), cv::Scalar(0, 0, 0), cv::FILLED);
        cv::putText(frame, "帧号：" + std::to_string(frame_id), cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 2);
        cv::putText(frame, "时间：" + std::to_string(time_s) + "秒", cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 2);

        // 把当前帧处理结果写入视频文件
        if (!(writer.write(frame)))
        {
            std::cerr << "第 " << frame_id << " 帧未能写入" << std::endl;
        }

        // 帧编号自加
        frame_id++;
    }

    // 资源释放
    writer.release();
}