#include "RawCorrectionAlg.h"
#include <qDebug>

RawCorrectionAlg::RawCorrectionAlg():
    m_bIsExistBadPixelsImage(false)
{
    //导入坏点图
    UpdateBadPixelsRaw();
}

RawCorrectionAlg::~RawCorrectionAlg()
{

}

Mat RawCorrectionAlg::DarkFieldCorrection(const Mat& image, const Mat& darkField)
{
    Mat corrected;
    subtract(image, darkField, corrected);
    corrected = max(corrected, 0); // 防止出现负值
    return corrected;
}

Mat RawCorrectionAlg::FlatFieldCorrection(const Mat& image, const Mat& flatField)
{
    Mat flatFieldNormalized, corrected;
    Scalar meanFlatField = mean(flatField);
    flatFieldNormalized = flatField / meanFlatField[0]; // 归一化平场图像
    divide(image, flatFieldNormalized, corrected);

    return corrected;
}

Mat RawCorrectionAlg::AirCorrection(const Mat& image, double factor)
{
    Mat corrected;
    image.convertTo(corrected, -1, factor, 0); // 增加亮度，factor为比例
    return corrected;
}

Mat RawCorrectionAlg::BadPixelsCorrection(const Mat& image)
{
    //1、判断有无坏点图
    if (!m_bIsExistBadPixelsImage)
    {
        //导入坏点图
        UpdateBadPixelsRaw();
        if (!m_bIsExistBadPixelsImage)
            return Mat(image);
    }

    //2、判断原图和坏点图高宽是否一致
    Mat rawImage = image;
    if (rawImage.size() != m_MatBadPixelsImage.size()) {
        qDebug() << "Error: RAW image and bad pixel map must have the same size.";
        return Mat(image);
    }

    //3、遍历坏点图,校正坏点
    for (int y = 0; y < rawImage.rows; ++y) {
        for (int x = 0; x < rawImage.cols; ++x) {
            if (m_MatBadPixelsImage.at<uchar>(y, x) > 0) { // 坏点
                int count = 0;
                int sum = 0;

                // 遍历 3x3 邻域
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int ny = y + dy;
                        int nx = x + dx;

                        // 确保在图像范围内并且不是坏点
                        if (ny >= 0 && ny < rawImage.rows && nx >= 0 && nx < rawImage.cols &&
                            m_MatBadPixelsImage.at<uchar>(ny, nx) == 0) {
                            sum += rawImage.at<ushort>(ny, nx); // 累加值
                            ++count;
                        }
                    }
                }

                if (count > 0) {
                    rawImage.at<ushort>(y, x) = sum / count; // 用邻域均值修复坏点
                }
            }
        }
    }

    return rawImage;
}

Mat RawCorrectionAlg::AutoImageCorrection(const Mat& image, const Mat& darkField, const Mat& flatField)
{
#if 1
    //1、坏点校正
    Mat image_, flatField_;
    subtract(image, darkField, image_);         //对原图进行一次暗场校正
    image_ = max(image_, 0);
    subtract(flatField, darkField, flatField_); //对背景图进行一次暗场校正
    flatField_ = max(flatField_, 0);

    //2、坏点校正
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
            if (m_MatBadPixelsImage.at<uchar>(y, x) > 0) { // 坏点
                int count = 0;
                int sumImage_ = 0;
                int sumFlatField_ = 0;

                // 遍历 3x3 邻域
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int ny = y + dy;
                        int nx = x + dx;

                        // 确保在图像范围内并且不是坏点
                        if (ny >= 0 && ny < m_MatBadPixelsImage.rows && nx >= 0 && nx < m_MatBadPixelsImage.cols &&
                            m_MatBadPixelsImage.at<uchar>(ny, nx) == 0) {
                            sumImage_ += image_.at<ushort>(ny, nx); // 累加值
                            sumFlatField_ += flatField_.at<ushort>(ny, nx); // 累加值
                            ++count;
                        }
                    }
                }

                if (count > 0) {
                    image_.at<ushort>(y, x) = sumImage_ / count; // 用邻域均值修复坏点
                    flatField_.at<ushort>(y, x) = sumFlatField_ / count; // 用邻域均值修复坏点
                }
            }
        }
    }

    //3、坏点校正
    flatField_.convertTo(flatField_, CV_32F);
    image_.convertTo(image_, CV_32F);

    Mat flatFieldNormalized, corrected;
    Scalar meanFlatField = mean(flatField_);
    flatFieldNormalized = flatField_ / meanFlatField[0]; // 归一化平场图像
    divide(image_, flatFieldNormalized, corrected);

    return corrected;
