#pragma
#include <opencv2/opencv.hpp>

// 图片 mask 提取及其形态学处理的参数配置
struct mask_config
{    
    const cv::Scalar mask_hsv_ltv = cv::Scalar(0, 109, 150); // inRange的下限值
    const cv::Scalar mask_hsv_utr = cv::Scalar(37, 255, 255); // inRange的上限值
};

// 灯条配对条件的参数配置
struct lightPair_config
{
    const double distance_tv = 100; // 灯条中心点距离阈值px
    const double areaDiff_tv = 200; // 灯条面积差阈值px
    const double angleDiff_tv = 5; // 灯条旋转角度差阈值px
    const double heightDiff_tv = 30; // 灯条中心点高度差阈值px
    const double distanceRatio_utv = 1.3; // 灯条中心点距离和装甲板高度比阈值上限
    const double distanceRatio_ltv = 0.9; // 灯条中心点距离和装甲板高度比阈值下限
};

// 装甲板的参数配置
struct armor_config
{
    const double armor_object_width = 0.135; // 装甲板实体宽度 0.135m
    const double armor_object_height = 0.056; // 装甲板实体高度 0.056m
    const double armor_image_width = 73; // 装甲板像宽度 73px
    const double armor_image_height = 78; // 装甲板像高度 78px    
    const double armor_removeDuplicates_tv = 30; // 装甲板去重阈值 30px
    const double armor_reprojection_error_tv_px = 1.0; // 重投影与原始点的误差可疑阈值 1.0px

    // 世界坐标系（以装甲板中心点为原点）的装甲板四角点坐标
    std::vector<cv::Point3d> armor_object_vertices = 
    {
        {-armor_object_width / 2, -armor_object_height / 2, 0}, // 左上
        {armor_object_width / 2, -armor_object_height / 2, 0},  // 右上
        {armor_object_width / 2, armor_object_height / 2, 0},   // 右下
        {-armor_object_width / 2, armor_object_height / 2, 0}   // 左下
    };
};

// 相关的矩阵参数配置
struct matrix_config
{
    // 相机内参矩阵
    cv::Mat K = (cv::Mat_<double>(3, 3) << 
                 1769.321404, 0, 762.985097,
                 0, 1768.461805, 572.3718881,
                 0, 0, 1);

    // 相机镜头畸变系数矩阵
    cv::Mat D = (cv::Mat_<double>(1, 5) << 0.1301456438, -3.237695144, -0.0007849919824, 0.001116547072, 17.43296795);
};