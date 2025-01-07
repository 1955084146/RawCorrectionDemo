#pragma once
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <QImage>

using namespace cv;
using namespace std;

#define DEFAULT_DEADPIXLHEIGHT 208      //坏点默认高阈值
#define DEFAULT_DEADPIXLLOW     0       //坏点默认低阈值

#define DEFAULT_BADMAP_DIR  "./DirBadMap"       //坏点图默认存储文件夹
#define DEFAULT_BADMAP_FILE  "badMap.raw"       //坏点图默认文件
#define DEFAULT_BADMAP_HEIGHT 1024              //坏点图默认文件高
#define DEFAULT_BADMAP_WIDTH  1024              //坏点图默认文件宽

class RawCorrectionAlg
{
public:
    RawCorrectionAlg();
    ~RawCorrectionAlg();

public:
    // 暗场校正
    // 注意：同一图像请勿重复执行暗场校正，以免影响结果。
    // 参数说明：
    //   image      - 输入图像（原始图像）。
    //   darkField  - 暗场图（用于校正的背景图像）。
    // 返回值：
    //   cv::Mat    - 校正后的图像。
    Mat DarkFieldCorrection(const Mat& image, const Mat& darkField);

    // 平场校正
    // 参数说明：
    //  image:      原图
    //  flatField:  背景图(平场图)
    // 返回值：
    //   cv::Mat    - 校正后的图像。
    Mat FlatFieldCorrection(const Mat& image, const Mat& flatField);

    // 空气校正（亮度调整）
    // 参数说明：
    //  factor:     亮度倍数
    // 返回值：
    //   cv::Mat    - 校正后的图像。
    Mat AirCorrection(const Mat& image, double factor);

    // 校正坏点
    // 参数说明：
    //  image:              原图
    // 返回值：
    //   cv::Mat    - 校正后的图像。
    Mat BadPixelsCorrection(const Mat& image);

    // 预处理校正  暗场校正->坏点校正->平场校正
    // 参数说明：
    //  image:      原图
    //  darkField:  暗场图
    //  flatField:  背景图(平场图)
    // 返回值：
    //   cv::Mat    - 校正后的图像。
    Mat AutoImageCorrection(const Mat& image, const Mat& darkField, const Mat& flatField);

    // 读取16位的Raw格式文件，将其转换为cv::Mat
    // 参数说明：
    //  image:输出Mat文件
    //  rawFilePath:文件路径
    //  height:图高
    //  width:图宽
    // 返回值：
    //   int    - 成功返回0 失败返回-1
    int ReadRawToMat(cv::Mat& image, std::string rawFilePath, int height, int width);

    // 将不同位数cv::Mat转化为指定位数
    // 参数说明：
    //  inputImage:     输入图像
    //  outputImage:    输出图像
    //  targetType:     目标图像类型，例如 CV_8U, CV_16U, CV_32F 等
    //  scale:          缩放因子，用于调整输入值的范围             用于归一化时使用
    //  offset:         偏移量，用于对每个像素值添加固定偏移       用于归一化时使用
    void ConvertMatImage(const Mat& inputImage, Mat& outputImage, int targetType, double scale = 1.0, double offset = 0.0);

    // 将任意位数cv::Mat转为QImage对象，方便在Qt的界面内显示
    // 参数说明：
    //  mat:        输入图像
    // 返回值：
    //   QImage    - 输出图像
    QImage MatToQImage(const cv::Mat& mat);

    // 读取固定文件夹里的坏点图到程序中,或者通过传入一张图片获取坏点图
    // 返回值：
    //  是否更新成功
    int UpdateBadPixelsRaw();

private:
    Mat m_MatBadPixelsImage;            //坏点图    8位
    bool m_bIsExistBadPixelsImage;      //坏点图是否存在
};

