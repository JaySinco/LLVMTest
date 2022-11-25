#pragma once
#include "top-panel.h"
#include "packets-view.h"
#include "protocol-prop-view.h"
#include "hex-view.h"
#include <QWidget>
#include <QSplitter>

class MainWindow: public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void retranslateUi();

private:
    QSplitter* splitter_v1_;
    QSplitter* splitter_v2_;
    QSplitter* splitter_h1_;
    TopPanel* top_panel_;
    PacketsView* packets_view_;
    ProtocolPropView* property_view_;
    HexView* hex_view_;
};