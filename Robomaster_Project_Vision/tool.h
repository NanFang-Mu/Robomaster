#pragma
#include <opencv2/opencv.hpp>

// 工具函数声明
// 文件夹检测与管理
void folder_management();

// 视频模式下命令行参数设置
void video_command_set(int argc, char **argv, std::string& read_path);

// 图片模式下命令行参数设置
void image_command_set(int argc, char **argv, std::string& read_path);

// 视频标注
void video_mark(cv::VideoCapture& cap);