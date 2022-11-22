#include "sniffer-main-window.h"
#include <QtWidgets/QFileSystemModel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListView>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>

MainWindow::MainWindow(QWidget* parent): QWidget(parent)
{
    resize(499, 319);
    hor_layout_ = new QHBoxLayout(this);

    splitter_ = new QSplitter(this);
    splitter_->setOrientation(Qt::Horizontal);

    tree_view_ = new QTreeView(splitter_);
    QSizePolicy tree_view_size_plc(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tree_view_size_plc.setHorizontalStretch(2);
    tree_view_size_plc.setVerticalStretch(0);
    tree_view_->setSizePolicy(tree_view_size_plc);
    splitter_->addWidget(tree_view_);

    list_view_ = new QListView(splitter_);
    QSizePolicy list_view_size_plc(QSizePolicy::Ignored, QSizePolicy::Expanding);
    list_view_size_plc.setHorizontalStretch(1);
    list_view_size_plc.setVerticalStretch(0);
    list_view_->setSizePolicy(list_view_size_plc);
    splitter_->addWidget(list_view_);

    hor_layout_->addWidget(splitter_);

    auto model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());
    list_view_->setModel(model);
    list_view_->setRootIndex(model->index(QDir::currentPath()));
    tree_view_->setModel(model);
    tree_view_->setRootIndex(model->index(QDir::currentPath()));

    retranslateUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::retranslateUi() { setWindowTitle(tr("Net Sniffer")); }