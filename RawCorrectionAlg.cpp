#include "RawCorrectionAlg.h"
#include <qDebug>

RawCorrectionAlg::RawCorrectionAlg():
    m_bIsExistBadPixelsImage(false)
{
    //���뻵��ͼ
    UpdateBadPixelsRaw();
}

RawCorrectionAlg::~RawCorrectionAlg()
{

}

Mat RawCorrectionAlg::DarkFieldCorrection(const Mat& image, const Mat& darkField)
{
    Mat corrected;
    subtract(image, darkField, corrected);
    corrected = max(corrected, 0); // ��ֹ���ָ�ֵ
    return corrected;
}

Mat RawCorrectionAlg::FlatFieldCorrection(const Mat& image, const Mat& flatField)
{
    Mat flatFieldNormalized, corrected;
    Scalar meanFlatField = mean(flatField);
    flatFieldNormalized = flatField / meanFlatField[0]; // ��һ��ƽ��ͼ��
    divide(image, flatFieldNormalized, corrected);

    return corrected;
}

Mat RawCorrectionAlg::AirCorrection(const Mat& image, double factor)
{
    Mat corrected;
    image.convertTo(corrected, -1, factor, 0); // �������ȣ�factorΪ����
    return corrected;
}

Mat RawCorrectionAlg::BadPixelsCorrection(const Mat& image)
{
    //1���ж����޻���ͼ
    if (!m_bIsExistBadPixelsImage)
    {
        //���뻵��ͼ
        UpdateBadPixelsRaw();
        if (!m_bIsExistBadPixelsImage)
            return Mat(image);
    }

    //2���ж�ԭͼ�ͻ���ͼ�߿��Ƿ�һ��
    Mat rawImage = image;
    if (rawImage.size() != m_MatBadPixelsImage.size()) {
        qDebug() << "Error: RAW image and bad pixel map must have the same size.";
        return Mat(image);
    }

    //3����������ͼ,У������
    for (int y = 0; y < rawImage.rows; ++y) {
        for (int x = 0; x < rawImage.cols; ++x) {
            if (m_MatBadPixelsImage.at<uchar>(y, x) > 0) { // ����
                int count = 0;
                int sum = 0;

                // ���� 3x3 ����
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int ny = y + dy;
                        int nx = x + dx;

                        // ȷ����ͼ��Χ�ڲ��Ҳ��ǻ���
                        if (ny >= 0 && ny < rawImage.rows && nx >= 0 && nx < rawImage.cols &&
                            m_MatBadPixelsImage.at<uchar>(ny, nx) == 0) {
                            sum += rawImage.at<ushort>(ny, nx); // �ۼ�ֵ
                            ++count;
                        }
                    }
                }

                if (count > 0) {
                    rawImage.at<ushort>(y, x) = sum / count; // �������ֵ�޸�����
                }
            }
        }
    }

    return rawImage;
}

Mat RawCorrectionAlg::AutoImageCorrection(const Mat& image, const Mat& darkField, const Mat& flatField)
{
#if 1
    //1������У��
    Mat image_, flatField_;
    subtract(image, darkField, image_);         //��ԭͼ����һ�ΰ���У��
    image_ = max(image_, 0);
    subtract(flatField, darkField, flatField_); //�Ա���ͼ����һ�ΰ���У��
    flatField_ = max(flatField_, 0);

    //2������У��
    if (!m_bIsExistBadPixelsImage)
    {
        UpdateBadPixelsRaw();
        if (!m_bIsExistBadPixelsImage)
            return Mat(image);
    }

    if (image_.size() != m_MatBadPixelsImage.size() && flatField_.size() != m_MatBadPixelsImage.size()) {
        qDebug() << "Error: RAW image and bad pixel map must have the same size.";
        return Mat(image);
    }

    for (int y = 0; y < m_MatBadPixelsImage.rows; ++y) {
        for (int x = 0; x < m_MatBadPixelsImage.cols; ++x) {
            if (m_MatBadPixelsImage.at<uchar>(y, x) > 0) { // ����
                int count = 0;
                int sumImage_ = 0;
                int sumFlatField_ = 0;

                // ���� 3x3 ����
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int ny = y + dy;
                        int nx = x + dx;

                        // ȷ����ͼ��Χ�ڲ��Ҳ��ǻ���
                        if (ny >= 0 && ny < m_MatBadPixelsImage.rows && nx >= 0 && nx < m_MatBadPixelsImage.cols &&
                            m_MatBadPixelsImage.at<uchar>(ny, nx) == 0) {
                            sumImage_ += image_.at<ushort>(ny, nx); // �ۼ�ֵ
                            sumFlatField_ += flatField_.at<ushort>(ny, nx); // �ۼ�ֵ
                            ++count;
                        }
                    }
                }

                if (count > 0) {
                    image_.at<ushort>(y, x) = sumImage_ / count; // �������ֵ�޸�����
                    flatField_.at<ushort>(y, x) = sumFlatField_ / count; // �������ֵ�޸�����
                }
            }
        }
    }

    //3������У��
    flatField_.convertTo(flatField_, CV_32F);
    image_.convertTo(image_, CV_32F);

    Mat flatFieldNormalized, corrected;
    Scalar meanFlatField = mean(flatField_);
    flatFieldNormalized = flatField_ / meanFlatField[0]; // ��һ��ƽ��ͼ��
    divide(image_, flatFieldNormalized, corrected);

    return corrected;
