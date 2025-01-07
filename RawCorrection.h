#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RawCorrection.h"
#include <QString>
#include <QImage>

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

#include "RawCorrectionAlg.h"       //У���㷨��

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
    // ���ð�ť�ۺ���
    void SlotSetImageSizeButtonDown();

    // ����ͼƬ��ť�ۺ���
    void SlotImportButtonDown();

    // ��ʾͼƬ��ť�ۺ���
    void SlotShowButtonDown();

    // У����ť�ۺ���
    void SlotRawCorrectionButtonDown();

    // ���水ť�ۺ���
    void SlotSaveButtonDown();

    // SpinBox��ֵ��ı�ۺ���
    void SlotChangeSpinBoxValue(int);

    // SpinBox��ֵ��ı�ۺ���
    void SlotChangeSpinBoxValue(double);

    // QScrollBar��ֵ��ı�ۺ���
    void SlotChangeScrollBarValue(int);

private:
    // �������Mat������
    void MatRelease();

    // ��ʾָ��cv::Matͼ��lable��
    // ����˵����
    //  image:��ʾ��ͼ
    void ShowImage(const cv::Mat& image);

private:
    Ui::RawCorrectionClass ui;

    int     m_int32Width;           //ͼ����
    int     m_int32Height;          //ͼ��߶�
    int     m_int32DeadPixelHeight; //����У������ֵ
    int     m_int32DeadPixelLow;    //����У������ֵ
    double  m_f64AirMul;            //����У������ϵ��

    RawCorrectionAlg     m_RawCorrectionAlg;     //rawУ���㷨��

    // ͼ�����
    cv::Mat m_MatProj;              //ԭͼ
    cv::Mat m_MatDark;              //����ͼ
    cv::Mat m_MatBkg;               //����ͼ
    cv::Mat m_MatTemp;              //У���е�ͼ��

    QString m_strProjRawFilePath;     //ԭͼ·��
    QString m_strDarkRawFilePath;     //����ͼ·��
    QString m_strBkgRawFilePath;      //����ͼ·��

    QStringList   m_strListProjPaths;           // �洢ͼƬ·��
    int           M_int32CurrentIndex;         // ��ǰͼƬ����
};
