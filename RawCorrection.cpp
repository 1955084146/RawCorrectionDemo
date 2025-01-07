#include "RawCorrection.h"
#include <qDebug.h>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>

#include <chrono>

#define ONCE_CORRECTION_TEST 0       //是否只执行一次校正  这里用于检验各个校正是否正确  0 1次  1 多次
#define PIC_SAVE_TEST 0         //是否将各个过程的图片保存下来    用于检验各个步骤图片是否保持异常  0不保存 1保存

#define DEFAULT_HEIGHT 1024     //图片默认高度
#define DEFAULT_WIDTH 1024      //图片默认宽度

#define DEFAULT_AIRMUL 1.0              //空气校正默认系数
#define DEFAULT_DEADPIXLHEIGHT 208      //坏点校正默认高阈值
#define DEFAULT_DEADPIXLLOW     0       //坏点校正默认低阈值

RawCorrection::RawCorrection(QWidget* parent)
    : QMainWindow(parent),
    m_int32Height(DEFAULT_HEIGHT),
    m_int32Width(DEFAULT_WIDTH),
    m_int32DeadPixelHeight(DEFAULT_DEADPIXLHEIGHT),
    m_int32DeadPixelLow(DEFAULT_DEADPIXLLOW),
    m_f64AirMul(DEFAULT_AIRMUL)
{
    ui.setupUi(this);

    // 使用 findChildren 获取所有 QPushButton 类型的子对象
    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) 
    {
        button->setEnabled(false);
    }
    ui.AirMulSpinBox->setEnabled(false);
    ui.DeadPixelHeightSpinBox->setEnabled(false);
    ui.DeadPixelLowSpinBox->setEnabled(false);

    ui.setButton->setEnabled(true);     //只有设置图片大小才能进行校正
    ui.HeightSpinBox->setValue(DEFAULT_HEIGHT);
    ui.WidthSpinBox->setValue(DEFAULT_WIDTH);

    ui.AirMulSpinBox->setValue(DEFAULT_AIRMUL);
    ui.DeadPixelHeightSpinBox->setValue(DEFAULT_DEADPIXLHEIGHT);
    ui.DeadPixelLowSpinBox->setValue(0);

#if 0//PIC_SAVE_TEST
    ui.ShowProjButton->hide();
    ui.ShowDarkButton->hide();
    ui.ShowBkgButton->hide();
    ui.ShowImageButton->hide();
#endif

    connect(ui.setButton, &QPushButton::clicked, this, &RawCorrection::SlotSetImageSizeButtonDown);

    connect(ui.DeadPixelButton, &QPushButton::clicked, this, &RawCorrection::SlotRawCorrectionButtonDown);
    connect(ui.FlatFieldButton, &QPushButton::clicked, this, &RawCorrection::SlotRawCorrectionButtonDown);
    connect(ui.AirButton, &QPushButton::clicked, this, &RawCorrection::SlotRawCorrectionButtonDown);
    connect(ui.DarkFieldButton, &QPushButton::clicked, this, &RawCorrection::SlotRawCorrectionButtonDown);

    connect(ui.AutoCorrectionButton, &QPushButton::clicked, this, &RawCorrection::SlotRawCorrectionButtonDown);

    connect(ui.ImportProjButton, &QPushButton::clicked, this, &RawCorrection::SlotImportButtonDown);
    connect(ui.ImportDarkButton, &QPushButton::clicked, this, &RawCorrection::SlotImportButtonDown);
    connect(ui.ImportBkgButton, &QPushButton::clicked, this, &RawCorrection::SlotImportButtonDown);
    connect(ui.ImportProjDirButton, &QPushButton::clicked, this, &RawCorrection::SlotImportButtonDown);

    connect(ui.ShowProjButton, &QPushButton::clicked, this, &RawCorrection::SlotShowButtonDown);
    connect(ui.ShowDarkButton, &QPushButton::clicked, this, &RawCorrection::SlotShowButtonDown);
    connect(ui.ShowBkgButton, &QPushButton::clicked, this, &RawCorrection::SlotShowButtonDown);
    connect(ui.ShowImageButton, &QPushButton::clicked, this, &RawCorrection::SlotShowButtonDown);

    connect(ui.SaveImageButton, &QPushButton::clicked, this, &RawCorrection::SlotSaveButtonDown);

    connect(ui.AirMulSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, QOverload<double>::of(&RawCorrection::SlotChangeSpinBoxValue));
    connect(ui.DeadPixelHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, QOverload<int>::of(&RawCorrection::SlotChangeSpinBoxValue));
    connect(ui.DeadPixelLowSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, QOverload<int>::of(&RawCorrection::SlotChangeSpinBoxValue));
    connect(ui.PicNumScrollBar, QOverload<int>::of(&QScrollBar::valueChanged), this, QOverload<int>::of(&RawCorrection::SlotChangeScrollBarValue));


    //// 创建坏点图：初始为全黑   人为制造一个坏点图
    //Mat badPixelsImage = Mat::zeros(Size(1024,1024), CV_8UC1);
    //for (int y = 1; y < badPixelsImage.rows - 1; ++y) {
    //    for (int x = 1; x < badPixelsImage.cols - 1; ++x) {
    //        // 判断是否为坏点
    //        if (x%50 == 0 || y%50 == 0) {
    //            badPixelsImage.at<uchar>(y, x) = 255; // 标记坏点
    //        }
    //    }
    //}
    //// 打开文件输出流
    //ofstream file("badMap.raw", ios::out | ios::binary);
    //if (!file.is_open()) {
    //    cerr << "错误：无法打开文件 " << "！" << endl;
    //    return;
    //}
    //// 写入矩阵数据
    //file.write(reinterpret_cast<const char*>(badPixelsImage.data), badPixelsImage.total() * badPixelsImage.elemSize());
    //// 关闭文件
    //file.close();
}

