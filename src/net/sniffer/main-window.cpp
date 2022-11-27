#include "main-window.h"
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent)
{
    resize(800, 600);
    {
        splitter_v1_ = new QSplitter(Qt::Vertical, this);

        property_view_ = new ProtocolPropView(this);
        QSizePolicy property_view_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        property_view_sp.setVerticalStretch(3);
        property_view_->setSizePolicy(property_view_sp);
        splitter_v1_->addWidget(property_view_);

        hex_view_ = new HexView(this);
        QSizePolicy hex_view_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        hex_view_sp.setVerticalStretch(1);
        hex_view_->setSizePolicy(hex_view_sp);
        splitter_v1_->addWidget(hex_view_);

        int large = QGuiApplication::primaryScreen()->virtualSize().height();
        splitter_v1_->setSizes(QList<int>({large * 3, large}));
    }
    {
        splitter_h1_ = new QSplitter(Qt::Horizontal, this);

        packets_view_ = new PacketsView(this);
        QSizePolicy packets_view_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        packets_view_sp.setHorizontalStretch(1);
        packets_view_->setSizePolicy(packets_view_sp);
        splitter_h1_->addWidget(packets_view_);

        QSizePolicy splitter_v1_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        splitter_v1_sp.setHorizontalStretch(1);
        splitter_v1_->setSizePolicy(splitter_v1_sp);
        splitter_h1_->addWidget(splitter_v1_);

        int large = QGuiApplication::primaryScreen()->virtualSize().width();
        splitter_h1_->setSizes(QList<int>({large, large}));
    }
    {
        QWidget* central = new QWidget(this);
        QVBoxLayout* layout_v1 = new QVBoxLayout(central);

        top_panel_ = new TopPanel(this);
        QSizePolicy top_panel_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        top_panel_sp.setVerticalStretch(1);
        top_panel_->setSizePolicy(top_panel_sp);
        layout_v1->addWidget(top_panel_);

        QSizePolicy splitter_h1_sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
        splitter_h1_sp.setVerticalStretch(10);
        splitter_h1_->setSizePolicy(splitter_h1_sp);
        layout_v1->addWidget(splitter_h1_);

        setCentralWidget(central);
    }

    bottom_panel_ = new BottomPanel(this);
    setStatusBar(bottom_panel_);
    retranslateUi();

    connect(top_panel_, &TopPanel::shouldStartSniff, packets_view_, &PacketsView::start);
    connect(top_panel_, &TopPanel::shouldStopSniff, packets_view_, &PacketsView::stop);
    connect(top_panel_, &TopPanel::shouldClearResult, packets_view_, &PacketsView::clear);
    connect(packets_view_, &PacketsView::stopped, top_panel_, &TopPanel::onSniffStopped);
    connect(packets_view_, &PacketsView::packetSizeChanged, bottom_panel_,
            &BottomPanel::onPacketSizeChanged);
}

MainWindow::~MainWindow() = default;

void MainWindow::retranslateUi() { setWindowTitle(tr("Net Sniffer")); }