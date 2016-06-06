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
            if(index.row() % 3 == 0) {
                return Qt::Checked;
            } else if(index.row() % 3 == 1) {
                return Qt::PartiallyChecked;
            } else {
                return Qt::Unchecked;
            }
        }
    }
    return QFileSystemModel::data(index, role);
}
