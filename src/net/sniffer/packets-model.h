#pragma once
#include "type.h"
#include "protocol.h"
#include <QAbstractTableModel>

class PacketsModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PacketsModel(QObject* parent = nullptr);
    ~PacketsModel() override;

    int rowCount(QModelIndex const& parent) const override;
    int columnCount(QModelIndex const& parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(QModelIndex const& index, int role) const override;

    void receivePacket(net::Packet const& pac);

private:
    std::vector<net::ProtocolStack> data_;
};