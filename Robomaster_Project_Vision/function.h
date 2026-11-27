#pragma
#include <opencv2/opencv.hpp>

// 核心对象说明
// 结构体：灯条
struct LightBar
{
    bool colorFlag = false; // 颜色标志，true表示红色，false表示蓝色
    std::vector<cv::Point> contour = {}; // 轮廓
    cv::RotatedRect minRect = {}; // 最小旋转外接矩形
    cv::Point2f center = {}; // 中心点
    double length = -1; // 长度
    double width = -1; // 宽度
    double angle = -1; // 角度
    double area = -1; // 轮廓面积
    int id = -1; // 编号
};

// 结构体：装甲板
struct Armor
{       
    bool colorFlag; // 颜色标志，true表示红色，false表示蓝色    
    cv::Point2f center; // 中心点，单精度px
    std::vector<cv::Point2f> vertices = std::vector<cv::Point2f>(4); // 四角点集合，单精度px
    cv::Point2d reprojection_center; // 重投影中心点，双精度px
    std::vector<cv::Point2d> reprojection_points; // 重投影得到的角点，双精度px
    cv::Mat rmat_World2Camara; // 将装甲板从世界坐标系变换到相机坐标系的旋转矩阵
    
    // 需要写入 csv 的数据
    int frame_id = -1; // 可被外界更改的当前所处帧
    double time_s = -1; // 可被外界更改的当前所处时间，单位秒s
    int armor_id = -1; // 编号（先x小到大，x一样再y小到大）
    bool is_valid = false; // 装甲板是否有效（是否得到可用的四角点）    
    bool is_solvePnP = false; // PnP是否结算成功
    double tx_m = -1; // 相机坐标系下的装甲板中心x，单位米m
    double ty_m = -1; // 相机相机坐标系下的装甲板中心y，单位米m
    double tz_m = -1; // 相机相机坐标系下的装甲板中心z，单位米m
    double distance_m = -1; // 相光心与装甲板中心的距离，单位米m
    double rx_rad = -1; // 旋转向量 x 分量
    double ry_rad = -1; // 旋转向量 y 分量
    double rz_rad = -1; // 旋转向量 z 分量
    double reprojection_error_px = -1; // 重投影偏移值，单位像素px
    bool is_suspicious = false; // 是否可疑（重投影偏差是否超过了阈值）
};

// 全局临时变量声明
extern int image_width; // 可被外界更改的当前图像宽度
extern int image_height; // 可被外界更改的当前图像高度
extern int frame_id; // 可被外界更改的当前所处帧
extern double time_s; // 可被外界更改的当前所处时间
extern std::vector<Armor> armor_listTemp; // 临时装甲板数组数据，用于存储检测到的装甲板数据，便于写入 csv 文件

// 辅助功能函数声明
// 检测到的装甲板的排序签名
bool armor_idComp(const Armor& armor1, const Armor& armor2);

// 核心功能函数声明
// 图片标注（单帧接口函数），可调整参数为1选Debug模式（增加重投影标注）
void imageMark(cv::Mat& image, const int mode);

// 从图片中检测装甲板
std::vector<Armor> detectArmor(cv::Mat& target_mask);                             

// 位姿解算处理（依赖于全局装甲板变量来写入数据）
void pose_resolve(Armor& armor);