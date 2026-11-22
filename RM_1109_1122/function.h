#pragma
#include <opencv2/opencv.hpp>
#include <string>

// 结构体：灯条，包含颜色标志，最小旋转外接矩形，中心点，长度，宽度，角度，轮廓面积，编号
struct LightBar
{
    bool colorFlag; // 颜色标志，true表示红色，false表示蓝色
    std::vector<cv::Point> contour; // 轮廓
    cv::RotatedRect minRect; // 最小
    cv::Point2f center; // 中心点
    double length; // 长度
    double width; // 宽度
    double angle; // 角度
    double area; // 轮廓面积
    int id; // 编号
};

// 结构体：装甲板，包含颜色标志，最小旋转外接矩形，中心点，四角点，长度，宽度，角度，编号，是否有效
struct Armor
{   
    int id; // 编号
    bool colorFlag; // 颜色标志，true表示红色，false表示蓝色    
    cv::Point2f center; // 中心点，单精度
    std::vector<cv::Point2f> vertices = std::vector<cv::Point2f>(4); // 四角点集合，单精度
    bool isValid; // 是否有效
};

// 函数声明

// 声明函数：返回装甲板容器，参数为3个可修改的矩阵
std::vector<Armor> detectArmor(cv::Mat &target_mask, cv::Mat &lightbar_candidate, cv::Mat &armor_result, bool colorFlag);

// 声明函数：单帧接口函数
std::vector<Armor> detectArmor(const cv::Mat& image);

// 计算位姿是否有效，函数
bool pose_is_solved(const std::vector<cv::Point2f> &armor_image_vertices, // 像素坐标系下的装甲板四角点坐标（已经进行过点序规范）
                   cv::Vec3d &rvec, cv::Vec3d &tvec, // 保留tvec和rvec
                   double &distance, // 保留distance，即armor中心到相机光心的距离
                   double &tz); // 保留相机z轴方向深度深度