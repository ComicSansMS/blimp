#include <ui/snapshot_contents_model.hpp>

#include <gbBase/Assert.hpp>

#include <QDateTime>
#include <QLocale>

SnapshotContentsModel::SnapshotContentsModel(QObject* parent)
    :QAbstractItemModel(parent)
{
    clear();
}

void SnapshotContentsModel::clear()
{
    m_items.clear();
    m_items.push_back(SnapshotContentItem{ .path = {}, .file_element = std::nullopt, .children = {},
                                           .index = 0, .parent_index = 0, .row = 0 });
}

SnapshotContentsModel::SnapshotContentItem& SnapshotContentsModel::newItem(SnapshotContentItem& parent, QString const& p)
{
    auto const new_index = m_items.size();
    SnapshotContentItem new_item{
        .path = p,
        .index = new_index,
        .parent_index = parent.index,
        .row = parent.row + 1
    };
    parent.children.push_back(new_index);
    m_items.emplace_back(std::move(new_item));
    return m_items.back();
}

SnapshotContentsModel::SnapshotContentItem& SnapshotContentsModel::getItemByIndex(std::size_t index)
{
    GHULBUS_PRECONDITION(index < m_items.size());
    return m_items[index];
}

void SnapshotContentsModel::addItem(BlimpDB::FileElement const file_element)
{
    std::size_t n = 0;
    for (auto const& p : file_element.info.path) {
        auto const p_str = QString::fromStdString(p.generic_string());
        auto const it_child = std::find_if(begin(m_items[n].children), end(m_items[n].children),
                                           [this, &p_str](std::size_t idx) { return m_items[idx].path == p_str; });
        if (it_child != end(m_items[n].children)) {
            n = *it_child;
        } else {
            n = newItem(m_items[n], p_str).index;
        }
    }
    m_items[n].file_element = file_element;
}

void SnapshotContentsModel::finalize()
{
    for (auto& i : m_items) {
        if ((i.index == 0) || (i.parent_index == 0)) { continue; }
        if (!i.file_element) {
            SnapshotContentItem& parent = m_items[i.parent_index];
            if (parent.children.size() == 1) {
                parent.children = i.children;
                m_items[parent.children.front()].parent_index = parent.index;
                parent.path = parent.path + "\\" + i.path;
                i.index = 0;
            }
        }
    }
}

Qt::ItemFlags SnapshotContentsModel::flags(QModelIndex const& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QModelIndex SnapshotContentsModel::index(int row, int column, QModelIndex const& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    SnapshotContentItem const* n;
    if (!parent.isValid()) {
        n = &m_items.front();
    } else {
        n = static_cast<SnapshotContentItem const*>(parent.internalPointer());
    }
    if (row < n->children.size()) {
        return createIndex(row, column, &m_items[n->children[row]]);
    }
    return {};
}

QModelIndex SnapshotContentsModel::parent(QModelIndex const& index) const
{
    if (!index.isValid()) {
        return {};
    }

    SnapshotContentItem const* child_item = static_cast<SnapshotContentItem*>(index.internalPointer());
    if (child_item->parent_index == 0) {
        return {};
    }

    SnapshotContentItem* parent_item = &m_items[child_item->parent_index];
    return createIndex(parent_item->row, 0, parent_item);
}

int SnapshotContentsModel::rowCount(QModelIndex const& index) const
{
    SnapshotContentItem* parent_item;
    if (index.column() > 0) {
        return 0;
    }

    if (!index.isValid()) {
        parent_item = &(m_items[0]);
    } else {
        parent_item = static_cast<SnapshotContentItem*>(index.internalPointer());
    }

    return static_cast<int>(parent_item->children.size());
}

int SnapshotContentsModel::columnCount(QModelIndex const& index) const
{
    return 3;
}

QVariant SnapshotContentsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Orientation::Horizontal) {
        if(role == Qt::DisplayRole)
        {
            switch(section) {
            case 0: return QLatin1String("Path");
            case 1: return QLatin1String("Date");
            case 2: return QLatin1String("Size");
            }
        }
    }
    return {};
}

QVariant SnapshotContentsModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    SnapshotContentItem* item = static_cast<SnapshotContentItem*>(index.internalPointer());

    auto const col = index.column();
    if (col == 0) {
        return item->path;
    }

    auto const time_format_str = QLocale::system().dateTimeFormat(QLocale::ShortFormat);
    auto date_to_string = [time_format_str](std::chrono::system_clock::time_point const& timep) -> QString {
        auto const timep_t = std::chrono::system_clock::to_time_t(timep);
        return QString(QDateTime::fromTime_t(timep_t, Qt::UTC).toLocalTime().toString(time_format_str));
    };
    if (item->file_element) {
        if (col == 1) {
            return date_to_string(item->file_element->info.modified_time);
        } else if (col == 2) {
            return item->file_element->info.size;
        }
    }
    return {};
}
