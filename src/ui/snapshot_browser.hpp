#ifndef BLIMP_INCLUDE_GUARD_UI_SNAPSHOT_BROWSER_HPP
#define BLIMP_INCLUDE_GUARD_UI_SNAPSHOT_BROWSER_HPP

#include <QWidget>

class BlimpDB;
class SnapshotContentsModel;

class QBoxLayout;
class QComboBox;
class QTreeView;

class SnapshotBrowser : public QWidget {
    Q_OBJECT
private:
    QBoxLayout* m_layout;
    QComboBox* m_comboSnapshotList;
    SnapshotContentsModel* m_model;
    QTreeView* m_treeView;
public:
    SnapshotBrowser(QWidget* parent);

    void setData(BlimpDB& blimpdb);
};

#endif
