#pragma once
#include <QStatusBar>
#include <QLabel>

class BottomPanel: public QStatusBar
{
    Q_OBJECT

public:
    explicit BottomPanel(QWidget* parent = nullptr);
    ~BottomPanel() override;

    void retranslateUi();

public slots:
    void onPacketSizeChanged(size_t n);

private:
    QLabel* status_left_;
    QLabel* status_right_;
};