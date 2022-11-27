#pragma once
#include "packets-model.h"
#include "packets-acquirer.h"
#include <QTableView>

class PacketsView: public QTableView
{
    Q_OBJECT

public:
    explicit PacketsView(QWidget* parent = nullptr);
    ~PacketsView() override;

    void retranslateUi();

public slots:
    void start(net::Ip4 hint);
    void stop();
    void clear();

signals:
    void stopped();
    void packetSizeChanged(size_t n);

private:
    PacketsModel* model_;
    PacketsAcquirer* acquirer_ = nullptr;
};