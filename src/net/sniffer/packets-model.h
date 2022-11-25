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
    Qt::ItemFlags flags(QModelIndex const& index) const override;

    void receivePacket(std::vector<net::Packet> const& pacs);

private:
    enum ColType : int
    {
        kTIME,
        kTYPE,
        kDIP,
        kSIP,
        kTOTAL,
    };

    struct ModelData
    {
        net::Packet pac;
        net::ProtocolStack stack;
    };

    std::vector<ModelData> data_;
};