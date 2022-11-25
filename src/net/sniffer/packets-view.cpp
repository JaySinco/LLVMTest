#include "packets-view.h"
#include "type.h"

PacketsView::PacketsView(QWidget* parent): QTableView(parent)
{
    setShowGrid(true);
    retranslateUi();

    model_ = new PacketsModel(this);
    setModel(model_);
    acquirer_ = new PacketsAcquirer(net::Ip4::kNull, this);
    connect(acquirer_, &PacketsAcquirer::packetReceived, model_, &PacketsModel::receivePacket);
    acquirer_->start();
}

PacketsView::~PacketsView() = default;

void PacketsView::retranslateUi() {}