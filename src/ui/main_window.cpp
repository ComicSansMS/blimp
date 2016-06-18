
#include <ui/main_window.hpp>

#include <ui/filesystem_model.hpp>

#include <db/blimpdb.hpp>

#include <file_scanner.hpp>

#include <QBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QTreeView>
#include <QWidget>

#include <boost/filesystem.hpp>

struct MainWindow::Pimpl
{
    QWidget* central;
    QTreeView* treeview;
    FileSystemModel* model;
    QBoxLayout* layout;
    QPushButton* okButton;
    struct StatusBarWidgets {
        QLabel* progressLabel;
        QProgressBar* progressBar;
    } statusBar;
    FileScanner fileScanner;

    Pimpl(MainWindow* parent)
        :central(new QWidget(parent)), treeview(new QTreeView), model(new FileSystemModel(parent)),
         layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, central)),
         okButton(new QPushButton("Write checks to DB", parent))
    {
    }
};

MainWindow::MainWindow()
    :m_pimpl(std::make_unique<MainWindow::Pimpl>(this))
{
    setFixedSize(800, 600);

    statusBar();        // first call creates the status bar
    m_pimpl->statusBar.progressLabel = new QLabel();
    m_pimpl->statusBar.progressBar = new QProgressBar();
    statusBar()->addPermanentWidget(m_pimpl->statusBar.progressBar);
    statusBar()->addPermanentWidget(m_pimpl->statusBar.progressLabel);
    m_pimpl->statusBar.progressBar->hide();

    auto const drives = QDir::drives();
    m_pimpl->model->setRootPath(drives.first().absolutePath());
    m_pimpl->model->setFilter(QDir::AllDirs | QDir::Dirs | QDir::NoDotAndDotDot);
    m_pimpl->treeview->setModel(m_pimpl->model);
    m_pimpl->treeview->resize(width(), height());
    m_pimpl->treeview->setColumnWidth(0, 400);
    m_pimpl->treeview->hideColumn(1);
    m_pimpl->layout->addWidget(m_pimpl->treeview);
    m_pimpl->layout->addWidget(m_pimpl->okButton);
    m_pimpl->central->setLayout(m_pimpl->layout);
    setCentralWidget(m_pimpl->central);
    connect(m_pimpl->treeview, &QTreeView::clicked, m_pimpl->model, &FileSystemModel::itemClicked);
    connect(m_pimpl->okButton, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    connect(&m_pimpl->fileScanner, &FileScanner::fileListCompleted,
            this, &MainWindow::onFileScanFileListCompleted, Qt::QueuedConnection);
    show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::onButtonClicked()
{
    auto const checked_files = m_pimpl->model->getCheckedFilePaths();
    auto const qt_target_file = QFileDialog::getSaveFileName(this, "Save File Database", QString(),
                                                             "Blimp Database File (*.blimpdb)");
    auto const target_file = std::string(qt_target_file.toUtf8().constData());
    if(boost::filesystem::exists(target_file)) {
        boost::filesystem::remove(target_file);
    }
    BlimpDB::createNewFileDatabase(target_file, checked_files);
    m_pimpl->fileScanner.addFilesToScan(checked_files);
    m_pimpl->statusBar.progressBar->setMinimum(0);
    m_pimpl->statusBar.progressBar->setMaximum(0);
    m_pimpl->statusBar.progressBar->show();
    m_pimpl->statusBar.progressLabel->setText("Scanning...");
    m_pimpl->fileScanner.startScanning();
}

void MainWindow::onFileScanFileListCompleted(std::uintmax_t n_files)
{
    m_pimpl->statusBar.progressLabel->setText(QString::fromStdString("Scanning 0/" + std::to_string(n_files)));
    m_pimpl->statusBar.progressBar->setMinimum(0);
    m_pimpl->statusBar.progressBar->setMaximum(n_files);
}
