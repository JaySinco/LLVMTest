#pragma once
#include <QPushButton>

class ProtocolPropView: public QPushButton
{
    Q_OBJECT

public:
    explicit ProtocolPropView(QWidget* parent = nullptr);
    ~ProtocolPropView() override;

    void retranslateUi();
};