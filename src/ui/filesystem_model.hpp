
#include <QFileSystemModel>

class FileSystemModel : public QFileSystemModel {
    Q_OBJECT
public:
    FileSystemModel(QObject* parent);

    QVariant data(QModelIndex const& index, int role) const override;
};

