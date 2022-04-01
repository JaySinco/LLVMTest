#pragma once
#include "ui_go-to-cell-dialog.h"
#include <QtWidgets/QDialog>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class GoToCellDialog: public QDialog, public Ui::GoToCellDialog
{
    Q_OBJECT

public:
    GoToCellDialog(QWidget* parent = nullptr);

private slots:
    void on_lineEdit_textChanged();
};
