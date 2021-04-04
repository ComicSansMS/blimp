#include <ui/snapshot_browser.hpp>

#include <db/blimpdb.hpp>
#include <ui/snapshot_contents_model.hpp>

#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QTreeView>

namespace {
int getNumberOfDigits(std::size_t n)
{
    int ret = 0;
    for (; n != 0; n /= 10) { ++ret; }
    return ret;
}
}

SnapshotBrowser::SnapshotBrowser(QWidget* parent)
    :QWidget(parent), m_layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, this)),
     m_comboSnapshotList(new QComboBox(this)), m_model(new SnapshotContentsModel(this)),
     m_treeView(new QTreeView(this))
{
    m_layout->addWidget(m_comboSnapshotList);
    m_treeView->setModel(m_model);
    m_layout->addWidget(m_treeView);
}

void SnapshotBrowser::setData(BlimpDB& blimpdb)
{
    auto const snapshots = blimpdb.getSnapshots();
    m_comboSnapshotList->clear();

    int count = 0;
    int const n_digits = getNumberOfDigits(snapshots.size());
    for (auto const& s : snapshots) {
        m_comboSnapshotList->addItem(QString("#%1 - %2")
                                     .arg(count, n_digits, 10, QChar('0'))
                                     .arg(QString::fromStdString(s.name)));
        ++count;
    }

    m_model->clear();
    auto const file_infos = blimpdb.getFileElementsForSnapshot(snapshots.front().id);
    for (auto const& f : file_infos) { m_model->addItem(f); }
    m_model->finalize();
}

