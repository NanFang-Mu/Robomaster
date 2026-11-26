#pragma
#include <opencv2/opencv.hpp>

// 核心对象说明
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
    bool is_PnP; //PnP是否结算成功
};

// 核心功能函数声明
// 图片标注（单帧接口函数）
void imageMark(cv::Mat& image);

// 从图片中检测装甲板
std::vector<Armor> detectArmor(cv::Mat& target_mask);                             

// 解算位姿是否有效，解算数据计算
bool pose_is_solved(const std::vector<cv::Point2f> &armor_image_vertices, // 像素坐标系下的装甲板四角点坐标（已经进行过点序规范）
                    cv::Vec3d &rvec, cv::Vec3d &tvec,                     // 作为返回平移向量tvec和旋转向量rvec
                    double &distance,                                     // 保留distance，即armor中心到相机光心的距离
                    double &tz,                                           // 保留相机z轴方向深度深度
                    std::vector<cv::Point2d> &reprojection_points);       // 返回重投影得到的角点，可选