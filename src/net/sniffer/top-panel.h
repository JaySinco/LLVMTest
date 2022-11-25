#pragma once
#include <QPushButton>

class TopPanel: public QPushButton
{
    Q_OBJECT

public:
    explicit TopPanel(QWidget* parent = nullptr);
    ~TopPanel() override;

    void retranslateUi();
};