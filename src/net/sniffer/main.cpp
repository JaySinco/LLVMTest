#include "utils/logging.h"
#include "main-window.h"
#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    MY_TRY;
    utils::initLogger(argv[0]);
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
    MY_CATCH;
}
