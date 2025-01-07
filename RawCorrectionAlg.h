#pragma once
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <QImage>

using namespace cv;
using namespace std;

#define DEFAULT_DEADPIXLHEIGHT 208      //����Ĭ�ϸ���ֵ
#define DEFAULT_DEADPIXLLOW     0       //����Ĭ�ϵ���ֵ

#define DEFAULT_BADMAP_DIR  "./DirBadMap"       //����ͼĬ�ϴ洢�ļ���
#define DEFAULT_BADMAP_FILE  "badMap.raw"       //����ͼĬ���ļ�
#define DEFAULT_BADMAP_HEIGHT 1024              //����ͼĬ���ļ���
#define DEFAULT_BADMAP_WIDTH  1024              //����ͼĬ���ļ���

class RawCorrectionAlg
{
public:
    RawCorrectionAlg();
    ~RawCorrectionAlg();

public:
    // ����У��
    // ע�⣺ͬһͼ�������ظ�ִ�а���У��������Ӱ������
    // ����˵����
    //   image      - ����ͼ��ԭʼͼ�񣩡�
    //   darkField  - ����ͼ������У���ı���ͼ�񣩡�
    // ����ֵ��
    //   cv::Mat    - У�����ͼ��
    Mat DarkFieldCorrection(const Mat& image, const Mat& darkField);

    // ƽ��У��
    // ����˵����
    //  image:      ԭͼ
    //  flatField:  ����ͼ(ƽ��ͼ)
    // ����ֵ��
    //   cv::Mat    - У�����ͼ��
    Mat FlatFieldCorrection(const Mat& image, const Mat& flatField);

    // ����У�������ȵ�����
    // ����˵����
    //  factor:     ���ȱ���
    // ����ֵ��
    //   cv::Mat    - У�����ͼ��
    Mat AirCorrection(const Mat& image, double factor);

    // У������
    // ����˵����
    //  image:              ԭͼ
    // ����ֵ��
    //   cv::Mat    - У�����ͼ��
    Mat BadPixelsCorrection(const Mat& image);

    // Ԥ����У��  ����У��->����У��->ƽ��У��
    // ����˵����
    //  image:      ԭͼ
    //  darkField:  ����ͼ
    //  flatField:  ����ͼ(ƽ��ͼ)
    // ����ֵ��
    //   cv::Mat    - У�����ͼ��
    Mat AutoImageCorrection(const Mat& image, const Mat& darkField, const Mat& flatField);

    // ��ȡ16λ��Raw��ʽ�ļ�������ת��Ϊcv::Mat
    // ����˵����
    //  image:���Mat�ļ�
    //  rawFilePath:�ļ�·��
    //  height:ͼ��
    //  width:ͼ��
    // ����ֵ��
    //   int    - �ɹ�����0 ʧ�ܷ���-1
    int ReadRawToMat(cv::Mat& image, std::string rawFilePath, int height, int width);

    // ����ͬλ��cv::Matת��Ϊָ��λ��
    // ����˵����
    //  inputImage:     ����ͼ��
    //  outputImage:    ���ͼ��
    //  targetType:     Ŀ��ͼ�����ͣ����� CV_8U, CV_16U, CV_32F ��
    //  scale:          �������ӣ����ڵ�������ֵ�ķ�Χ             ���ڹ�һ��ʱʹ��
    //  offset:         ƫ���������ڶ�ÿ������ֵ��ӹ̶�ƫ��       ���ڹ�һ��ʱʹ��
    void ConvertMatImage(const Mat& inputImage, Mat& outputImage, int targetType, double scale = 1.0, double offset = 0.0);

    // ������λ��cv::MatתΪQImage���󣬷�����Qt�Ľ�������ʾ
    // ����˵����
    //  mat:        ����ͼ��
    // ����ֵ��
    //   QImage    - ���ͼ��
    QImage MatToQImage(const cv::Mat& mat);

    // ��ȡ�̶��ļ�����Ļ���ͼ��������,����ͨ������һ��ͼƬ��ȡ����ͼ
    // ����ֵ��
    //  �Ƿ���³ɹ�
    int UpdateBadPixelsRaw();

private:
    Mat m_MatBadPixelsImage;            //����ͼ    8λ
    bool m_bIsExistBadPixelsImage;      //����ͼ�Ƿ����
};

