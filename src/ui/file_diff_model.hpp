#ifndef BLIMP_INCLUDE_GUARD_UI_FILE_DIFF_MODEL_HPP
#define BLIMP_INCLUDE_GUARD_UI_FILE_DIFF_MODEL_HPP

#include <file_info.hpp>

#include <QStandardItemModel>

#include <string>
#include <unordered_map>
#include <vector>

class FileDiffModel : public QStandardItemModel {
    Q_OBJECT
public:
    FileDiffModel(QObject* parent);

    void setFileIndexData(std::vector<FileInfo> const& file_index, FileIndexDiff const& file_index_diff);

private:

private:

};

#endif
