#include "../utils.h"
#include "./testing-widget.h"
#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    TestingWidget w;
    w.show();
    return app.exec();
}
