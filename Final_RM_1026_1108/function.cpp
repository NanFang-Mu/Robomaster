#include "function.h"
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/geometry.hpp>
#include <string>
#include <algorithm>

// 函数：返回装甲板容器，参数为4个可修改的矩阵，一个颜色标志(识别红色)
std::vector<Armor> detectArmor(cv::Mat &target_mask, cv::Mat &lightbar_candidate, cv::Mat &armor_result, bool colorFlag)
{
    // 把target_mask转为hsv
    cv::cvtColor(target_mask, target_mask, cv::COLOR_BGR2HSV);

    // 对target_mask进行滤波，消除噪声
    cv::GaussianBlur(target_mask, target_mask, cv::Size(5, 5), 0);

    // 对target_mask进行hsv二值化处理
    cv::inRange(target_mask, cv::Scalar(0, 109, 182), cv::Scalar(37, 255, 255), target_mask);

    // 对target_mask进行开运算
    cv::erode(target_mask, target_mask, cv::Mat(), cv::Point(-1, -1), 2);
    cv::dilate(target_mask, target_mask, cv::Mat(), cv::Point(-1, -1), 5);

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
        lightBars[i].colorFlag = colorFlag; // 颜色标志，true表示红色，false表示
    }

    // 遍历灯条容器,在lightbar_candidate.png上绘制轮廓的最小外接旋转矩形，中心点和对应编号
    for (int i = 0; i < lightBars.size(); i++)
    {

        // 遍历灯条容器,在lightbar_candidate.png上绘制轮廓的最小外接旋转矩形
        cv::Point2f vertices[4];
        lightBars[i].minRect.points(vertices);
        for (int j = 0; j < 4; j++)
        {
            cv::line(lightbar_candidate, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }

        cv::Point2f center = lightBars[i].minRect.center;
        cv::putText(lightbar_candidate, std::to_string(lightBars[i].id), center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
        cv::circle(lightbar_candidate, center, 2, cv::Scalar(0, 255, 0), -1);
        //同时在终端输出面积,长度,宽度,角度,中心点坐标，颜色标志
        // std::cout << "LightBar " << lightBars[i].id << " Area:" << lightBars[i].area << std::endl;
        // std::cout << "LightBar " << lightBars[i].id << " Length:" << lightBars[i].length << std::endl;
        // std::cout << "LightBar " << lightBars[i].id << " Width:" << lightBars[i].width << std::endl;
        // std::cout << "LightBar " << lightBars[i].id << " Angle:" << lightBars[i].angle << std::endl;
        // std::cout << "LightBar " << lightBars[i].id << " center:" << lightBars[i].minRect.center << std::endl;
        // std::cout << "LightBar " << lightBars[i].id << " Color:" << lightBars[i].colorFlag << std::endl;
        // std::cout << std::endl;
    }

    // 声明一个装甲版容器
    std::vector<Armor> armors;

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

            // 终端输出上述数据
            // std::cout << "LightBar " << i + 1 << " and " << "LightBar " << j + 1 << " comparison:" << std::endl;
            // std::cout << "Distance: " << distance << std::endl;
            // std::cout << "Area Difference: " << areaDiff << std::endl;
            // std::cout << "Height Difference: " << heightDiff << std::endl;
            // std::cout << "Angle Difference: " << angleDiff << std::endl;            
            // std::cout << "DistanceRatio: " << distanceRatio << std::endl;
            // std::cout << std::endl;

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
                // 全部数据通过，视为有效
                armor.isValid = true;                

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

                // 输出灯条四角点坐标
                // std::cout << std::endl << "灯条四角点坐标：" << std::endl;
                // for (int k = 0; k < 4; k++)
                // {
                //     std::cout << "LightBar " << lightBars[i].id << " Vertex " << k << ": (" << pts_i[k].x << ", " << pts_i[k].y << ")" << std::endl;
                // }
                // for (int k = 0; k < 4; k++)
                // {
                //     std::cout << "LightBar " << lightBars[j].id << " Vertex " << k << ": (" << pts_j[k].x << ", " << pts_j[k].y << ")" << std::endl;
                // }
                // std::cout << std::endl;

                // 根据x坐标判断左右灯条
                if (lightBars[i].minRect.center.x < lightBars[j].minRect.center.x)
                {
                    //i,j
                    armor.vertices[0] = pts_i[2]; // 左上
                    armor.vertices[1] = pts_j[3]; // 右上
                    armor.vertices[2] = pts_j[0]; // 右下
                    armor.vertices[3] = pts_i[1]; // 左下
                }
                else
                {
                    //j,i
                    armor.vertices[0] = pts_j[2];
                    armor.vertices[1] = pts_i[3];
                    armor.vertices[2] = pts_i[0];
                    armor.vertices[3] = pts_j[1];
                }

                // 在终端输出armor的颜色标志，中心点，四角点，是否有效                    
                // std::cout << "Armor Color: " << armor.colorFlag << std::endl;
                // std::cout << "Armor Center: (" << armor.center.x << ", " << armor.center.y << ")" << std::endl;
                // std::cout << "Armor Vertex: ";
                // for (cv::Point2f Point : armor.vertices)
                // {
                //     std::cout << "(" << Point.x << ", " << Point.y << ")" << " ";
                // }
                // std::cout << std::endl;
                // std::cout << "Armor Valid: " << armor.isValid << std::endl;
                // std::cout << std::endl;

                // 根据armor.vertices连线
                for (int k = 0; k < 4; k++)
                {
                    cv::line(armor_result, armor.vertices[k], armor.vertices[(k + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }

                // 在图像上绘制装甲板中心点
                cv::circle(armor_result, armor.center, 5, cv::Scalar(0, 255, 0), -1);                

                // 新增有效装甲板进入容器中
                armors.push_back(armor);
                armors[armors.size() - 1].id = armors.size();

                // 在armor_result上绘制装甲板的编号
                cv::putText(armor_result, std::to_string(armors[armors.size() - 1].id), armor.center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);

                // 在armor_result上绘制装甲板的四角点编号
                for (int v = 0; v < 4; v++)
                {
                    cv::circle(armor_result, armor.vertices[v], 5, cv::Scalar(0, 255, 0), -1);
                    std::string text = std::to_string(armors.size()) + "." + std::to_string(v);
                    cv::putText(armor_result, text, armor.vertices[v], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
                }
            }
        }
    }    

    // 返回装甲板容器
    return armors;
}