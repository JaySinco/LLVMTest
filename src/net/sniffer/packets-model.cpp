#include "packets-model.h"
#include "ethernet.h"
#include <QFont>

PacketsModel::PacketsModel(QObject* parent): QAbstractTableModel(parent) {}

PacketsModel::~PacketsModel() = default;

int PacketsModel::rowCount(QModelIndex const& parent) const { return data_.size(); }

int PacketsModel::columnCount(QModelIndex const& parent) const { return 3; }

void PacketsModel::receivePacket(net::Packet const& pac)
{
    beginInsertRows(QModelIndex(), data_.size(), data_.size());
    data_.push_back(net::ProtocolStack::decode(pac));
    endInsertRows();
}

QVariant PacketsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return {};
    }
    if (role != Qt::DisplayRole) {
        return {};
    }
    switch (section) {
        case 0:
            return tr("time");
        case 1:
            return tr("dmac");
        case 2:
            return tr("smac");
    }
    return {};
}

QVariant PacketsModel::data(QModelIndex const& index, int role) const
{
    switch (index.column()) {
        case 0:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return QString::fromStdString(data_.at(index.row()).getTimeStr());
                case Qt::TextAlignmentRole:
                    return Qt::AlignLeft;
                case Qt::FontRole: {
                    QFont fnt;
                    fnt.setPointSize(9);
                    return fnt;
                }
                case Qt::ToolTipRole:
                    return {};
            }

        case 1:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return QString::fromStdString(data_.at(index.row())
                                                      .get<net::Ethernet>(net::Protocol::kEthernet)
                                                      .dmac()
                                                      .toStr());
                case Qt::TextAlignmentRole:
                    return Qt::AlignLeft;
                case Qt::FontRole: {
                    QFont fnt;
                    fnt.setPointSize(9);
                    return fnt;
                }
            }

        case 2:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return QString::fromStdString(data_.at(index.row())
                                                      .get<net::Ethernet>(net::Protocol::kEthernet)
                                                      .smac()
                                                      .toStr());
                case Qt::TextAlignmentRole:
                    return Qt::AlignLeft;
                case Qt::FontRole: {
                    QFont fnt;
                    fnt.setPointSize(9);
                    return fnt;
                }
            }
    }
    return {};
}