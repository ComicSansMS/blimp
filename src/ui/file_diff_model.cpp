#include <ui/file_diff_model.hpp>

#include <gbBase/Assert.hpp>


FileDiffModel::FileDiffModel(QObject* parent)
    :QStandardItemModel(parent)
{
    setColumnCount(6);
    setHorizontalHeaderItem(0, new QStandardItem("Name"));
    setHorizontalHeaderItem(1, new QStandardItem("Size"));
    setHorizontalHeaderItem(2, new QStandardItem("Changed"));
    setHorizontalHeaderItem(3, new QStandardItem());
    setHorizontalHeaderItem(4, new QStandardItem("Old Size"));
    setHorizontalHeaderItem(5, new QStandardItem("Old Changed"));
}

void FileDiffModel::setFileIndexData(std::vector<FileInfo> const& file_index, FileIndexDiff const& file_index_diff)
{
    GHULBUS_PRECONDITION(file_index.size() == file_index_diff.index_files.size());
    for(int i = 0; i < file_index.size(); ++i)
    {
        auto set_item_props = [](QStandardItem* item, bool checkable=false) -> QStandardItem* {
                item->setCheckable(checkable);
                item->setEditable(false);
                item->setDragEnabled(false);
                item->setDropEnabled(false);
                return item;
            };
        {
            auto name_item = new QStandardItem(file_index[i].path.generic_string().c_str());
            setItem(i, 0, set_item_props(name_item, true));
        }
        auto sync_to_string = [](FileSyncStatus sync_status) -> char const* {
            switch(sync_status) {
            case FileSyncStatus::Unchanged:   return "Unchanged";
            case FileSyncStatus::NewFile:     return "New";
            case FileSyncStatus::FileChanged: return "Updated";
            case FileSyncStatus::FileRemoved: return "Removed";
            default:                          return "Unknown";
            }
        };
        setItem(i, 1, set_item_props(new QStandardItem(std::to_string(file_index[i].size).c_str())));
        setItem(i, 2, set_item_props(new QStandardItem("<Date>")));
        setItem(i, 3, set_item_props(new QStandardItem(sync_to_string(file_index_diff.index_files[i].sync_status))));
        setItem(i, 4, set_item_props(new QStandardItem(std::to_string(file_index_diff.index_files[i].reference_size).c_str())));
        setItem(i, 5, set_item_props(new QStandardItem("<OldDate>")));
    }
}
