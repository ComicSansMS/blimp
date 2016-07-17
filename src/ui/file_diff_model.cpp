#include <ui/file_diff_model.hpp>

#include <gbBase/Assert.hpp>

#include <QDateTime>
#include <QLocale>

#include <chrono>

FileDiffModel::FileDiffModel(QObject* parent)
    :QAbstractItemModel(parent)
{
}

void FileDiffModel::setFileIndexData(std::vector<FileInfo> const& file_index, FileIndexDiff const& file_index_diff)
{
    GHULBUS_PRECONDITION(file_index.size() == file_index_diff.index_files.size());
    beginResetModel();
    m_file_index = file_index;
    m_file_index_diff = file_index_diff;
    m_entry_checked.clear();
    m_entry_checked.resize(file_index.size(), true);
    endResetModel();
}

Qt::ItemFlags FileDiffModel::flags(QModelIndex const& index) const
{
    GHULBUS_ASSERT(index.isValid());
    auto const flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
    return flags;
}

bool FileDiffModel::hasChildren(QModelIndex const& index) const
{
    return (index.isValid()) ? false : true;
}

QModelIndex FileDiffModel::index(int row, int column, QModelIndex const& parent) const
{
    GHULBUS_ASSERT(!parent.isValid());
    return createIndex(row, column);
}

QModelIndex FileDiffModel::parent(QModelIndex const& index) const
{
    return QModelIndex();
}

int FileDiffModel::rowCount(QModelIndex const& index) const
{
    return (index.isValid()) ? 0 : static_cast<int>(m_file_index.size());
}

int FileDiffModel::columnCount(QModelIndex const& index) const
{
    return (index.isValid()) ? 0 : 6;
}

QVariant FileDiffModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Orientation::Horizontal) {
        if(role == Qt::DisplayRole)
        {
            switch(section) {
            case 0: return QLatin1String("Name");
            case 1: return QLatin1String("Size");
            case 2: return QLatin1String("Changed");
            case 3: return QLatin1String("");
            case 4: return QLatin1String("Old Size");
            case 5: return QLatin1String("Old Changed");
            }
        }
    }
    return QVariant();
}

QVariant FileDiffModel::data(QModelIndex const& index, int role) const
{
    if((!index.isValid()) || (index.row() >= m_file_index.size())) {
        return QVariant();
    }

    if(role == Qt::DisplayRole) {
        auto const row = index.row();
        auto const column = index.column();
        auto const time_format_str = QLocale::system().dateTimeFormat(QLocale::ShortFormat);
        auto date_to_string = [time_format_str](std::chrono::system_clock::time_point const& timep) -> QString {
            auto const timep_t = std::chrono::system_clock::to_time_t(timep);
            return QString(QDateTime::fromTime_t(timep_t, Qt::UTC).toLocalTime().toString(time_format_str));
        };
        auto filesize_to_string = [](std::uintmax_t bytes) -> QString {
            std::uintmax_t const kb = 1024;
            std::uintmax_t const mb = 1024 * kb;
            std::uintmax_t const gb = 1024 * mb;
            std::uintmax_t const tb = 1024 * gb;
            if (bytes >= tb) {
                return tr("%1 TB").arg(QLocale().toString(static_cast<double>(bytes) / tb, 'f', 3));
            } else if (bytes >= gb) {
                return tr("%1 GB").arg(QLocale().toString(static_cast<double>(bytes) / gb, 'f', 2));
            } else if (bytes >= mb) {
                return tr("%1 MB").arg(QLocale().toString(static_cast<double>(bytes) / mb, 'f', 1));
            } else if (bytes >= kb) {
                return tr("%1 KB").arg(QLocale().toString(bytes / kb));
            } else {
                return tr("%1 bytes").arg(QLocale().toString(bytes));
            }
        };
        auto const sync_state = m_file_index_diff.index_files[row].sync_status;
        if(column == 0) {
            return QString(m_file_index[row].path.generic_string().c_str());
        } else if(column == 1) {
            return filesize_to_string(m_file_index[row].size);
        } else if(column == 2) {
            return date_to_string(m_file_index[row].modified_time);
        } else if(column == 3) {
            switch(sync_state) {
            case FileSyncStatus::Unchanged:   return QLatin1String("Unchanged");
            case FileSyncStatus::NewFile:     return QLatin1String("New");
            case FileSyncStatus::FileChanged: return QLatin1String("Updated");
            case FileSyncStatus::FileRemoved: return QLatin1String("Removed");
            default:                          return QLatin1String("Unknown");
            }
        } else if(column == 4) {
            if(sync_state == FileSyncStatus::NewFile) {
                return QLatin1String("---");
            } else {
                return filesize_to_string(m_file_index_diff.index_files[row].reference_size);
            }
        } else if(column == 5) {
            if(sync_state == FileSyncStatus::NewFile) {
                return QLatin1String("---");
            } else {
                return date_to_string(m_file_index_diff.index_files[row].reference_modified_time);
            }
        }
        GHULBUS_UNREACHABLE();
    } else if(role == Qt::CheckStateRole) {
        if(index.column() == 0) {
            return m_entry_checked[index.row()] ? Qt::Checked : Qt::Unchecked;
        }
    }

    return QVariant();
}

bool FileDiffModel::setData(QModelIndex const& index, QVariant const& value, int role)
{
    if(role == Qt::CheckStateRole)
    {
        m_entry_checked[index.row()] = (value.toInt() == Qt::Checked);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}
