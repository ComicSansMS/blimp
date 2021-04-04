#ifndef BLIMP_INCLUDE_GUARD_UI_SNAPSHOT_CONTENTS_MODEL_HPP
#define BLIMP_INCLUDE_GUARD_UI_SNAPSHOT_CONTENTS_MODEL_HPP

#include <file_info.hpp>

#include <QAbstractItemModel>

#include <boost/filesystem/path.hpp>

#include <optional>
#include <vector>

class SnapshotContentsModel : public QAbstractItemModel {
    Q_OBJECT
public:
    struct SnapshotContentItem {
        QString path;
        std::optional<FileInfo> finfo;
        std::vector<std::size_t> children;
        std::size_t index;
        std::size_t parent_index;
        int row;
    };
private:
    mutable std::vector<SnapshotContentItem> m_items;
public:
    SnapshotContentsModel(QObject* parent);

    /** @name Implementation of QAbstractItemModel
    * @{
    */
    Qt::ItemFlags flags(QModelIndex const& index) const override;
    QModelIndex index(int row, int column, QModelIndex const& parent) const override;
    QModelIndex parent(QModelIndex const& index) const override;
    int rowCount(QModelIndex const& index) const override;
    int columnCount(QModelIndex const& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(QModelIndex const& index, int role) const override;
    //bool setData(QModelIndex const& index, QVariant const& value, int role) override;
    //! @}

    void clear();
    SnapshotContentItem& newItem(SnapshotContentItem& parent, QString const& p);
    SnapshotContentItem& getItemByIndex(std::size_t index);
    void addItem(FileInfo const finfo);
    void finalize();
};

#endif
