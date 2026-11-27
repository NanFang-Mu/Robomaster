#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>

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

    // 清空./output文件夹中的所有文件夹和文件
    for (const auto& entry : std::filesystem::directory_iterator("./output"))
    {
        std::filesystem::remove_all(entry.path());
    }
    
    // 检测文件夹./output/armor_suspicious/是否存在，不存在则创建
    if (!std::filesystem::exists("./output/armor_suspicious"))
    {
        std::filesystem::create_directory("./output/armor_suspicious");
        std::cout << "创建文件夹./output/armor_suspicious" << std::endl;
    }

    // 在./output/下面检测frame_data.csv是否存在，不存在则创建
    if (!std::filesystem::exists("./output/frame_data.csv"))
    {        
        std::ofstream file("./output/frame_data.csv");
        file << "frame_id,time_s,detect_valid,candidate_id,pnp_valid,tx_m,ty_m,tz_m,distance_m,rx_rad,ry_rad,rz_rad,reprojection_error_px,is_suspicious\n";
        file.close();
        std::cout << "创建文件./output/frame_data.csv" << std::endl;
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
void video_mark(cv::VideoCapture& cap, const int mode)
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
        // 防御性检查：确保读到的不是空帧
        if (frame.empty())
        {
            std::cerr << "第 " << frame_id << " 帧读取为空" << std::endl;
            break;
        }

        // 计算相对时间
        double time_s = frame_id / fps;

        // 全局临时变量设置帧号和时间点
        ::frame_id = frame_id;
        ::time_s = time_s;

        // 对单帧进行标注
        imageMark(frame, mode);

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
        else
        {
            std::ofstream file("./output/frame_data.csv", std::ios::app);
            if (::armor_listTemp.empty())
            {   
                // 无候选装甲板时，仅保留帧号、时间和无效标记
                file << ::frame_id << ","
                     << ::time_s << ","
                     << 0 << ","                                          
                     << ",,,,,,,,,,\n";
            }
            else
            {
                // 有候选装甲板时
                for (const Armor &armor : ::armor_listTemp)
                {                   
                    file << armor.frame_id << ","
                         << armor.time_s << ","
                         << armor.is_valid << ","               
                         << armor.armor_id << ","
                         << armor.is_solvePnP << ",";                         
                        if (armor.is_solvePnP) // 位姿解算成功时，写入位姿数据
                        {
                            file << armor.tx_m << ","
                                 << armor.ty_m << ","
                                 << armor.tz_m << ","
                                 << armor.distance_m << ","
                                 << armor.rx_rad << ","
                                 << armor.ry_rad << ","
                                 << armor.rz_rad << ","
                                 << armor.reprojection_error_px << ","
                                 << armor.is_suspicious << "\n";
                        }                                             
                        else // 位姿解算失败时，位姿数据留空
                        {
                            file << ",,,,,,,,\n";
                        }                         
                }
            }
            file.close();
        }

        // 进度可视化
        if (frame_id == total_frames * 0.2)
        {
            std::cout << "视频处理进度：20%" << std::endl;
        }
        if (frame_id == total_frames * 0.4)
        {
            std::cout << "视频处理进度：40%" << std::endl;
        }
        if (frame_id == total_frames * 0.6)
        {
            std::cout << "视频处理进度：60%" << std::endl;
        }
        if (frame_id == total_frames * 0.8)
        {
            std::cout << "视频处理进度：80%" << std::endl;
        }

        // 全局临时装甲板数据清空
        ::armor_listTemp.clear();

        // 帧编号自加
        frame_id++;
    }
    std::cout << "视频处理进度：100%" << std::endl;

    // 资源释放
    writer.release();
}