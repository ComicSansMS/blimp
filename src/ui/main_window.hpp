
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

private:
    QWidget* m_central;
    QTreeView* m_treeview;
    FileSystemModel* m_model;
    QBoxLayout* m_layout;
    QPushButton* m_okButton;
};
