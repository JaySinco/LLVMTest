#pragma once
#include <QPushButton>

class HexView: public QPushButton
{
    Q_OBJECT

public:
    explicit HexView(QWidget* parent = nullptr);
    ~HexView() override;

    void retranslateUi();
};