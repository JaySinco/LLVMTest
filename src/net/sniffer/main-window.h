#pragma once
#include "top-panel.h"
#include "bottom-panel.h"
#include "packets-view.h"
#include "protocol-prop-view.h"
#include "hex-view.h"
#include <QMainWindow>
#include <QSplitter>

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void retranslateUi();

private:
    QSplitter* splitter_v1_;
    QSplitter* splitter_h1_;
    TopPanel* top_panel_;
    BottomPanel* bottom_panel_;
    PacketsView* packets_view_;
    ProtocolPropView* property_view_;
    HexView* hex_view_;
};