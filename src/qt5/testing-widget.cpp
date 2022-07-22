#include "./testing-widget.h"
#include "ui_testing-widget.h"
#include <QtWidgets/QFileSystemModel>

TestingWidget::TestingWidget(QWidget* parent): QWidget(parent), ui(new Ui::TestingWidget)
{
    ui->setupUi(this);
    auto model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());
    ui->listView->setModel(model);
    ui->listView->setRootIndex(model->index(QDir::currentPath()));
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(QDir::currentPath()));
}

TestingWidget::~TestingWidget() { delete ui; }
