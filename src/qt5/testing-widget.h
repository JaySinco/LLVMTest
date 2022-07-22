#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
class TestingWidget;
}

class TestingWidget: public QWidget
{
    Q_OBJECT

public:
    explicit TestingWidget(QWidget* parent = nullptr);
    ~TestingWidget();

private:
    Ui::TestingWidget* ui;
};
