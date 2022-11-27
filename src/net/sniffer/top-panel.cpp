#include "top-panel.h"
#include "platform.h"
#include "driver.h"
#include <QHBoxLayout>

TopPanel::TopPanel(QWidget* parent): QWidget(parent), all_apts_(net::allAdaptors())
{
    ip4_drop_down_ = new QComboBox(this);
    for (auto& apt: all_apts_) {
        ip4_drop_down_->addItem(QString::fromStdString(FSTR("{} ({})", apt.ip.toStr(), apt.desc)));
    }
    ip4_drop_down_->setCurrentIndex(net::Driver::selectAdaptorIndex(net::Ip4::kNull));
    start_ = new QPushButton(this);
    stop_ = new QPushButton(this);
    stop_->setEnabled(false);
    clear_ = new QPushButton(this);

    QHBoxLayout* layout_h1 = new QHBoxLayout(this);
    layout_h1->addWidget(ip4_drop_down_);
    layout_h1->addWidget(start_);
    layout_h1->addWidget(stop_);
    layout_h1->addWidget(clear_);

    retranslateUi();

    connect(start_, &QPushButton::clicked, [&]() {
        start_->setEnabled(false);
        auto ip4 = all_apts_.at(this->ip4_drop_down_->currentIndex()).ip;
        emit this->shouldStartSniff(ip4);
        stop_->setEnabled(true);
        ip4_drop_down_->setEnabled(false);
    });
    connect(stop_, &QPushButton::clicked, [&]() {
        stop_->setEnabled(false);
        emit this->shouldStopSniff();
    });
    connect(clear_, &QPushButton::clicked, this, &TopPanel::shouldClearResult);
}

TopPanel::~TopPanel() = default;

void TopPanel::retranslateUi()
{
    start_->setText(tr("Start"));
    stop_->setText(tr("Stop"));
    clear_->setText(tr("Clear"));
}

void TopPanel::onSniffStopped()
{
    start_->setEnabled(true);
    ip4_drop_down_->setEnabled(true);
}