
#include <QFileSystemModel>

#include <string>
#include <unordered_map>
#include <vector>

namespace std
{
template<> struct hash<QModelIndex>
{
    typedef QModelIndex argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const
    {
        auto id = s.internalId();
        return hash<decltype(id)>()(id);
    }
};
}

class FileSystemModel : public QFileSystemModel {
    Q_OBJECT
public:
    FileSystemModel(QObject* parent);

    QVariant data(QModelIndex const& index, int role) const override;

    Qt::ItemFlags flags(QModelIndex const& index) const override;

    std::vector<std::string> getCheckedFilePaths() const;

public slots:
     void itemClicked(QModelIndex const& index);

private slots:
    void onNewRow(QModelIndex const& parent, int start, int end);

private:
    void propagateCheckChangeDown(QModelIndex const& node_index, Qt::CheckState new_state);
    void propagateCheckChangeUp(QModelIndex const& node_index, Qt::CheckState new_state);

    std::vector<QModelIndex> getChildren(QModelIndex const& node_index) const;

    void addCheckedFilePaths_rec(QModelIndex const& node_index, std::vector<std::string>& checkedFilePaths) const;

private:
    std::unordered_map<QModelIndex, Qt::CheckState> m_checkMap;
};
