#include "RawCorrection.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RawCorrection w;
    w.show();
    return a.exec();
}
