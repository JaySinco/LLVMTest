#pragma once
#include <QtWidgets/QDialog>

namespace Ui
{
class GoToCellDialog;
}

class GoToCellDialog: public QDialog
{
    Q_OBJECT

public:
    explicit GoToCellDialog(QWidget* parent = nullptr);
    ~GoToCellDialog();

private slots:
    void on_lineEdit_textChanged();

private:
    Ui::GoToCellDialog* ui;
};
