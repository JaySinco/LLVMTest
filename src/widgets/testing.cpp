#include "utils/base.h"
#include "testing-widget.h"
#include <QtWidgets/QApplication>
#include <boost/timer/timer.hpp>

int main(int argc, char** argv)
{
    boost::timer::auto_cpu_timer timer;
    QApplication app(argc, argv);
    TestingWidget w;
    w.show();
    return app.exec();
}
