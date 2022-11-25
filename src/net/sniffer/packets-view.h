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

private:
    PacketsModel* model_;
    PacketsAcquirer* acquirer_;
};