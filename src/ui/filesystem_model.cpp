#include <ui/filesystem_model.hpp>


FileSystemModel::FileSystemModel(QObject* parent)
    :QFileSystemModel(parent)
{}


QVariant FileSystemModel::data(QModelIndex const& index, int role) const
{
    if(role == Qt::CheckStateRole)
    {
        if(index.column() == 0)
        {
            auto it = m_checkMap.find(index);
            if(it != end(m_checkMap)) {
                return it->second;
            } else {
                return Qt::Unchecked;
            }
        }
    } else if(role == Qt::DecorationRole) {
        return QVariant();
    }
    auto const path = QFileSystemModel::filePath(index);
    auto const casted_role = static_cast<Qt::ItemDataRole>(role);
    QVariant const ret = QFileSystemModel::data(index, casted_role);
    return ret;
}

Qt::ItemFlags FileSystemModel::flags(QModelIndex const& index) const
{
    if(index.column() == 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserTristate;
    }
    auto const ret = QFileSystemModel::flags(index);
    return ret;
}

void FileSystemModel::itemClicked(QModelIndex const& index)
{
    if(index.column() == 0) {
        // checkbox clicked
        if(m_checkMap.find(index) == end(m_checkMap)) {
            m_checkMap.insert(std::make_pair(index, Qt::Unchecked));
        }
        auto& element = m_checkMap[index];
        if(element == Qt::Checked) {
            element = Qt::Unchecked;
        } else if (element == Qt::PartiallyChecked) {
            element = Qt::Checked;
        } else {
            element = Qt::PartiallyChecked;
        }
        QVector<int> roles_changed;
        roles_changed.push_back(Qt::CheckStateRole);
        emit dataChanged(index, index, roles_changed);
    }
}