#else
    Mat iamge_, flatField_, corrected;
    Mat ProjImage32Bit, BkgImage32Bit;

    //暗场校正
    iamge_ = DarkFieldCorrection(image, darkField);
    flatField_ = DarkFieldCorrection(flatField, darkField);

    //坏点校正
    iamge_ = BadPixelsCorrection(iamge_);
    flatField_ = BadPixelsCorrection(flatField_);

    //平场校正
    ConvertMatImage(iamge_, ProjImage32Bit, CV_32F);
    ConvertMatImage(flatField_, BkgImage32Bit, CV_32F);
    corrected = FlatFieldCorrection(ProjImage32Bit, BkgImage32Bit);

    return corrected;
#endif
}

int RawCorrectionAlg::ReadRawToMat(cv::Mat& image, std::string rawFilePath, int picHeight, int picWidth)
{
    // 打开文件
    std::ifstream rawFile(rawFilePath, std::ios::binary);
    if (!rawFile) {
        std::cerr << "无法打开 RAW 文件: " << rawFilePath << std::endl;
        return -1;
    }

    // 计算数据总大小
    size_t dataSize = picHeight * picWidth;
    if (dataSize == 0)
    {
        std::cerr << "图像宽高输入不对! " << std::endl;
        rawFile.close();
        return -1;
    }

    // 读取文件数据到内存
    vector<uint16_t> data(dataSize);
    rawFile.read(reinterpret_cast<char*>(data.data()), dataSize * sizeof(uint16_t));
    rawFile.close();

    ////按照32位浮点型放进mat里
    //Mat bkg(picHeight, picWidth, CV_32F);
    //for (size_t y = 0; y < picHeight; ++y){
    //    for (size_t x = 0; x < picWidth; ++x){
    //        bkg.at<float>(y, x) = static_cast<float>(data[x + y* picHeight]);
    //    }
    //}

    //按照16位浮点型放进mat里
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
    // 确保输入图像非空
    if (inputImage.empty()) {
        cerr << "Input image is empty!" << endl;
        return;
    }

    // 转换图像数据类型
    inputImage.convertTo(outputImage, targetType, scale, offset);
}

QImage RawCorrectionAlg::MatToQImage(const cv::Mat& mat)
{
    switch (mat.type()) {
    case CV_8UC1: // 灰度图
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    case CV_8UC3: // BGR 格式
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888)
            .rgbSwapped(); // 转换为 RGB
    case CV_8UC4: // BGRA 格式
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA8888);
    default:
        qWarning("Unsupported cv::Mat format!");
        return QImage();
    }
}

int RawCorrectionAlg::UpdateBadPixelsRaw()
{
    m_bIsExistBadPixelsImage = false;

    // 打开文件
    std::string rawFilePath = QString("%1/%2").arg(DEFAULT_BADMAP_DIR).arg(DEFAULT_BADMAP_FILE).toStdString();
    std::ifstream rawFile(rawFilePath, std::ios::binary);
    if (!rawFile) {
        std::cerr << "无法打开 RAW 文件: " << rawFilePath << std::endl;
        return -1;
    }

    // 计算数据总大小
    size_t dataSize = DEFAULT_BADMAP_HEIGHT * DEFAULT_BADMAP_WIDTH;
    if (dataSize == 0)
    {
        std::cerr << "图像宽高输入不对! " << std::endl;
        rawFile.close();
        return -1;
    }

    // 读取文件数据到内存
    vector<uchar> data(dataSize);
    rawFile.read(reinterpret_cast<char*>(data.data()), dataSize * sizeof(uchar));
    rawFile.close();

    //按照8位的格式读取
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
