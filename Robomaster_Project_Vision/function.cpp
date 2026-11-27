#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <opencv2/geometry.hpp>

#include "function.h"
#include "config.h"

// 全局临时变量定义
int image_width = 1;
int image_height = 1;
int frame_id = -1;
double time_s = -1;
std::vector<Armor> armor_listTemp;

// 图片标注，可调整参数为1选Debug模式（增加重投影标注）
void imageMark(cv::Mat &image, const int mode)
{
    // 全局变量赋值
    if (image.empty())
    {
        return;
    }
    else
    {
        ::image_width = image.cols;
        ::image_height = image.rows;
    }

    // 声明或定义变量
    cv::Mat image_temp = image.clone();
    std::vector<Armor> armor_list = detectArmor(image_temp);

    // 全局临时变量根据该帧装甲板数量进行设置
    for (int i = 0; i < armor_list.size(); i++)
    {
        ::armor_listTemp.push_back(armor_list[i]); // 写入的装甲板已经有效（从detectArmor函数返回来的都成功提取了四角点）
        ::armor_listTemp[i].frame_id = ::frame_id;
        ::armor_listTemp[i].time_s = ::time_s;
    }

    // 接下来所有的操作将在全局临时变量的装甲板数组中记录

    // 若装甲板为空，视为未检测到装甲板，右上角红字提示，并结束标注
    if (::armor_listTemp.empty())
    {
        int baseline = 0;
        cv::Size textSize = cv::getTextSize("[No Candidate Armor]", cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
        cv::rectangle(image, cv::Point(10, 60 - textSize.height), cv::Point(18 + textSize.width, 60), cv::Scalar(0, 0, 0), cv::FILLED);
        cv::putText(image, "[No Candidate Armor]", cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

        return;
    }

    // 若装甲板不为空，视为检测到装甲板，进行装甲板检测情况的图片标注
    else
    {
        for (Armor &armor : ::armor_listTemp)
        {
            // 计算是否可疑
            if (armor.reprojection_error_px > 1.0)
            {
                // 可疑标记
                armor.is_suspicious = true;
            }
            else
            {
                armor.is_suspicious = false;
            }

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

            // 位姿解算
            pose_resolve(armor);

            // 解算位姿成功时
            if (armor.is_solvePnP)
            {
                // 位姿解算情况的图片标注
                // 准备文本
                std::string text1, text2, text3;
                if (armor.is_valid)
                {
                    text1 = "[Find Armor " + std::to_string(armor.armor_id) + "]";
                }
                else
                {
                    text1 = "[No Candidate Armor]";
                }
                if (armor.is_solvePnP)
                {
                    text2 = "[SolvePnP Success]";
                    text3 = "[Distance " + std::to_string(armor.distance_m) + " m]";
                }
                else
                {
                    text2 = "[SolvePnP Failure]";
                    text3 = "";
                }

                // 计算文本高度宽度
                int baseline = 0;
                cv::Size textSize = cv::getTextSize(text1 + " " + text2 + " " + text3, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

                // 绘制黑色背景矩形
                cv::rectangle(image, armor.vertices[0] + cv::Point2f(0, -textSize.height - 20), armor.vertices[0] + cv::Point2f(textSize.width+1, -20), cv::Scalar(0, 0, 0), cv::FILLED);

                // 绘制 黄色正常 或 紫粉色可疑 或 红色未检测 等文字
                if (armor.is_valid)
                {
                    if (armor.is_solvePnP)
                    {
                        cv::putText(image, text1 + " " + text2 + " " + text3,
                                    armor.vertices[0] + cv::Point2f(0, -20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
                    }
                    else
                    {
                        cv::putText(image, text1 + " " + text2 + " " + text3, 
                                    armor.vertices[0] + cv::Point2f(0, -20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
                    }
                }

                // 可调整参数为1选Debug模式（增加重投影标注和可疑ROI裁剪记录）
                if (mode == 1)
                {
                    // 重投影标注
                    for (int k = 0; k < armor.reprojection_points.size(); k++)
                    {
                        // 四角点连线
                        cv::line(image, armor.reprojection_points[k], armor.reprojection_points[(k + 1) % 4], cv::Scalar(253, 86, 224), 2);
                        // 绘制重投影的四角点
                        cv::circle(image, armor.reprojection_points[k], 4, cv::Scalar(139, 0, 139), -1);
                    }
                    // 在重投影的中心绘制准心
                    cv::drawMarker(image, armor.reprojection_center, cv::Scalar(139, 0, 139), cv::MARKER_CROSS, 10, 2);   
                    
                    if (armor.is_suspicious == true)
                    {   
                        // 准备裁剪的图片
                        cv::Mat armor_suspicious_roi = image.clone();       

                        // 准备 ROI 数据
                        int tl_x = 0, tl_y = 0, br_x = ::image_width - 1, br_y = ::image_height - 1;
                        if (tl_x < armor.reprojection_points[0].x-100)
                        {
                            tl_x = armor.reprojection_points[0].x-100;
                        }
                        if (tl_y < armor.reprojection_points[0].y-100)
                        {
                            tl_y = armor.reprojection_points[0].y-100;
                        }
                        if (br_x > armor.reprojection_points[2].x+100)
                        {
                            br_x = armor.reprojection_points[2].x+100;
                        }
                        if (br_y > armor.reprojection_points[2].y+100)
                        {
                            br_y = armor.reprojection_points[2].y+100;                            
                        }

                        cv::Rect roi(tl_x, tl_y, br_x-tl_x, br_y-tl_y);
                        armor_suspicious_roi = armor_suspicious_roi(roi);

                        // 对图片对象进行保存
                        if (!(cv::imwrite("./output/armor_suspicious/frame_" + std::to_string(::frame_id) + "_armor_" + std::to_string(armor.armor_id) + ".png", armor_suspicious_roi)))
                        {
                            std::cerr << "图片保存失败，请检查输出路径 ./output/armor_suspicious/frame_" + std::to_string(::frame_id) + "_armor_" + std::to_string(armor.armor_id) + ".png 是否正确" << std::endl;
                            return;
                        }
                    }
                }
            }
        }
    }
}

// 从图片中检测装甲板
std::vector<Armor> detectArmor(cv::Mat& target_mask)
{
    // 图片 mask 提取及其形态学处理的参数
    mask_config mask_parameter;

    // 把target_mask转为hsv
    cv::cvtColor(target_mask, target_mask, cv::COLOR_BGR2HSV);

    // 对target_mask进行滤波，消除噪声
    cv::GaussianBlur(target_mask, target_mask, cv::Size(5, 5), 0);    

    // 对target_mask进行hsv二值化处理
    cv::inRange(target_mask, mask_parameter.mask_hsv_ltv, mask_parameter.mask_hsv_utr, target_mask);

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
            bool isColorFlagMatched = false; // true红 false蓝
            if (lightBars[i].colorFlag == lightBars[j].colorFlag)
            {
                isColorFlagMatched = true;
            }
            else
            {
                isColorFlagMatched = false;
                continue;
            }

            // 声明装甲板的相关参数配置
            armor_config armor_parameter;
            // 声明一个临时装甲板结构体
            Armor armor;
            // 声明灯条配对的条件参数配置
            lightPair_config lightPair_tv;

            // 计算两个灯条之间的距离
            double distance = cv::norm(lightBars[i].minRect.center - lightBars[j].minRect.center);
            // 计算两个灯条的面积差
            double areaDiff = std::abs(lightBars[i].minRect.size.area() - lightBars[j].minRect.size.area());
            // 计算两个灯条的角度差
            double angleDiff = std::abs(lightBars[i].angle - lightBars[j].angle);
            // 计算两个灯条中心点的高度差
            double heightDiff = std::abs(lightBars[i].minRect.center.y - lightBars[j].minRect.center.y);            
            // 声明灯条距离差与装甲板高度的比值
            double distanceRatio = distance / armor_parameter.armor_image_height;

            // 如果颜色、距离、面积差、角度差和高度差都满足装甲板的条件，则认为找到了
            if (   isColorFlagMatched == true
                && distance < lightPair_tv.distance_tv
                && areaDiff < lightPair_tv.areaDiff_tv
                && angleDiff < lightPair_tv.angleDiff_tv
                && heightDiff < lightPair_tv.heightDiff_tv
                && distanceRatio < lightPair_tv.distanceRatio_utv
                && distanceRatio > lightPair_tv.distanceRatio_ltv)    
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

                // 成功提取四角点，视为该装甲板有效
                armor.is_valid = true;
                
                // 根据四角点计算中心点
                cv::Point2f center = (tl + tr + br + bl) / 4.0;
                armor.center = center;

                // 新增有效装甲板进入容器中
                armor_list.push_back(armor);
                armor_list[armor_list.size() - 1].armor_id = armor_list.size() - 1;            
            }
        }
    }

    // 去重
    std::vector<Armor> final_armor_list;
    armor_config armor_parameter;
    for (size_t i = 0; i < armor_list.size(); i++)
    {
        bool is_duplicate = false;
        for (size_t j = 0; j < final_armor_list.size(); j++)
        {
            // 计算两个候选装甲板中心点的距离
            double dist = cv::norm(armor_list[i].center - final_armor_list[j].center);
            // 如果两个框中心点距离小于去重阈值，就认定它们是同一个目标
            if (dist < armor_parameter.armor_removeDuplicates_tv)
            {
                is_duplicate = true;
                break;
            }
        }
        if (!is_duplicate)
        {   
            // 不是重复，就添加到最终装甲板容器中
            final_armor_list.push_back(armor_list[i]);

            // 排序，先按中心点x小到大，x相等的又按y小到大
            std::sort(final_armor_list.begin(), final_armor_list.end(), armor_idComp);
        }
    }

    // 重新编号
    for (int i = 0; i < final_armor_list.size(); i++)
    {
        final_armor_list[i].armor_id = i;
    }

    // 返回装甲板容器
    return final_armor_list;
}

// 检测到的装甲板的排序签名
bool armor_idComp(const Armor& armor1, const Armor& armor2)
{
    if (armor1.center.x < armor2.center.x)
    {
        return true;
    }
    else if (armor1.center.x == armor2.center.x)
    {
        if (armor1.center.y < armor2.center.y)
        {
            return true;
        }
    }
    return false;    
}

// 位姿解算处理，注意位姿解算一定要确定所有数据都是double
void pose_resolve(Armor &armor)
{
    // 声明装甲板的相关参数配置
    armor_config armor_parameter;
    // 声明相关的矩阵参数配置
    matrix_config matrix_parameter;
    // 声明相关数据
    cv::Vec3d rvec, tvec; // 旋转向量和位移向量

    // 装甲板像的四角点成员由 float 转换为 double 数据类型
    std::vector<cv::Point2d> armor_image_vertices2d;
    for (int i = 0; i < armor.vertices.size(); i++) // 装甲板图像四角点已经排好序，直接依次赋值
    {
        armor_image_vertices2d.push_back(static_cast<cv::Point2d>(armor.vertices[i]));
    }

    // 返回值状态确认是否成功求解rvec和tvec
    const bool is_solvePnP = cv::solvePnP(armor_parameter.armor_object_vertices, armor_image_vertices2d,
                                          matrix_parameter.K, matrix_parameter.D,
                                          rvec, tvec);

    // 如果位姿解算成功
    if (is_solvePnP)
    {
        // 将位姿数据写入全局装甲板数组
        armor.is_solvePnP = true;
        armor.distance_m = std::sqrt(tvec[0] * tvec[0] + tvec[1] * tvec[1] + tvec[2] * tvec[2]);
        armor.tx_m = std::sqrt(tvec[0] * tvec[0]);
        armor.ty_m = std::sqrt(tvec[1] * tvec[1]);
        armor.tz_m = std::sqrt(tvec[2] * tvec[2]);
        armor.rx_rad = std::sqrt(rvec[0] * rvec[0]);
        armor.ry_rad = std::sqrt(rvec[1] * rvec[1]);
        armor.rz_rad = std::sqrt(rvec[2] * rvec[2]);
        cv::Rodrigues(rvec, armor.rmat_World2Camara);

        // 重投影，得到一组重投影的四角点
        cv::projectPoints(armor_parameter.armor_object_vertices,
                          rvec,
                          tvec,
                          matrix_parameter.K,
                          matrix_parameter.D,
                          armor.reprojection_points);

        // 对原始四角点和重投影四角点进行误差记录与分析
        if (4 == armor.vertices.size() && 4 == armor.reprojection_points.size()) // 不满足就不做分析，保持-1异常
        {
            // 计算重投影四角点的中心
            armor.reprojection_center = (armor.reprojection_points[0] + armor.reprojection_points[1] + armor.reprojection_points[2] + armor.reprojection_points[3]) / 4.0;

            // 计算四组角点中每组两个角点的距离，再取其算术平均值作为误差
            std::vector<double> armor_vertices_error;
            for (int i = 0; i < armor.vertices.size(); i++)
            {
                armor_vertices_error.push_back(std::sqrt(std::pow(armor.vertices[i].x - armor.reprojection_points[i].x, 2) +
                                                         std::pow(armor.vertices[i].y - armor.reprojection_points[i].y, 2)));
            }
            double armor_vertices_error_sum = 0;
            for (const double &error : armor_vertices_error)
            {
                armor_vertices_error_sum += error;
            }
            armor.reprojection_error_px = armor_vertices_error_sum / armor.vertices.size();

            // 计算误差是否可疑
            if (armor.reprojection_error_px > armor_parameter.armor_reprojection_error_tv_px)
            {
                armor.is_suspicious = true;
            }
        }

        return;
    }
    // 如果位姿解算失败
    else
    {
        armor.is_solvePnP = false;
        return;
    }
}
