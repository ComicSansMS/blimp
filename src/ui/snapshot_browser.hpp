#ifndef BLIMP_INCLUDE_GUARD_UI_SNAPSHOT_BROWSER_HPP
#define BLIMP_INCLUDE_GUARD_UI_SNAPSHOT_BROWSER_HPP

#include <db/file_element_id.hpp>

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

signals:
    void fileRetrievalRequest(FileElementId);
private slots:
    void onItemDoubleClicked(QModelIndex const& idx);
};

#endif
