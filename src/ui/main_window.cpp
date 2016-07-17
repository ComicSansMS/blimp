
#include <ui/main_window.hpp>

#include <ui/file_diff_model.hpp>
#include <ui/filesystem_model.hpp>

#include <db/blimpdb.hpp>

#include <file_scanner.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <QBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStringBuilder>
#include <QTreeView>
#include <QWidget>

#include <boost/filesystem.hpp>

struct MainWindow::Pimpl
{
    QStackedWidget* central;

    struct WelcomePage {
        QWidget* widget;
        QBoxLayout* layout;
        QLabel* labelHeader;
        QPushButton* buttonNew;
        QPushButton* buttonOpen;
        QLabel* labelRecentFiles;
        QListWidget* listRecentFiles;
        QPushButton* buttonOpenRecent;

        WelcomePage(MainWindow* parent)
            :widget(new QWidget(parent)),
             layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, widget)),
             labelHeader(new QLabel(widget)),
             buttonNew(new QPushButton(widget)),
             buttonOpen(new QPushButton(widget)),
             labelRecentFiles(new QLabel(widget)),
             listRecentFiles(new QListWidget(widget)),
             buttonOpenRecent(new QPushButton(widget))
        {
            labelHeader->setText("<div style=\"font-size:xx-large;font-weight:bold\">Welcome to Blimp CB</div>");
            labelHeader->setAlignment(Qt::AlignCenter);
            layout->addWidget(labelHeader);

            layout->addStretch(2);

            buttonNew->setText("Create New Blimp Backup File");
            buttonNew->setMinimumHeight(50);
            layout->addWidget(buttonNew);

            buttonOpen->setText("Open Existing Blimp Backup File");
            buttonOpen->setMinimumHeight(50);
            layout->addWidget(buttonOpen);

            layout->addStretch(1);

            labelRecentFiles->setText("Recently Used Backup Files:");
            layout->addWidget(labelRecentFiles);
            listRecentFiles->setEnabled(false);

            layout->addWidget(listRecentFiles);
            buttonOpenRecent->setText("Open Selected");
            buttonOpenRecent->setEnabled(false);
            layout->addWidget(buttonOpenRecent);

            layout->addStretch(2);
        }
    } welcomePage;

    struct ScanSelectPage {
        QWidget* widget;
        QBoxLayout* layout;
        QTreeView* fstreeview;
        FileSystemModel* fsmodel;
        QPushButton* buttonScanSelected;

        ScanSelectPage(MainWindow* parent)
            :widget(new QWidget(parent)),
             layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, widget)),
             fstreeview(new QTreeView(widget)),
             fsmodel(new FileSystemModel(widget)),
             buttonScanSelected(new QPushButton(widget))
        {
            auto const drives = QDir::drives();
            fsmodel->setRootPath(drives.first().absolutePath());
            fsmodel->setFilter(QDir::AllDirs | QDir::Dirs | QDir::NoDotAndDotDot);
            fstreeview->setModel(fsmodel);
            fstreeview->setColumnWidth(0, parent->width() / 2);
            fstreeview->hideColumn(1);
            layout->addWidget(fstreeview);

            buttonScanSelected->setText("Scan Selected");
            layout->addWidget(buttonScanSelected);
        }
    } scanSelectPage;

    struct ProgressPage {
        QWidget* widget;
        QBoxLayout* layout;

        QLabel* labelHeader;
        QLabel* labelProgress1high;
        QProgressBar* progress1;
        QLabel* labelProgress1low;

        QLabel* labelProgress2high;
        QProgressBar* progress2;
        QLabel* labelProgress2low;

        QPushButton* buttonCancel;

        ProgressPage(MainWindow* parent)
            :widget(new QWidget(parent)),
             layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, widget)),
             labelHeader(new QLabel(widget)),
             labelProgress1high(new QLabel(widget)),
             progress1(new QProgressBar(widget)),
             labelProgress1low(new QLabel(widget)),
             labelProgress2high(new QLabel(widget)),
             progress2(new QProgressBar(widget)),
             labelProgress2low(new QLabel(widget)),
             buttonCancel(new QPushButton(widget))
        {
            layout->addWidget(labelHeader);
            layout->addStretch(2);
            layout->addWidget(labelProgress1high);
            layout->addWidget(progress1);
            labelProgress1low->setAlignment(Qt::AlignRight);
            layout->addWidget(labelProgress1low);
            layout->addWidget(labelProgress2high);
            layout->addWidget(progress2);
            labelProgress2low->setAlignment(Qt::AlignRight);
            layout->addWidget(labelProgress2low);
            layout->addStretch(2);
            buttonCancel->setMinimumHeight(40);
            layout->addWidget(buttonCancel);
        }
    } progressPage;

    struct FileDiffPage {
        QWidget* widget;
        QBoxLayout* layout;
        QTreeView* diffview;
        FileDiffModel* diffmodel;

        FileDiffPage(MainWindow* parent)
            :widget(new QWidget(parent)),
             layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, widget)),
             diffview(new QTreeView(widget)),
             diffmodel(new FileDiffModel(widget))
        {
            diffview->setModel(diffmodel);
            diffview->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
            diffview->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
            diffview->setUniformRowHeights(true); // impact on perf?
            layout->addWidget(diffview);
        }
    } fileDiffPage;

    struct StatusBarWidgets {
        QLabel* progressLabel;
        QProgressBar* progressBar;
    } statusBar;

    FileScanner fileScanner;
    std::uintmax_t numberOfFilesInIndex;
    std::unique_ptr<BlimpDB> blimpdb;

    Pimpl(MainWindow* parent)
        :central(new QStackedWidget(parent)),
         welcomePage(parent), scanSelectPage(parent), progressPage(parent), fileDiffPage(parent),
         numberOfFilesInIndex(0)
    {
    }
};