RawCorrection::~RawCorrection()
{}

void RawCorrection::SlotSetImageSizeButtonDown()
{
    m_int32Height = ui.HeightSpinBox->value();
    m_int32Width = ui.WidthSpinBox->value();

    // 使用 findChildren 获取所有 QPushButton 类型的子对象
    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) 
    {
        button->setEnabled(true);
    }
    ui.AirMulSpinBox->setEnabled(true);
    ui.DeadPixelHeightSpinBox->setEnabled(true);
    ui.DeadPixelLowSpinBox->setEnabled(true);

    MatRelease();
}

void RawCorrection::SlotImportButtonDown()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (NULL == btn)
    {
        std::cerr << " btn is null" << std::endl;
        return;
    }

    QString strName;
    if (btn == ui.ImportProjDirButton)
    {
        strName = QFileDialog::getExistingDirectory(
            this,
            "open a file.",
            ".");
        if (strName.isEmpty()) {
            QMessageBox::warning(this, "Warning!", "Failed to open the dir!");
            return;
        }
    }
    else
    {
        strName = QFileDialog::getOpenFileName(
            this,
            "open a file.",
            ".",
            "images(*.raw)");
        if (strName.isEmpty()) {
            QMessageBox::warning(this, "Warning!", "Failed to open the image!");
            return;
        }
    }

    //qDebug() << strName;

    if (btn == ui.ImportProjButton)
    {
        this->m_strProjRawFilePath = strName;
        m_RawCorrectionAlg.ReadRawToMat(m_MatProj, strName.toStdString(), m_int32Height, m_int32Width);
        m_MatTemp = m_MatProj;

        ShowImage(m_MatProj);
        //qDebug() << "m_MatProj.type = " << m_MatProj.type();
#if PIC_SAVE_TEST
        // 打开文件输出流
        ofstream file("proj_32_1024x1024.raw", ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "错误：无法打开文件 " << "！" << endl;
            return;
        }

        // 写入矩阵数据
        file.write(reinterpret_cast<const char*>(m_MatProj.data), m_MatProj.total() * m_MatProj.elemSize());

        // 关闭文件
        file.close();
#endif
    }
    else if (btn == ui.ImportDarkButton)
    {
        this->m_strDarkRawFilePath = strName;
        m_RawCorrectionAlg.ReadRawToMat(m_MatDark, strName.toStdString(), m_int32Height, m_int32Width);
        ShowImage(m_MatDark);

        //qDebug() << "m_MatDark.type = " << m_MatDark.type();
#if PIC_SAVE_TEST
        // 打开文件输出流
        ofstream file("dark_32_1024x1024.raw", ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "错误：无法打开文件 " << "！" << endl;
            return;
        }

        // 写入矩阵数据
        file.write(reinterpret_cast<const char*>(m_MatDark.data), m_MatDark.total() * m_MatDark.elemSize());

        // 关闭文件
        file.close();
#endif
    }
    else if (btn == ui.ImportBkgButton)
    {
        this->m_strBkgRawFilePath = strName;
        m_RawCorrectionAlg.ReadRawToMat(m_MatBkg, strName.toStdString(), m_int32Height, m_int32Width);
        ShowImage(m_MatBkg);

        //qDebug() << "m_MatBkg.type = " << m_MatBkg.type();
#if PIC_SAVE_TEST
        // 打开文件输出流
        ofstream file("bkg_32_1024x1024.raw", ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "错误：无法打开文件 " << "！" << endl;
            return;
        }

        // 写入矩阵数据
        file.write(reinterpret_cast<const char*>(m_MatBkg.data), m_MatBkg.total() * m_MatBkg.elemSize());

        // 关闭文件
        file.close();
#endif
    }
    else if (btn == ui.ImportProjDirButton)
    {
        m_strListProjPaths.clear();
        QDir dir(strName);
        QStringList filters = { "*.raw" };
        QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

        for (const QFileInfo& fileInfo : fileList) {
            m_strListProjPaths.append(fileInfo.absoluteFilePath());
        }
        ui.PicNumScrollBar->setMaximum(m_strListProjPaths.size());
        ui.PicNumLabel->setText(QString("%1/%2").arg(1).arg(ui.PicNumScrollBar->maximum()));

        Mat matShow;
        m_RawCorrectionAlg.ReadRawToMat(matShow, m_strListProjPaths.value(0).toStdString(), m_int32Height, m_int32Width);
        ShowImage(matShow);
    }
    else
    {
        qDebug() << "No Button";
        return;
    }
}

