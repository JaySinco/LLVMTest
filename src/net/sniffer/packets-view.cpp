#include "packets-view.h"
#include "type.h"
#include <QHeaderView>

PacketsView::PacketsView(QWidget* parent): QTableView(parent)
{
    setShowGrid(true);

    auto hh = horizontalHeader();
    hh->setVisible(true);
    hh->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    auto vh = verticalHeader();
    vh->setSectionResizeMode(QHeaderView::Fixed);
    vh->setDefaultSectionSize(10);

    retranslateUi();

    model_ = new PacketsModel(this);
    setModel(model_);
    acquirer_ = new PacketsAcquirer(net::Ip4::kNull, 500, this);
    connect(acquirer_, &PacketsAcquirer::packetReceived, model_, &PacketsModel::receivePacket);
    acquirer_->start();
}

PacketsView::~PacketsView() = default;

void PacketsView::retranslateUi() {}