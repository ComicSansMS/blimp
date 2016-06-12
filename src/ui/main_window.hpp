#ifndef BLIMP_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP
#define BLIMP_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP

#include <ui/filesystem_model.hpp>

#include <QMainWindow>

#include <QBoxLayout>
#include <QPushButton>
#include <QTreeView>
#include <QWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

    ~MainWindow();

public slots:
    void onButtonClicked();

private:
    QWidget* m_central;
    QTreeView* m_treeview;
    FileSystemModel* m_model;
    QBoxLayout* m_layout;
    QPushButton* m_okButton;
};

#endif
