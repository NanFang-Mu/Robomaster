#include "function.h"
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/geometry.hpp>
#include <string>
#include <cmath>
#include <algorithm>

// 图片标注
void imageMark(cv::Mat& image)
{
    // 声明或定义变量
    cv::Mat image_temp = image.clone();
    std::vector<Armor> armor_list = detectArmor(image_temp);
    bool is_validArmor;

    // 如果armor_list为空，则跳过标注
    if (armor_list.empty())
    {
        is_validArmor = false;
        return;
    }
    // 否则进行标注
    else
    {
        is_validArmor = true;
        for (const Armor &armor : armor_list)
        {            
            // 装甲板检测情况的图片标注           
            // 绘制装甲板的四角点连线
            for (int k = 0; k < armor.vertices.size(); k++)
            {
                cv::line(image, armor.vertices[k], armor.vertices[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
            }
            // 绘制装甲板的四角点及编号
            for (int v = 0; v < armor.vertices.size(); v++)
            {
                cv::circle(image, armor.vertices[v], 4, cv::Scalar(0, 255, 255), -1);                
                cv::putText(image, std::to_string(v), armor.vertices[v] + cv::Point2f(5, -5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
            }           
            // 在装甲板的中心绘制准心
            cv::drawMarker(image, armor.center, cv::Scalar(0, 255, 255), cv::MARKER_CROSS, 10, 2);
        }
    }

    // 对装甲板数组里的每一块装甲板进行重投影处理
    for (int i = 0; i < armor_list.size(); i++)
    {
        // 声明相关位姿解算需要的变量
        cv::Vec3d rvec;
        cv::Vec3d tvec;                               // 作为返回平移向量tvec和旋转向量rvec
        double distance = 0;                          // 保留distance，即armor中心到相机光心的距离
        double tz = 0;                                // 保留相机z轴方向深度深度
        std::vector<cv::Point2d> reprojection_points; // 返回重投影得到的角点，可选

        // 尝试解算位姿
        armor_list[i].is_PnP = pose_is_solved(armor_list[i].vertices, rvec, tvec, distance, tz, reprojection_points);

        // 解算位姿成功时
        if (armor_list[i].is_PnP)
        {
            // 计算reprojection_Points数组里的四个点的x，y各自的算术平均值
            double x_average = 0, y_average = 0;
            cv::Point2d reprojection_center;
            double x_sum = 0, y_sum = 0;
            for (int v = 0; v < reprojection_points.size(); v++)
            {
                x_sum += reprojection_points[v].x;
                y_sum += reprojection_points[v].y;
            }
            x_average = x_sum / reprojection_points.size();
            y_average = y_sum / reprojection_points.size();
            reprojection_center.x = x_average;
            reprojection_center.y = y_average;

            // 位姿解算情况的图片标注
            // 准备文本
            std::string text1, text2;
            if (is_validArmor)
            {
                text1 = "检测到装甲板    Armor_id : " + std::to_string(armor_list[i].id);
            }
            if (armor_list[i].is_PnP)
            {
                text2 = "姿态解算成功";
            }
            else
            {
                text2 = "姿态解算失败";
            }
            // 计算文本高度宽度
            int baseline = 0;
            cv::Size textSize = cv::getTextSize(text1 + "    " + text2, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            // 绘制黑色背景矩形
            cv::rectangle(image, armor_list[i].vertices[0] + cv::Point2f(0, -textSize.height - 20), armor_list[i].vertices[0] + cv::Point2f(textSize.width, -20), cv::Scalar(0, 0, 0), cv::FILLED);
            // 绘制黄色正常或红色可疑文字
            if (is_validArmor && armor_list[i].is_PnP)
            {
                cv::putText(image, text1 + "    " + text2,
                            armor_list[i].vertices[0] + cv::Point2f(0, -20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
            }
            else
            {
                cv::putText(image, text1 + "    " + text2,
                            armor_list[i].vertices[0] + cv::Point2f(0, -20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
            }

            // 可注释项：重投影情况的图片标注
            for (int k = 0; k < reprojection_points.size(); k++)
            {
                // 四角点连线
                cv::line(image, reprojection_points[k], reprojection_points[(k + 1) % 4], cv::Scalar(253, 86, 224), 2);
                // 绘制重投影的四角点
                cv::circle(image, reprojection_points[k], 4, cv::Scalar(139, 0, 139), -1);
            }
            // 在重投影的中心绘制准心
            cv::drawMarker(image, reprojection_center, cv::Scalar(139, 0, 139), cv::MARKER_CROSS, 10, 2);
        }
    }
}


// 从图片中检测装甲板
std::vector<Armor> detectArmor(cv::Mat& target_mask)
{
    // 把target_mask转为hsv
    cv::cvtColor(target_mask, target_mask, cv::COLOR_BGR2HSV);

    // 对target_mask进行滤波，消除噪声
    cv::GaussianBlur(target_mask, target_mask, cv::Size(5, 5), 0);

    // 对target_mask进行hsv二值化处理
    cv::inRange(target_mask, cv::Scalar(0, 109, 150), cv::Scalar(37, 255, 255), target_mask);

    // 对target_mask进行开运算
    cv::erode(target_mask, target_mask, cv::Mat(), cv::Point(-1, -1), 1);
    cv::dilate(target_mask, target_mask, cv::Mat(), cv::Point(-1, -1), 1);

    // 轮廓集合
    std::vector<std::vector<cv::Point>> contours;

    // 寻找target_mask中的轮廓,并把找到的轮廓放进contours容器中
    cv::findContours(target_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 根据轮廓数，用轮廓信息初始化灯条结构体，并把灯条结构体放入灯条容器中，并计算相应的轮廓面积，最小旋转外接矩形，并给予编号
    std::vector<LightBar> lightBars(contours.size());
    for (size_t i = 0; i < contours.size(); ++i)
    {
        lightBars[i].contour = contours[i];
        lightBars[i].minRect = cv::minAreaRect(contours[i]);
        lightBars[i].center = lightBars[i].minRect.center;
        lightBars[i].length = lightBars[i].minRect.size.height;
        lightBars[i].width = lightBars[i].minRect.size.width;
        lightBars[i].angle = lightBars[i].minRect.angle;
        //把角度映射到(0, -180]
        if (lightBars[i].width < lightBars[i].length)
        {
            std::swap(lightBars[i].length, lightBars[i].width); // 确保width>=height
            lightBars[i].angle -= 90;
        }

        lightBars[i].area = cv::contourArea(contours[i]);
        lightBars[i].id = i + 1;
        lightBars[i].colorFlag = true; // 颜色标志，true表示红色，false表示不是红色
    }

    // 声明一个装甲版容器
    std::vector<Armor> armor_list;

    // 遍历灯条容器，按一定条件进行配对
    for (size_t i = 0; i < lightBars.size(); ++i)
    {
        for (size_t j = i + 1; j < lightBars.size(); ++j)
        {
            // 判断灯条颜色是否相同
            if (lightBars[i].colorFlag == lightBars[j].colorFlag)
            {
                bool isColorFlagMatched = true;
            }
            else
            {
                bool isColorFlagMatched = false;
                continue;
            }

            // 计算两个灯条之间的距离
            double distance = cv::norm(lightBars[i].minRect.center - lightBars[j].minRect.center);
            // 计算两个灯条的面积差
            double areaDiff = std::abs(lightBars[i].minRect.size.area() - lightBars[j].minRect.size.area());
            // 计算两个灯条的角度差
            double angleDiff = std::abs(lightBars[i].angle - lightBars[j].angle);
            // 计算两个灯条中心点的高度差
            double heightDiff = std::abs(lightBars[i].minRect.center.y - lightBars[j].minRect.center.y);
            // 声明装甲板的宽度和长度（沿用angle定义）
            double armor_width = 73;
            double armor_length = 78;
            // 声明灯条距离差与装甲板宽度的比值
            double distanceRatio = distance / armor_width;

            // 声明一个临时装甲板结构体
            Armor armor;

            // 如果颜色、距离、面积差、角度差和高度差都满足装甲板的条件，则认为找到了
            if (   distance < 100
                && areaDiff < 200
                && angleDiff < 5 
                && heightDiff < 30 
                && distanceRatio < 1.3
                && distanceRatio > 0.9)    
            {                                    
                // 将配对的两个灯条的所有角点放到容器中
                std::vector<cv::Point> armorPoints;
                // 获取灯条i的minRect成员的的四个角点
                cv::Point2f rectPoints[4];
                lightBars[i].minRect.points(rectPoints);
                for (int j = 0; j < 4; j++)
                {
                    armorPoints.push_back(rectPoints[j]);
                }
                // 获取灯条j的minRect成员的的四个角点
                cv::Point2f rectPoints2[4];
                lightBars[j].minRect.points(rectPoints2);
                for (int j = 0; j < 4; j++)
                {
                    armorPoints.push_back(rectPoints2[j]);
                }                

                // 根据灯条颜色来初始化装甲板的颜色标志
                armor.colorFlag = lightBars[i].colorFlag;

                // 计算配对灯条中心点连线的中心点
                cv::Point2f centerPoint = (lightBars[i].minRect.center + lightBars[j].minRect.center) / 2;

                //  根据中心点初始化装甲板中心点
                armor.center = centerPoint;

                // 当前配对的是灯条i和j
                cv::Point2f pts_i[4], pts_j[4];
                lightBars[i].minRect.points(pts_i);
                lightBars[j].minRect.points(pts_j);    

                // 将两根灯条的8个顶点放到一个容器里
                std::vector<cv::Point2f> all_pts;
                for (int k = 0; k < 4; k++)
                {
                    all_pts.push_back(pts_i[k]);
                    all_pts.push_back(pts_j[k]);
                }

                // 寻找8个点中的四个极值点
                cv::Point2f tl, tr, br, bl;
                float min_sum = 1e6;   // 找左上角：x+y 最小
                float max_sum = -1e6;  // 找右下角：x+y 最大
                float min_diff = 1e6;  // 找右上角：y-x 最小 (或者 x-y 最大)
                float max_diff = -1e6; // 找左下角：y-x 最大

                for (const auto &pt : all_pts)
                {
                    float sum = pt.x + pt.y;
                    float diff = pt.y - pt.x;

                    if (sum < min_sum)
                    {
                        min_sum = sum;
                        tl = pt;
                    }
                    if (sum > max_sum)
                    {
                        max_sum = sum;
                        br = pt;
                    }
                    if (diff < min_diff)
                    {
                        min_diff = diff;
                        tr = pt;
                    }
                    if (diff > max_diff)
                    {
                        max_diff = diff;
                        bl = pt;
                    }
                }

                // 按照顺序赋值
                armor.vertices[0] = tl; // 左上
                armor.vertices[1] = tr; // 右上
                armor.vertices[2] = br; // 右下
                armor.vertices[3] = bl; // 左下                               

                // 新增有效装甲板进入容器中
                armor_list.push_back(armor);
                armor_list[armor_list.size() - 1].id = armor_list.size() - 1;            
            }
        }
    }

    // 去重
    std::vector<Armor> final_armor_list;
    for (size_t i = 0; i < armor_list.size(); i++)
    {
        bool is_duplicate = false;
        for (size_t j = 0; j < final_armor_list.size(); j++)
        {
            // 计算两个候选装甲板中心点的距离
            double dist = cv::norm(armor_list[i].center - final_armor_list[j].center);
            // 如果两个框中心点距离小于30像素，就认定它们是同一个目标
            if (dist < 30.0)
            {
                is_duplicate = true;
                break;
            }
        }
        if (!is_duplicate) // 不是重复，就添加到最终装甲板容器中
        {
            final_armor_list.push_back(armor_list[i]);
            final_armor_list[final_armor_list.size() - 1].id = final_armor_list.size() - 1; // 重新编号
        }
    }

    // 返回装甲板容器
    return final_armor_list;
}

// 计算位姿是否有效
bool pose_is_solved(const std::vector<cv::Point2f> &armor_image_vertices, // 像素坐标系下的装甲板四角点坐标（已经进行过点序规范）                    
                    cv::Vec3d &rvec, cv::Vec3d &tvec, // 作为返回平移向量tvec和旋转向量rvec
                    double &distance, // 保留distance，即armor中心到相机光心的距离
                    double &tz,// 保留相机z轴方向深度深度 
                    std::vector<cv::Point2d> &reprojection_points) // 返回重投影得到的角点，可选                   
{
    // 3D 模型点（单位：米），原点在装甲板中心，板面位于 Z=0 平面
    const double w = 0.135;
    const double h = 0.056;
    std::vector<cv::Point3d> armor_world_vertices = // 世界坐标系（以装甲板中心点为原点）的装甲板四角点坐标
    {
        {-w / 2, -h / 2, 0}, // 左上
        {w / 2, -h / 2, 0},  // 右上
        {w / 2, h / 2, 0},   // 右下
        {-w / 2, h / 2, 0}   // 左下
    };

    // 相机内参与畸变系数
    cv::Mat K = (cv::Mat_<double>(3, 3) << 
                 1769.321404, 0, 762.985097,
                 0, 1768.461805, 572.3718881,
                 0, 0, 1);
    cv::Mat D = (cv::Mat_<double>(1, 5) << 0.1301456438, -3.237695144, -0.0007849919824, 0.001116547072, 17.43296795);

    // 把传入的armor_image_vertices又float转换为double数据类型
    std::vector<cv::Point2d> armor_image_vertices2;
    for (int i = 0; i < armor_image_vertices.size(); i++)
    {
        armor_image_vertices2.push_back(static_cast<cv::Point2d>(armor_image_vertices[i]));
    }

    // 返回值状态确认是否成功求解rvec和tvec
    const bool is_solved = cv::solvePnP(armor_world_vertices, armor_image_vertices2, K, D, rvec, tvec);

    if (is_solved)
    {   
        // 根据solvePnP得到的tvec计算distance，distance装甲板中心到相机光心的距离
        distance = std::sqrt(tvec[0] * tvec[0] + tvec[1] * tvec[1] + tvec[2] * tvec[2]);

        //保留tz，作为装甲板中心在相机坐标系下Z轴上的深度
        tz = std::sqrt(tvec[2] * tvec[2]); 

        // 根据solvePnP得到的rvec计算角度
        cv::Mat rotation_matrix;
        cv::Rodrigues(rvec, rotation_matrix);

        // 重投影，得到一组重投影的四角点        
        cv::projectPoints(armor_world_vertices, rvec, tvec, K, D, reprojection_points);

        // 对原始四角点和重投影四角点进行误差分析
        double reprojection_error_px = 0;
        double e_arr[4] = {0, 0, 0, 0};
        for (int i = 0; i < 4; i++)
        {
            e_arr[i] = sqrt(std::pow(armor_image_vertices2[i].x - reprojection_points[i].x, 2)
                          + std::pow(armor_image_vertices2[i].y - reprojection_points[i].y, 2));
        }

        return true;
    }
    else
    {
        // 解算失败则返回空矩阵，后续用.size()作为条件跳过其绘图
        reprojection_points = {};    
        return false;
    }
}