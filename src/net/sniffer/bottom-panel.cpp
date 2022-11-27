#include "bottom-panel.h"
#include "utils/logging.h"

BottomPanel::BottomPanel(QWidget* parent): QStatusBar(parent)
{
    status_left_ = new QLabel("", this);
    status_left_->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    status_right_ = new QLabel("", this);
    status_right_->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    addPermanentWidget(status_left_, 1);
    addPermanentWidget(status_right_, 1);
    setSizeGripEnabled(false);

    onPacketSizeChanged(0);

    retranslateUi();
}

BottomPanel::~BottomPanel() = default;

void BottomPanel::retranslateUi() {}

void BottomPanel::onPacketSizeChanged(size_t n)
{
    status_left_->setText(QString::fromStdString(FSTR("Total: {}", n)));
}
