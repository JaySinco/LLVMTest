#include "../utils.h"
#include "./go-to-cell-dialog.h"
#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    GoToCellDialog w;
    w.show();
    return app.exec();
}
