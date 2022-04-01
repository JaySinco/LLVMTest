#include "../utils.h"
#include "./find-dialog.h"
#include "./go-to-cell-dialog.h"
#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    auto* window = new GoToCellDialog;
    window->show();
    return app.exec();
}