void RawCorrection::SlotShowButtonDown()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (NULL == btn)
    {
        std::cerr << " btn is null" << std::endl;
        return;
    }

    if (btn == ui.ShowProjButton)
    {
        ShowImage(m_MatProj);
    }
    else if (btn == ui.ShowDarkButton)
    {
        ShowImage(m_MatDark);
    }
    else if (btn == ui.ShowBkgButton)
    {
        ShowImage(m_MatBkg);
    }
    else if (btn == ui.ShowImageButton)
    {
        ShowImage(m_MatTemp);
    }
    else
    {
        qDebug() << "No Button";
        return;
    }
}

void RawCorrection::SlotRawCorrectionButtonDown()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (NULL == btn)
    {
        std::cerr << " btn is null" << std::endl;
        return;
    }

    if (m_MatProj.empty())      //没有导入原图，不能进行校正
    {
        QMessageBox::information(this, "Image Error!", "Please import the original image!");
        return;
    }

    if (btn == ui.AirButton)
    {
        Mat CorrectionImage;
#if ONCE_CORRECTION_TEST
        ConvertMatImage(m_MatTemp, CorrectionImage, CV_32F);    //将图片转为18位，确保校正计算精度不丢失
        m_MatTemp = AirCorrection(CorrectionImage, m_f64AirMul);
#else
        auto start = std::chrono::high_resolution_clock::now(); // 开始计时

        m_RawCorrectionAlg.ConvertMatImage(m_MatProj, CorrectionImage, CV_32F);    //将图片转为18位，确保校正计算精度不丢失
        m_MatTemp = m_RawCorrectionAlg.AirCorrection(CorrectionImage, m_f64AirMul);

        auto end = std::chrono::high_resolution_clock::now();   // 结束计时
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        qDebug() << "Air Execution time: " << duration.count() << " microseconds";

#endif

#if PIC_SAVE_TEST
        // 打开文件输出流
        ofstream file("air_32_1024x1024.raw", ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "错误：无法打开文件 " << "！" << endl;
            return;
        }

        // 写入矩阵数据
        file.write(reinterpret_cast<const char*>(m_MatTemp.data), m_MatTemp.total() * m_MatTemp.elemSize());

        // 关闭文件
        file.close();
#endif
    }
    else if (btn == ui.DarkFieldButton)
    {
        if (m_MatDark.empty())      //没有导入暗场图，不能进行校正
        {
            QMessageBox::information(this, "Image Error!", "Please import the dark Image!");
            return;
        }
#if ONCE_CORRECTION_TEST
        Mat CorrectionImage;
        m_RawCorrectionAlg.ConvertMatImage(m_MatTemp, CorrectionImage, CV_16U);        //确保位数一致
        m_MatTemp = DarkFieldCorrection(CorrectionImage, m_MatDark);
#else
        auto start = std::chrono::high_resolution_clock::now(); // 开始计时

        m_MatTemp = m_RawCorrectionAlg.DarkFieldCorrection(m_MatProj, m_MatDark);

        auto end = std::chrono::high_resolution_clock::now();   // 结束计时
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        qDebug() << "Dark Execution time: " << duration.count() << " microseconds";
#endif

#if PIC_SAVE_TEST
        // 打开文件输出流
        ofstream file("dark_32_1024x1024.raw", ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "错误：无法打开文件 " << "！" << endl;
            return;
        }

        // 写入矩阵数据
        file.write(reinterpret_cast<const char*>(m_MatTemp.data), m_MatTemp.total() * m_MatTemp.elemSize());

        // 关闭文件
        file.close();
#endif
    }
    else if (btn == ui.DeadPixelButton)
    {
        //Mat badPixelsImage;
#if ONCE_CORRECTION_TEST
        Mat CorrectionImage;
        m_RawCorrectionAlg.ConvertMatImage(m_MatTemp, CorrectionImage, CV_16U);                   //确保位数一致
        m_MatTemp = BadPixelsCorrection(m_MatTemp, badPixelsImage);     //坏点阈值<m_int32DeadPixelLow  >m_int32DeadPixelHeight 
#else
        auto start = std::chrono::high_resolution_clock::now(); // 开始计时

        m_MatTemp = m_RawCorrectionAlg.BadPixelsCorrection(m_MatProj);     //坏点阈值<m_int32DeadPixelLow  >m_int32DeadPixelHeight

        auto end = std::chrono::high_resolution_clock::now();   // 结束计时
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        qDebug() << "Bad Execution time: " << duration.count() << " microseconds";
#endif

        //// 保存坏点图
        //ofstream badPixelsFile("badPixelsImage_1024x1024.raw", ios::out | ios::binary);
        //if (!badPixelsFile.is_open()) {
        //    cerr << "错误：无法打开文件 " << "！" << endl;
        //    return;
        //}
        //badPixelsFile.write(reinterpret_cast<const char*>(badPixelsImage.data), badPixelsImage.total() * badPixelsImage.elemSize());
        //badPixelsFile.close();

#if PIC_SAVE_TEST
        // 打开文件输出流
        ofstream file("badcorrectionImage_1024x1024.raw", ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "错误：无法打开文件 " << "！" << endl;
            return;
        }

        // 写入矩阵数据
        file.write(reinterpret_cast<const char*>(correctionImage8Bit.data), correctionImage8Bit.total() * correctionImage8Bit.elemSize());

        // 关闭文件
        file.close();
#endif
    }
    else if (btn == ui.FlatFieldButton)
    {
        if (m_MatDark.empty() || m_MatBkg.empty())      //没有导入暗场图和背景图，不能进行校正
        {
            QMessageBox::information(this, "Image Error!", "Please import the dark Image or background Image!");
            return;
        }
        auto start = std::chrono::high_resolution_clock::now(); // 开始计时

        Mat ProjImage32Bit, DarkImage32Bit, BkgImage32Bit;
        m_RawCorrectionAlg.ConvertMatImage(m_MatDark, DarkImage32Bit, CV_32F);                 //将图片转为18位，确保校正计算精度不丢失
        m_RawCorrectionAlg.ConvertMatImage(m_MatBkg, BkgImage32Bit, CV_32F);                   //将图片转为18位，确保校正计算精度不丢失
#if ONCE_CORRECTION_TEST
        m_RawCorrectionAlg.ConvertMatImage(m_MatTemp, ProjImage32Bit, CV_32F);                 //将图片转为18位，确保校正计算精度不丢失
        m_MatTemp = FlatFieldCorrection(ProjImage32Bit, BkgImage32Bit);
#else
        m_RawCorrectionAlg.ConvertMatImage(m_MatProj, ProjImage32Bit, CV_32F);                 //将图片转为18位，确保校正计算精度不丢失
        m_MatTemp = m_RawCorrectionAlg.FlatFieldCorrection(ProjImage32Bit, BkgImage32Bit);

        auto end = std::chrono::high_resolution_clock::now();   // 结束计时
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        qDebug() << "Flat Execution time: " << duration.count() << " microseconds";
#endif

#if PIC_SAVE_TEST
        // 打开文件输出流
        ofstream file("flat_32_1024x1024.raw", ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "错误：无法打开文件 " << "！" << endl;
            return;
        }

        // 写入矩阵数据
        file.write(reinterpret_cast<const char*>(m_MatTemp.data), m_MatTemp.total() * m_MatTemp.elemSize());

        // 关闭文件
        file.close();
#endif
    }
    else if (btn == ui.AutoCorrectionButton)
    {
        if (m_MatDark.empty() || m_MatBkg.empty())      //没有导入暗场图和背景图，不能进行校正
        {
            QMessageBox::information(this, "Image Error!", "Please import the dark Image or background Image!");
            return;
        }
        auto start = std::chrono::high_resolution_clock::now(); // 开始计时

        m_MatTemp = m_RawCorrectionAlg.AutoImageCorrection(m_MatProj, m_MatDark, m_MatBkg);
        auto end = std::chrono::high_resolution_clock::now();   // 结束计时
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        qDebug() << "Flat Execution time: " << duration.count() << " microseconds";
    }
    else
    {
        qDebug() << "No Button";
        return;
    }

    ShowImage(m_MatTemp);
}