MainWindow::MainWindow()
{
    resize(800, 600);
    m_pimpl = std::make_unique<MainWindow::Pimpl>(this);

    statusBar();        // first call creates the status bar
    m_pimpl->statusBar.progressLabel = new QLabel();
    m_pimpl->statusBar.progressBar = new QProgressBar();
    statusBar()->addPermanentWidget(m_pimpl->statusBar.progressBar);
    statusBar()->addPermanentWidget(m_pimpl->statusBar.progressLabel);
    m_pimpl->statusBar.progressBar->hide();
    statusBar()->hide();

    // welcome page
    connect(m_pimpl->welcomePage.buttonNew, &QPushButton::clicked,
            this, &MainWindow::onNewDatabase);
    connect(m_pimpl->welcomePage.buttonOpen, &QPushButton::clicked,
            this, &MainWindow::onOpenDatabase);
    m_pimpl->central->addWidget(m_pimpl->welcomePage.widget);

    // scan select page
    connect(m_pimpl->scanSelectPage.buttonScanSelected, &QPushButton::clicked,
            this, &MainWindow::onStartFileScan);
    connect(m_pimpl->scanSelectPage.fstreeview, &QTreeView::clicked,
            m_pimpl->scanSelectPage.fsmodel, &FileSystemModel::itemClicked);
    m_pimpl->central->addWidget(m_pimpl->scanSelectPage.widget);

    // progress page
    m_pimpl->central->addWidget(m_pimpl->progressPage.widget);

    // file diff page
    m_pimpl->central->addWidget(m_pimpl->fileDiffPage.widget);

    m_pimpl->central->setCurrentWidget(m_pimpl->welcomePage.widget);
    setCentralWidget(m_pimpl->central);

    connect(&m_pimpl->fileScanner, &FileScanner::indexingUpdate,
            this, &MainWindow::onFileScanIndexingUpdate, Qt::QueuedConnection);
    connect(&m_pimpl->fileScanner, &FileScanner::indexingCompleted,
            this, &MainWindow::onFileScanIndexingCompleted, Qt::QueuedConnection);
    connect(&m_pimpl->fileScanner, &FileScanner::indexDiffCompleted,
            this, &MainWindow::onFileScanDiffCompleted, Qt::QueuedConnection);
    connect(&m_pimpl->fileScanner, &FileScanner::checksumCalculationUpdate,
            this, &MainWindow::onFileScanChecksumUpdate, Qt::QueuedConnection);
    connect(&m_pimpl->fileScanner, &FileScanner::checksumCalculationCompleted,
            this, &MainWindow::onFileScanChecksumCompleted, Qt::QueuedConnection);

    show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::onNewDatabase()
{
    auto const qt_target_file = QFileDialog::getSaveFileName(this, "Save File Database", QString(),
                                                             "Blimp Database File (*.blimpdb)");
    if(qt_target_file.isEmpty()) {
        return;
    }
    m_pimpl->blimpdb.reset();
    auto const target_file = std::string(qt_target_file.toUtf8().constData());
    try {
        if(boost::filesystem::exists(target_file)) {
            boost::filesystem::remove(target_file);
        }
    } catch(boost::filesystem::filesystem_error& e) {
        QMessageBox msgBox;
        msgBox.setText(QString("Error while accessing file ") + qt_target_file + ".");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(QString("Ensure that the file is not in use by another application."));
        msgBox.setDetailedText(QString("The reported error was:\n") + e.what() + ".");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    try {
        m_pimpl->blimpdb = std::make_unique<BlimpDB>(target_file, BlimpDB::OpenMode::CreateNew);
    } catch(std::exception& e) {
        QMessageBox msgBox;
        msgBox.setText(QString("Error trying to create new database file ") + qt_target_file + ".");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(QString("Ensure that the file is not in use by another application."));
        msgBox.setDetailedText(QString("The reported error was:\n") + e.what() + ".");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    statusBar()->show();
    m_pimpl->scanSelectPage.widget->setEnabled(true);
    m_pimpl->central->setCurrentWidget(m_pimpl->scanSelectPage.widget);
}

void MainWindow::onOpenDatabase()
{
    auto const qt_target_file = QFileDialog::getOpenFileName(this, "Open File Database", QString(),
                                                             "Blimp Database File (*.blimpdb)");
    if(qt_target_file.isEmpty()) {
        return;
    }
    auto const target_file = std::string(qt_target_file.toUtf8().constData());
    m_pimpl->blimpdb.reset();
    try {
        m_pimpl->blimpdb = std::make_unique<BlimpDB>(target_file, BlimpDB::OpenMode::OpenExisting);
    } catch(std::exception& e) {
        QMessageBox msgBox;
        msgBox.setText(QString("Error while accessing file ") + qt_target_file + ".");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(QString("Ensure that the file is a valid database file."));
        msgBox.setDetailedText(QString("The reported error was:\n") + e.what() + ".");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    m_pimpl->scanSelectPage.fsmodel->setCheckedFilePaths(m_pimpl->blimpdb->getUserSelection());
    statusBar()->show();
    m_pimpl->scanSelectPage.widget->setEnabled(true);
    m_pimpl->central->setCurrentWidget(m_pimpl->scanSelectPage.widget);
}

void MainWindow::onStartFileScan()
{
    GHULBUS_ASSERT(m_pimpl->blimpdb);
    m_pimpl->scanSelectPage.widget->setEnabled(false);
    auto const checked_files = m_pimpl->scanSelectPage.fsmodel->getCheckedFilePaths();
    try {
        m_pimpl->blimpdb->setUserSelection(checked_files);
    } catch(std::exception& e) {
        QMessageBox msgBox;
        msgBox.setText(QString("Error while accessing database."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(QString("The database file could be corrupt."));
        msgBox.setDetailedText(QString("The reported error was:\n") + e.what() + ".");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        m_pimpl->scanSelectPage.widget->setEnabled(true);
        return;
    }

    m_pimpl->progressPage.progress2->hide();
    m_pimpl->progressPage.labelProgress2high->hide();
    m_pimpl->progressPage.labelProgress2low->hide();

    m_pimpl->progressPage.labelHeader->setText("<div style=\"font-size:xx-large;font-weight:bold\">Scanning Files</div>");
    m_pimpl->progressPage.labelProgress1high->setText("Indexing...");
    m_pimpl->progressPage.labelProgress1low->setText("");
    m_pimpl->progressPage.progress1->setMinimum(0);
    m_pimpl->progressPage.progress1->setMaximum(0);
    m_pimpl->progressPage.progress1->setValue(0);
    m_pimpl->progressPage.buttonCancel->setText("Cancel Scanning");
    disconnect(m_pimpl->progressPage.buttonCancel, &QPushButton::clicked, nullptr, nullptr);
    connect(m_pimpl->progressPage.buttonCancel, &QPushButton::clicked,
            this, &MainWindow::onCancelFileScan);
    m_pimpl->progressPage.buttonCancel->setEnabled(true);

    m_pimpl->central->setCurrentWidget(m_pimpl->progressPage.widget);

    m_pimpl->fileScanner.addFilesForIndexing(checked_files);
    m_pimpl->fileScanner.startScanning(std::move(m_pimpl->blimpdb));
}

void MainWindow::onCancelFileScan()
{
    m_pimpl->progressPage.buttonCancel->setEnabled(false);
    m_pimpl->fileScanner.cancelScanning();
    m_pimpl->progressPage.buttonCancel->setEnabled(true);
    statusBar()->showMessage("Scanning canceled.", 5000);
}

void MainWindow::onFileScanIndexingUpdate(std::uintmax_t n_files)
{
    m_pimpl->progressPage.labelProgress1low->setText(
        QStringLiteral("Indexing ") % QString::number(n_files) % QStringLiteral("..."));
}

void MainWindow::onFileScanIndexingCompleted(std::uintmax_t n_files)
{
    m_pimpl->numberOfFilesInIndex = n_files;
    statusBar()->showMessage(QStringLiteral("Indexing complete. Found ") % QString::number(n_files) %
        QStringLiteral(" file") % (n_files == 1 ? QStringLiteral("") : QStringLiteral("s")) % QStringLiteral("."));
    m_pimpl->progressPage.labelProgress1low->setText("");
    m_pimpl->progressPage.labelProgress1high->setText("Comparing scan to database index...");
}

void MainWindow::onFileScanDiffCompleted()
{
    m_pimpl->fileScanner.joinScanning();
    m_pimpl->fileDiffPage.diffmodel->setFileIndexData(m_pimpl->fileScanner.getIndexList(),
                                                      m_pimpl->fileScanner.getIndexDiff());
    m_pimpl->central->setCurrentWidget(m_pimpl->fileDiffPage.widget);

}

void MainWindow::onFileScanChecksumUpdate(std::uintmax_t n_files)
{
    m_pimpl->statusBar.progressLabel->setText(QString::fromStdString("Scanning " + std::to_string(n_files) + "/" +
                                                                     std::to_string(m_pimpl->numberOfFilesInIndex)));
    m_pimpl->statusBar.progressBar->setValue(n_files);
}

void MainWindow::onFileScanChecksumCompleted()
{
    m_pimpl->fileScanner.cancelScanning();
    m_pimpl->fileDiffPage.diffmodel->setFileIndexData(m_pimpl->fileScanner.getIndexList(), m_pimpl->fileScanner.getIndexDiff());
}
