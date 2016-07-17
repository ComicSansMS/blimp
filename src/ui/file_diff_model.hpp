#ifndef BLIMP_INCLUDE_GUARD_UI_FILE_DIFF_MODEL_HPP
#define BLIMP_INCLUDE_GUARD_UI_FILE_DIFF_MODEL_HPP

#include <file_info.hpp>

#include <QAbstractItemModel>

#include <string>
#include <unordered_map>
#include <vector>

class FileDiffModel : public QAbstractItemModel {
    Q_OBJECT
public:
    FileDiffModel(QObject* parent);

    void setFileIndexData(std::vector<FileInfo> const& file_index, FileIndexDiff const& file_index_diff);

    Qt::ItemFlags flags(QModelIndex const& index) const override;
    bool hasChildren(QModelIndex const& index) const override;
    QModelIndex index(int row, int column, QModelIndex const& parent) const override;
    QModelIndex parent(QModelIndex const& index) const override;
    int rowCount(QModelIndex const& index) const override;
    int columnCount(QModelIndex const& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(QModelIndex const& index, int role) const override;

private:

private:
    std::vector<FileInfo> m_file_index;
    FileIndexDiff m_file_index_diff;
};

#endif