void RawCorrection::SlotSaveButtonDown()
{
    qDebug() << "m_MatTemp.type = " << m_MatTemp.type();

    // 打开文件输出流
    ofstream file("output.raw", ios::out | ios::binary);
    if (!file.is_open()) {
        cerr << "错误：无法打开文件 " << "！" << endl;
        return;
    }

    // 写入矩阵数据
    file.write(reinterpret_cast<const char*>(m_MatTemp.data), m_MatTemp.total() * m_MatTemp.elemSize());

    // 关闭文件
    file.close();

    QMessageBox::information(this, "Success!", "Save Success!");
}

void RawCorrection::SlotChangeSpinBoxValue(int value)
{
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(sender());
    if (NULL == spinBox)
    {
        std::cerr << " SpinBox is null" << std::endl;
        return;
    }
    qDebug() << value;

    if (spinBox == ui.DeadPixelHeightSpinBox)
    {
        m_int32DeadPixelHeight = value;
    }
    else if (spinBox == ui.DeadPixelLowSpinBox)
    {
        m_int32DeadPixelLow = value;
    }
    else
    {
        qDebug() << "No SpinBox";
        return;
    }
}

void RawCorrection::SlotChangeSpinBoxValue(double value)
{
    QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(sender());
    if (NULL == spinBox)
    {
        std::cerr << " SpinBox is null" << std::endl;
        return;
    }
    qDebug() << value;

    if (spinBox == ui.AirMulSpinBox)
    {
        m_f64AirMul = value;
    }
    else
    {
        qDebug() << "No SpinBox";
        return;
    }
}

