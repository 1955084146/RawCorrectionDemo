#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RawCorrection.h"
#include <QString>
#include <QImage>

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

#include "RawCorrectionAlg.h"       //校正算法类

using namespace cv;
using namespace std;

class RawCorrectionAlg;
class RawCorrection : public QMainWindow
{
    Q_OBJECT

public:
    RawCorrection(QWidget *parent = nullptr);
    ~RawCorrection();

public Q_SLOTS:
    // 设置按钮槽函数
    void SlotSetImageSizeButtonDown();

    // 导入图片按钮槽函数
    void SlotImportButtonDown();

    // 显示图片按钮槽函数
    void SlotShowButtonDown();

    // 校正按钮槽函数
    void SlotRawCorrectionButtonDown();

    // 保存按钮槽函数
    void SlotSaveButtonDown();

    // SpinBox数值框改变槽函数
    void SlotChangeSpinBoxValue(int);

    // SpinBox数值框改变槽函数
    void SlotChangeSpinBoxValue(double);

    // QScrollBar数值框改变槽函数
    void SlotChangeScrollBarValue(int);

private:
    // 清空所有Mat的数据
    void MatRelease();

    // 显示指定cv::Mat图在lable上
    // 参数说明：
    //  image:显示的图
    void ShowImage(const cv::Mat& image);

private:
    Ui::RawCorrectionClass ui;

    int     m_int32Width;           //图像宽度
    int     m_int32Height;          //图像高度
    int     m_int32DeadPixelHeight; //坏点校正高阈值
    int     m_int32DeadPixelLow;    //坏点校正低阈值
    double  m_f64AirMul;            //空气校正增益系数

    RawCorrectionAlg     m_RawCorrectionAlg;     //raw校正算法类

    // 图像矩阵
    cv::Mat m_MatProj;              //原图
    cv::Mat m_MatDark;              //暗场图
    cv::Mat m_MatBkg;               //背景图
    cv::Mat m_MatTemp;              //校正中的图像

    QString m_strProjRawFilePath;     //原图路径
    QString m_strDarkRawFilePath;     //暗场图路径
    QString m_strBkgRawFilePath;      //背景图路径

    QStringList   m_strListProjPaths;           // 存储图片路径
    int           M_int32CurrentIndex;         // 当前图片索引
};
