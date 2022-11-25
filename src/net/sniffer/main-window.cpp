#include "main-window.h"
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget* parent): QWidget(parent)
{
    resize(800, 600);
    {
        splitter_v2_ = new QSplitter(Qt::Vertical, this);

        property_view_ = new ProtocolPropView(splitter_v2_);
        QSizePolicy property_view_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        property_view_sp.setVerticalStretch(3);
        property_view_->setSizePolicy(property_view_sp);
        splitter_v2_->addWidget(property_view_);

        hex_view_ = new HexView(splitter_v2_);
        QSizePolicy hex_view_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        hex_view_sp.setVerticalStretch(1);
        hex_view_->setSizePolicy(hex_view_sp);
        splitter_v2_->addWidget(hex_view_);
    }
    {
        splitter_h1_ = new QSplitter(Qt::Horizontal, this);

        packets_view_ = new PacketsView(splitter_h1_);
        QSizePolicy packets_view_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        packets_view_sp.setHorizontalStretch(1);
        packets_view_->setSizePolicy(packets_view_sp);
        splitter_h1_->addWidget(packets_view_);

        QSizePolicy splitter_v2_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        splitter_v2_sp.setHorizontalStretch(1);
        splitter_v2_->setSizePolicy(splitter_v2_sp);
        splitter_h1_->addWidget(splitter_v2_);
    }
    {
        splitter_v1_ = new QSplitter(Qt::Vertical, this);

        top_panel_ = new TopPanel(splitter_v1_);
        QSizePolicy top_panel_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        top_panel_sp.setVerticalStretch(1);
        top_panel_->setSizePolicy(top_panel_sp);
        splitter_v1_->addWidget(top_panel_);

        QSizePolicy splitter_h1_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        splitter_h1_sp.setVerticalStretch(5);
        splitter_h1_->setSizePolicy(splitter_h1_sp);

        splitter_v1_->addWidget(splitter_h1_);
    }

    QHBoxLayout* layout_h1 = new QHBoxLayout(this);
    layout_h1->addWidget(splitter_v1_);

    retranslateUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::retranslateUi() { setWindowTitle(tr("Net Sniffer")); }