void RawCorrection::SlotChangeScrollBarValue(int value)
{
    int PicNumber = value - 1;
    ui.PicNumLabel->setText(QString("%1/%2  %3").arg(value).arg(ui.PicNumScrollBar->maximum()).arg(m_strListProjPaths.value(PicNumber).section('/', -1)));

    Mat matShow;
    m_RawCorrectionAlg.ReadRawToMat(matShow, m_strListProjPaths.value(PicNumber).toStdString(), m_int32Height, m_int32Width);
    matShow = m_RawCorrectionAlg.AutoImageCorrection(matShow, m_MatDark, m_MatBkg);
    ShowImage(matShow);
}

void RawCorrection::MatRelease()
{
    m_MatProj.release();
    m_MatDark.release();
    m_MatBkg.release();
    m_MatTemp.release();
}

void RawCorrection::ShowImage(const cv::Mat& image)
{
    Mat showMat;
    //归一化为 8 位以便显示
    double minVal = 0;
    double maxVal = 0;
    cv::minMaxLoc(image, &minVal, &maxVal);
    m_RawCorrectionAlg.ConvertMatImage(image, showMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));

    //将8位图显示在label上面    后面需要使用线程+绘制事件来显示
    ui.PicLabel->clear();
    QPixmap pixmap = QPixmap::fromImage(m_RawCorrectionAlg.MatToQImage(showMat));
    QPixmap fitpixmap = pixmap.scaled(ui.PicLabel->width(), ui.PicLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
    ui.PicLabel->setPixmap(pixmap);

}
