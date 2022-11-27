#include "packets-model.h"
#include "ethernet.h"
#include "ipv4.h"
#include "arp.h"
#include <QFont>

PacketsModel::PacketsModel(QObject* parent): QAbstractTableModel(parent) {}

PacketsModel::~PacketsModel() = default;

int PacketsModel::rowCount(QModelIndex const& parent) const { return data_.size(); }

int PacketsModel::columnCount(QModelIndex const& parent) const { return kTOTAL; }

void PacketsModel::receivePacket(std::vector<net::Packet> const& pacs)
{
    beginInsertRows(QModelIndex(), data_.size(), data_.size() + pacs.size() - 1);
    for (auto& pac: pacs) {
        data_.push_back(ModelData{pac, net::ProtocolStack::decode(pac)});
    }
    endInsertRows();
    emit packetSizeChanged(data_.size());
}

void PacketsModel::clear()
{
    beginResetModel();
    data_.clear();
    endResetModel();
    emit packetSizeChanged(data_.size());
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
        case kTIME:
            return tr("time");
        case kDIP:
            return tr("dip");
        case kSIP:
            return tr("sip");
        case kTYPE:
            return tr("type");
    }
    return {};
}

Qt::ItemFlags PacketsModel::flags(QModelIndex const& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PacketsModel::data(QModelIndex const& index, int role) const
{
    if (role == Qt::FontRole) {
        QFont font;
        font.setPointSize(9);
        return font;
    }
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignLeft;
    }
    if (role == Qt::DisplayRole) {
        auto& stack = data_.at(index.row()).stack;
        int col = index.column();
        if (col == kTIME) {
            return QString::fromStdString(stack.getTimeStr());
        }
        if (col == kTYPE) {
            return QString::fromStdString(net::Protocol::typeDesc(stack.innerMost()));
        }
        if (stack.has(net::Protocol::kIPv4)) {
            auto& ip4 = stack.get<net::Ipv4>(net::Protocol::kIPv4);
            if (col == kDIP) {
                return QString::fromStdString(ip4.dip().toStr());
            }
            if (col == kSIP) {
                return QString::fromStdString(ip4.sip().toStr());
            }
        }
        if (stack.has(net::Protocol::kARP)) {
            auto& arp = stack.get<net::Arp>(net::Protocol::kARP);
            if (col == kDIP) {
                return QString::fromStdString(arp.dip().toStr());
            }
            if (col == kSIP) {
                return QString::fromStdString(arp.sip().toStr());
            }
        }
        if (stack.has(net::Protocol::kRARP)) {
            auto& rarp = stack.get<net::Arp>(net::Protocol::kRARP);
            if (col == kDIP) {
                return QString::fromStdString(rarp.dip().toStr());
            }
            if (col == kSIP) {
                return QString::fromStdString(rarp.sip().toStr());
            }
        }
    }
    return {};
}
