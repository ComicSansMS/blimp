
#include <QFileSystemModel>

#include <unordered_map>

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

public slots:
     void itemClicked(QModelIndex const& index);

private:
    std::unordered_map<QModelIndex, Qt::CheckState> m_checkMap;
};
