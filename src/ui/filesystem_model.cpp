#include <ui/filesystem_model.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>


FileSystemModel::FileSystemModel(QObject* parent)
    :QFileSystemModel(parent)
{
    connect(this, &FileSystemModel::rowsInserted, this, &FileSystemModel::onNewRow);
}


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

std::vector<std::string> FileSystemModel::getCheckedFilePaths() const
{
    std::vector<std::string> ret;
    addCheckedFilePaths_rec(QModelIndex(), ret);
    return ret;
}

void FileSystemModel::addCheckedFilePaths_rec(QModelIndex const& node_index,
                                              std::vector<std::string>& checkedFilePaths) const
{
    auto const children = getChildren(node_index);
    auto const it_end = m_checkMap.end();
    for(auto const& child : children) {
        auto const it = m_checkMap.find(child);
        if(it != it_end) {
            if(it->second == Qt::Checked) {
                checkedFilePaths.emplace_back(QFileSystemModel::filePath(child).toUtf8());
            } else if(it->second == Qt::PartiallyChecked) {
                addCheckedFilePaths_rec(child, checkedFilePaths);
            }
        }
    }
}

void FileSystemModel::setCheckedFilePaths(std::vector<std::string> const& user_selection)
{
    m_checkMap.clear();
    for(auto const& f : user_selection) {
        auto const node_index = index(QString::fromStdString(f));
        m_checkMap[node_index] = Qt::Checked;
        auto it_parent = parent(node_index);
        while(it_parent.isValid()) {
            if(m_checkMap.find(it_parent) == end(m_checkMap)) {
                m_checkMap[it_parent] = Qt::PartiallyChecked;
            }
            it_parent = parent(it_parent);
        }
    }
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
        } else {
            element = Qt::Checked;
        }
        QVector<int> roles_changed;
        roles_changed.push_back(Qt::CheckStateRole);
        emit dataChanged(index, index, roles_changed);
        propagateCheckChangeDown(index, element);
        propagateCheckChangeUp(index, element);
    }
}

std::vector<QModelIndex> FileSystemModel::getChildren(QModelIndex const& node_index) const
{
    auto const nChildren = rowCount(node_index);
    std::vector<QModelIndex> children;
    children.reserve(nChildren);
    for(int i=0; i<nChildren; ++i) {
        auto const node_child = index(i, 0, node_index);
        GHULBUS_ASSERT(node_child.isValid());
        children.push_back(node_child);
    }
    return children;
}

void FileSystemModel::propagateCheckChangeDown(QModelIndex const& node_index, Qt::CheckState new_state)
{
    if (new_state == Qt::PartiallyChecked) {
        return;
    }
    // if we are fully (un-)checked, so are all our children and their children
    std::vector<QModelIndex> const children = getChildren(node_index);
    QVector<int> roles_changed;
    roles_changed.push_back(Qt::CheckStateRole);
    for(auto& child : children) {
        m_checkMap[child] = new_state;
        emit dataChanged(child, child, roles_changed);
        propagateCheckChangeDown(child, new_state);
    }
}

void FileSystemModel::propagateCheckChangeUp(QModelIndex const& node_index, Qt::CheckState new_state)
{
    auto const node_parent = parent(node_index);
    if(node_parent.isValid())
    {
        std::vector<QModelIndex> const siblings = getChildren(node_parent);

        bool parent_changed = false;
        if(new_state == Qt::Checked) {
            // my parent is checked if all my siblings are checked
            auto const is_checked = [this](QModelIndex const& idx) {
                auto it = m_checkMap.find(idx);
                return (it != end(m_checkMap)) && (it->second == Qt::Checked);
            };
            if(std::all_of(begin(siblings), end(siblings), is_checked)) {
                m_checkMap[node_parent] = Qt::Checked;
                parent_changed = true;
            }
        } else if(new_state == Qt::Unchecked) {
            // my parent is unchecked if all my siblings are unchecked
            auto const is_unchecked = [this](QModelIndex const& idx) {
                auto it = m_checkMap.find(idx);
                return (it == end(m_checkMap)) || (it->second == Qt::Unchecked);
            };
            if(std::all_of(begin(siblings), end(siblings), is_unchecked)) {
                m_checkMap[node_parent] = Qt::Unchecked;
                parent_changed = true;
            }
        }
        if(!parent_changed) {
            // if my parent is neither checked nor unchecked, it has to be partially checked
            auto& element = m_checkMap[node_parent];
            if(element != Qt::PartiallyChecked) {
                element = Qt::PartiallyChecked;
                parent_changed = true;
            }
        }
        if(parent_changed) {
            // if my parent changed, my grand-parent might need to change too
            QVector<int> roles_changed;
            roles_changed.push_back(Qt::CheckStateRole);
            emit dataChanged(node_parent, node_parent, roles_changed);
            propagateCheckChangeUp(node_parent, m_checkMap[node_parent]);
        }
    }
}

void FileSystemModel::onNewRow(QModelIndex const& parent, int start, int end)
{
    GHULBUS_LOG(Debug, "New rows [" << start << ".." << end << "]" << " under "
                       << (parent.isValid() ? filePath(parent).toUtf8().data() : "root"));
    auto it = m_checkMap.find(parent);
    if(it != m_checkMap.end()) {
        if((it->second == Qt::Checked) || (it->second == Qt::Unchecked)) {
            propagateCheckChangeDown(parent, it->second);
        }
    }
}
