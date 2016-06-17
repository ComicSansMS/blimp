
#include <ui/main_window.hpp>
#include <db/blimpdb.hpp>

#include <QFileDialog>

#include <boost/filesystem.hpp>

MainWindow::MainWindow()
    :m_central(new QWidget(this)), m_treeview(new QTreeView), m_model(new FileSystemModel(this)),
     m_layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, m_central)),
     m_okButton(new QPushButton("Write checks to DB", this))
{
    setFixedSize(800, 600);
    auto const drives = QDir::drives();
    m_model->setRootPath(drives.first().absolutePath());
    m_model->setFilter(QDir::AllDirs | QDir::Dirs | QDir::NoDotAndDotDot);
    m_treeview->setModel(m_model);
    m_treeview->resize(width(), height());
    m_treeview->setColumnWidth(0, 400);
    m_treeview->hideColumn(1);
    m_layout->addWidget(m_treeview);
    m_layout->addWidget(m_okButton);
    m_central->setLayout(m_layout);
    setCentralWidget(m_central);
    connect(m_treeview, &QTreeView::clicked, m_model, &FileSystemModel::itemClicked);
    connect(m_okButton, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::onButtonClicked()
{
    auto const checked_files = m_model->getCheckedFilePaths();
    auto const qt_target_file = QFileDialog::getSaveFileName(this, "Save File Database", QString(),
                                                             "Blimp Database File (*.blimpdb)");
    auto const target_file = std::string(qt_target_file.toUtf8().constData());
    if(boost::filesystem::exists(target_file)) {
        boost::filesystem::remove(target_file);
    }
    BlimpDB::createNewFileDatabase(target_file, checked_files);
}
