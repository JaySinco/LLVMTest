#pragma once
#include "type.h"
#include <QWidget>
#include <QPushButton>
#include <QComboBox>

class TopPanel: public QWidget
{
    Q_OBJECT

public:
    explicit TopPanel(QWidget* parent = nullptr);
    ~TopPanel() override;

    void retranslateUi();

public slots:
    void onSniffStopped();

signals:
    void shouldStartSniff(net::Ip4 hint);
    void shouldStopSniff();
    void shouldClearResult();

private:
    QPushButton* start_;
    QPushButton* stop_;
    QPushButton* clear_;
    QComboBox* ip4_drop_down_;
    std::vector<net::Adaptor> all_apts_;
};