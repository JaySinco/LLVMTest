#include "packets-view.h"
#include "type.h"
#include <QHeaderView>

PacketsView::PacketsView(QWidget* parent): QTableView(parent)
{
    setShowGrid(true);

    auto hh = horizontalHeader();
    hh->setVisible(true);
    hh->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    hh->setHighlightSections(false);

    auto vh = verticalHeader();
    vh->setSectionResizeMode(QHeaderView::Fixed);
    vh->setDefaultSectionSize(10);

    setSelectionBehavior(SelectionBehavior::SelectRows);
    retranslateUi();

    model_ = new PacketsModel(this);
    setModel(model_);
    connect(model_, &PacketsModel::packetSizeChanged, this, &PacketsView::packetSizeChanged);
}

PacketsView::~PacketsView() = default;

void PacketsView::retranslateUi() {}

void PacketsView::start(net::Ip4 hint)
{
    if (acquirer_ != nullptr) {
        MY_THROW("a acquirer already running, stop it first");
    }
    acquirer_ = new PacketsAcquirer(hint, 500, this);
    connect(acquirer_, &PacketsAcquirer::packetReceived, model_, &PacketsModel::receivePacket);
    connect(acquirer_, &PacketsAcquirer::stopped, this, &PacketsView::stopped);
    acquirer_->start();
}

void PacketsView::stop()
{
    delete acquirer_;
    acquirer_ = nullptr;
}

void PacketsView::clear() { model_->clear(); }