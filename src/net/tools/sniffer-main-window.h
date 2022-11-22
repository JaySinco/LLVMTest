#pragma once
#include <QtWidgets/QWidget>

class QHBoxLayout;
class QSplitter;
class QTreeView;
class QListView;

class MainWindow: public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void retranslateUi();

private:
    QHBoxLayout* hor_layout_;
    QSplitter* splitter_;
    QTreeView* tree_view_;
    QListView* list_view_;
};