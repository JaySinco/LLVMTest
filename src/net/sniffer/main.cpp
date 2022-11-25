#include "utils/args.h"
#include "main-window.h"
#include <QApplication>

int main(int argc, char** argv)
{
    MY_TRY;
    INIT_LOG(argc, argv);
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
    MY_CATCH;
}
