#include "../utils.h"
#include "./find-dialog.h"
#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    auto* window = new FindDialog;
    window->show();
    return app.exec();
}