#else
    Mat iamge_, flatField_, corrected;
    Mat ProjImage32Bit, BkgImage32Bit;

    //����У��
    iamge_ = DarkFieldCorrection(image, darkField);
    flatField_ = DarkFieldCorrection(flatField, darkField);

    //����У��
    iamge_ = BadPixelsCorrection(iamge_);
    flatField_ = BadPixelsCorrection(flatField_);

    //ƽ��У��
    ConvertMatImage(iamge_, ProjImage32Bit, CV_32F);
    ConvertMatImage(flatField_, BkgImage32Bit, CV_32F);
    corrected = FlatFieldCorrection(ProjImage32Bit, BkgImage32Bit);

    return corrected;
#endif
}

int RawCorrectionAlg::ReadRawToMat(cv::Mat& image, std::string rawFilePath, int picHeight, int picWidth)
{
    // ���ļ�
    std::ifstream rawFile(rawFilePath, std::ios::binary);
    if (!rawFile) {
        std::cerr << "�޷��� RAW �ļ�: " << rawFilePath << std::endl;
        return -1;
    }

    // ���������ܴ�С
    size_t dataSize = picHeight * picWidth;
    if (dataSize == 0)
    {
        std::cerr << "ͼ�������벻��! " << std::endl;
        rawFile.close();
        return -1;
    }

    // ��ȡ�ļ����ݵ��ڴ�
    vector<uint16_t> data(dataSize);
    rawFile.read(reinterpret_cast<char*>(data.data()), dataSize * sizeof(uint16_t));
    rawFile.close();

    ////����32λ�����ͷŽ�mat��
    //Mat bkg(picHeight, picWidth, CV_32F);
    //for (size_t y = 0; y < picHeight; ++y){
    //    for (size_t x = 0; x < picWidth; ++x){
    //        bkg.at<float>(y, x) = static_cast<float>(data[x + y* picHeight]);
    //    }
    //}

    //����16λ�����ͷŽ�mat��
    Mat bkg(picHeight, picWidth, CV_16U);
    for (size_t y = 0; y < picHeight; ++y) {
        for (size_t x = 0; x < picWidth; ++x) {
            bkg.at<uint16_t>(y, x) = data[x + y * picHeight];
        }
    }

    //qDebug() << bkg.size().height << bkg.size().width << bkg.type();

    image = bkg.clone();
}

void RawCorrectionAlg::ConvertMatImage(const Mat& inputImage, Mat& outputImage, int targetType, double scale, double offset)
{
    // ȷ������ͼ��ǿ�
    if (inputImage.empty()) {
        cerr << "Input image is empty!" << endl;
        return;
    }

    // ת��ͼ����������
    inputImage.convertTo(outputImage, targetType, scale, offset);
}

QImage RawCorrectionAlg::MatToQImage(const cv::Mat& mat)
{
    switch (mat.type()) {
    case CV_8UC1: // �Ҷ�ͼ
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    case CV_8UC3: // BGR ��ʽ
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888)
            .rgbSwapped(); // ת��Ϊ RGB
    case CV_8UC4: // BGRA ��ʽ
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA8888);
    default:
        qWarning("Unsupported cv::Mat format!");
        return QImage();
    }
}

int RawCorrectionAlg::UpdateBadPixelsRaw()
{
    m_bIsExistBadPixelsImage = false;

    // ���ļ�
    std::string rawFilePath = QString("%1/%2").arg(DEFAULT_BADMAP_DIR).arg(DEFAULT_BADMAP_FILE).toStdString();
    std::ifstream rawFile(rawFilePath, std::ios::binary);
    if (!rawFile) {
        std::cerr << "�޷��� RAW �ļ�: " << rawFilePath << std::endl;
        return -1;
    }

    // ���������ܴ�С
    size_t dataSize = DEFAULT_BADMAP_HEIGHT * DEFAULT_BADMAP_WIDTH;
    if (dataSize == 0)
    {
        std::cerr << "ͼ�������벻��! " << std::endl;
        rawFile.close();
        return -1;
    }

    // ��ȡ�ļ����ݵ��ڴ�
    vector<uchar> data(dataSize);
    rawFile.read(reinterpret_cast<char*>(data.data()), dataSize * sizeof(uchar));
    rawFile.close();

    //����8λ�ĸ�ʽ��ȡ
    Mat bkg(DEFAULT_BADMAP_HEIGHT, DEFAULT_BADMAP_WIDTH, CV_8U);
    for (size_t y = 0; y < DEFAULT_BADMAP_HEIGHT; ++y) {
        for (size_t x = 0; x < DEFAULT_BADMAP_WIDTH; ++x) {
            bkg.at<uchar>(y, x) = data[x + y * DEFAULT_BADMAP_HEIGHT];
        }
    }
    m_MatBadPixelsImage = bkg;
    m_bIsExistBadPixelsImage = true;

    qDebug() << bkg.size().height << bkg.size().width << bkg.type();

    return 0;
}
