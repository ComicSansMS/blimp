
#include <ui/main_window.hpp>

#include <ui/file_diff_model.hpp>
#include <ui/filesize_to_string.hpp>
#include <ui/filesystem_model.hpp>
#include <ui/snapshot_browser.hpp>

#include <db/blimpdb.hpp>

#include <exceptions.hpp>
#include <file_scanner.hpp>
#include <file_processor.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <QBoxLayout>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
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

#include <numeric>

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
            labelHeader->setText(
                "<div style=\"font-size:xx-large;font-weight:bold\">" + tr("Welcome to Blimp CB") + "</div>");
            labelHeader->setAlignment(Qt::AlignCenter);
            layout->addWidget(labelHeader);

            layout->addStretch(2);

            buttonNew->setText(tr("Create New Blimp Backup File"));
            buttonNew->setMinimumHeight(50);
            layout->addWidget(buttonNew);

            buttonOpen->setText(tr("Open Existing Blimp Backup File"));
            buttonOpen->setMinimumHeight(50);
            layout->addWidget(buttonOpen);

            layout->addStretch(1);

            labelRecentFiles->setText(tr("Recently Used Backup Files:"));
            layout->addWidget(labelRecentFiles);
            listRecentFiles->setEnabled(false);

            layout->addWidget(listRecentFiles);
            buttonOpenRecent->setText(tr("Open Selected"));
            buttonOpenRecent->setEnabled(false);
            layout->addWidget(buttonOpenRecent);

            layout->addStretch(2);
        }
    } welcomePage;

    struct SnapshotBrowserPage {
        QWidget* widget;
        QBoxLayout* layout;
        SnapshotBrowser* snapshotBrowser;
        QPushButton* buttonCreateNewSnapshot;

        SnapshotBrowserPage(QWidget* parent)
            :widget(new QWidget(parent)),
             layout(new QBoxLayout(QBoxLayout::Direction::TopToBottom, widget)),
             snapshotBrowser(new SnapshotBrowser(widget)),
             buttonCreateNewSnapshot(new QPushButton(widget))
        {
            layout->addWidget(snapshotBrowser);
            buttonCreateNewSnapshot->setText("New Snapshot");
            layout->addWidget(buttonCreateNewSnapshot);
        }
    } snapshotBrowserPage;

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

            buttonScanSelected->setText(tr("Scan Selected"));
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
        QBoxLayout* inner_layout;
        QPushButton* buttonOk;
        QLabel* labelSize;

        FileDiffPage(MainWindow* parent)
            :widget(new QWidget(parent)),
             layout(new QBoxLayout(QBoxLayout::Direction::LeftToRight, widget)),
             diffview(new QTreeView(widget)),
             diffmodel(new FileDiffModel(widget)),
             inner_layout(nullptr),
             buttonOk(new QPushButton(widget)),
             labelSize(new QLabel(widget))
        {
            diffview->setModel(diffmodel);
            diffview->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
            diffview->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
            diffview->setUniformRowHeights(true); // impact on perf?
            layout->addWidget(diffview);
            inner_layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
            layout->addLayout(inner_layout);
            buttonOk->setText(tr("OK"));
            inner_layout->addWidget(buttonOk);
            inner_layout->addStretch(1);
            inner_layout->addWidget(labelSize);
        }
    } fileDiffPage;

    struct CreateSnapshotPage {
        QWidget* widget;
        QFormLayout* layout;
        QLineEdit* editSnapshotName;
        QCheckBox* checkboxForceChecksum;
        QPushButton* buttonCreateSnapshot;
        QPushButton* buttonCancel;
        std::vector<FileInfo> checked_files;

        CreateSnapshotPage(MainWindow* parent)
            :widget(new QWidget(parent)),
             layout(new QFormLayout(widget)),
             editSnapshotName(new QLineEdit(widget)),
             checkboxForceChecksum(new QCheckBox(widget)),
             buttonCreateSnapshot(new QPushButton(widget)),
             buttonCancel(new QPushButton(widget))
        {
            layout->addRow("Snapshot Name: ", editSnapshotName);
            checkboxForceChecksum->setText("Recompute checksums even for unchanged files");
            checkboxForceChecksum->setChecked(false);
            checkboxForceChecksum->setEnabled(false);
            layout->addWidget(checkboxForceChecksum);
            buttonCreateSnapshot->setText("Create Snapshot");
            layout->addWidget(buttonCreateSnapshot);
            buttonCancel->setText("Cancel");
            layout->addWidget(buttonCancel);
        }
    } createSnapshotPage;

    struct StatusBarWidgets {
        QLabel* progressLabel;
        QProgressBar* progressBar;

        StatusBarWidgets() : progressLabel(nullptr), progressBar(nullptr) {}
    } statusBar;

    FileScanner fileScanner;
    FileProcessor fileProcessor;
    std::uint64_t numberOfFilesInIndex;
    std::unique_ptr<BlimpDB> blimpdb;

    Pimpl(MainWindow* parent)
        :central(new QStackedWidget(parent)),
         welcomePage(parent), snapshotBrowserPage(parent), scanSelectPage(parent), progressPage(parent),
         fileDiffPage(parent), createSnapshotPage(parent),
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

    // snapshot browser page
    connect(m_pimpl->snapshotBrowserPage.buttonCreateNewSnapshot, &QPushButton::clicked,
            this, &MainWindow::onNewSnapshot);
    m_pimpl->central->addWidget(m_pimpl->snapshotBrowserPage.widget);

    // scan select page
    connect(m_pimpl->scanSelectPage.buttonScanSelected, &QPushButton::clicked,
            this, &MainWindow::onStartFileScan);
    connect(m_pimpl->scanSelectPage.fstreeview, &QTreeView::clicked,
            m_pimpl->scanSelectPage.fsmodel, &FileSystemModel::itemClicked);
    m_pimpl->central->addWidget(m_pimpl->scanSelectPage.widget);

    // progress page
    m_pimpl->central->addWidget(m_pimpl->progressPage.widget);

    // file diff page
    connect(m_pimpl->fileDiffPage.buttonOk, &QPushButton::clicked,
            this, &MainWindow::onFileDiffApprove);
    m_pimpl->central->addWidget(m_pimpl->fileDiffPage.widget);

    // create snapshot page
    connect(m_pimpl->createSnapshotPage.buttonCreateSnapshot, &QPushButton::clicked,
            this, &MainWindow::onCreateSnapshotRequest);
    connect(m_pimpl->createSnapshotPage.buttonCancel, &QPushButton::clicked,
            this, &MainWindow::onCreateSnapshotCancel);
    m_pimpl->central->addWidget(m_pimpl->createSnapshotPage.widget);

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
    connect(&m_pimpl->fileProcessor, &FileProcessor::processingUpdateNewFile,
            this, &MainWindow::onProcessingUpdateNewFile, Qt::QueuedConnection);
    connect(&m_pimpl->fileProcessor, &FileProcessor::processingUpdateHashProgress,
            this, &MainWindow::onProcessingUpdateHashProgress, Qt::QueuedConnection);
    connect(&m_pimpl->fileProcessor, &FileProcessor::processingUpdateHashCompleted,
            this, &MainWindow::onProcessingUpdateHashCompleted, Qt::QueuedConnection);
    connect(&m_pimpl->fileProcessor, &FileProcessor::processingUpdateFileProgress,
            this, &MainWindow::onProcessingUpdateFileProgress, Qt::QueuedConnection);
    connect(&m_pimpl->fileProcessor, &FileProcessor::processingCompleted,
            this, &MainWindow::onProcessingCompleted, Qt::QueuedConnection);
    connect(&m_pimpl->fileProcessor, &FileProcessor::processingCanceled,
            this, &MainWindow::onProcessingCanceled, Qt::QueuedConnection);
    connect(m_pimpl->snapshotBrowserPage.snapshotBrowser, &SnapshotBrowser::fileRetrievalRequest,
            this, &MainWindow::onFileRetrievalRequested);

    show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* close_event)
{
    GHULBUS_UNUSED_VARIABLE(close_event);
    auto const answer = QMessageBox::warning(this, "Blimp",
                                             "Do you want to quit blimp and abort any running operations?",
                                             QMessageBox::Yes, QMessageBox::No);
    if (answer == QMessageBox::No) {
        close_event->ignore();
    } else {
        m_pimpl->fileScanner.cancelScanning();
        m_pimpl->fileProcessor.cancelProcessing();
        close_event->accept();
    }
}

void MainWindow::onNewDatabase()
{
    auto const qt_target_file = QFileDialog::getSaveFileName(this, tr("Save File Database"), QString(),
                                                             tr("Blimp Database File") + " (*.blimpdb)");
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
        msgBox.setText(tr("Error while accessing file ") + qt_target_file + ".");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(tr("Ensure that the file is not in use by another application."));
        msgBox.setDetailedText(tr("The reported error was:\n%1").arg(e.what()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    try {
        m_pimpl->blimpdb = std::make_unique<BlimpDB>(target_file, BlimpDB::OpenMode::CreateNew);
    } catch(std::exception& e) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error trying to create new database file ") + qt_target_file + ".");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(tr("Ensure that the file is not in use by another application."));
        msgBox.setDetailedText(tr("The reported error was:\n%1").arg(e.what()));
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
    auto const qt_target_file = QFileDialog::getOpenFileName(this, tr("Open File Database"), QString(),
                                                             tr("Blimp Database File") + " (*.blimpdb)");
    if(qt_target_file.isEmpty()) {
        return;
    }
    auto const target_file = std::string(qt_target_file.toUtf8().constData());
    m_pimpl->blimpdb.reset();
    try {
        m_pimpl->blimpdb = std::make_unique<BlimpDB>(target_file, BlimpDB::OpenMode::OpenExisting);
    } catch(std::exception& e) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error while accessing file ") + qt_target_file + ".");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(tr("Ensure that the file is a valid database file."));
        msgBox.setDetailedText(tr("The reported error was:\n%1").arg(e.what()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    m_pimpl->snapshotBrowserPage.snapshotBrowser->setData(*m_pimpl->blimpdb);
    m_pimpl->central->setCurrentWidget(m_pimpl->snapshotBrowserPage.widget);
}

void MainWindow::onNewSnapshot()
{
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
        msgBox.setText(tr("Error while accessing database."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setInformativeText(tr("The database file could be corrupt."));
        msgBox.setDetailedText(tr("The reported error was:\n%1").arg(e.what()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        m_pimpl->scanSelectPage.widget->setEnabled(true);
        return;
    }

    m_pimpl->progressPage.progress2->hide();
    m_pimpl->progressPage.labelProgress2high->hide();
    m_pimpl->progressPage.labelProgress2low->hide();

    m_pimpl->progressPage.labelHeader->setText(
        "<div style=\"font-size:xx-large;font-weight:bold\">" + tr("Scanning Files") + "</div>");
    m_pimpl->progressPage.labelProgress1high->setText(tr("Indexing..."));
    m_pimpl->progressPage.labelProgress1low->setText(tr(""));
    m_pimpl->progressPage.progress1->setMinimum(0);
    m_pimpl->progressPage.progress1->setMaximum(0);
    m_pimpl->progressPage.progress1->setValue(0);
    m_pimpl->progressPage.buttonCancel->setText(tr("Cancel Scanning"));
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
    statusBar()->showMessage(tr("Scanning canceled."), 5000);
    if (!m_pimpl->blimpdb) {
        m_pimpl->blimpdb = m_pimpl->fileScanner.joinScanning();
    }
}

void MainWindow::onFileScanIndexingUpdate(std::uint64_t n_files)
{
    m_pimpl->progressPage.labelProgress1low->setText(tr("Indexing %1...").arg(n_files));
}

void MainWindow::onFileScanIndexingCompleted(std::uint64_t n_files)
{
    m_pimpl->numberOfFilesInIndex = n_files;
    statusBar()->showMessage(tr("Indexing complete. Found %1 file%2.")
        .arg(QString::number(n_files), ((n_files == 1) ? "" : "s")));
    m_pimpl->progressPage.labelProgress1low->setText(tr(""));
    m_pimpl->progressPage.labelProgress1high->setText(tr("Comparing scan to database index..."));
}

void MainWindow::onFileScanDiffCompleted()
{
    m_pimpl->blimpdb = m_pimpl->fileScanner.joinScanning();
    auto const& index = m_pimpl->fileScanner.getIndexList();
    std::uint64_t const total_size =
        std::accumulate(begin(index), end(index), uint64_t{ 0 }, [](std::uint64_t acc, FileInfo const& finfo)
            {
                return finfo.size + acc;
            });
    m_pimpl->fileDiffPage.labelSize->setText(tr("Total size: %1").arg(filesize_to_string(total_size)));
    m_pimpl->fileDiffPage.diffmodel->setFileIndexData(m_pimpl->fileScanner.getIndexList(),
                                                      m_pimpl->fileScanner.getIndexDiff());
    m_pimpl->central->setCurrentWidget(m_pimpl->fileDiffPage.widget);

}

void MainWindow::onFileDiffApprove()
{
    m_pimpl->createSnapshotPage.checked_files = m_pimpl->fileDiffPage.diffmodel->getCheckedFiles();

    auto const snapshots = m_pimpl->blimpdb->getSnapshots();
    m_pimpl->createSnapshotPage.editSnapshotName->setText(QString("Snapshot #%1").arg(snapshots.size()));
    m_pimpl->central->setCurrentWidget(m_pimpl->createSnapshotPage.widget);
}

void MainWindow::onCreateSnapshotRequest()
{
    QString const snapshot_name = m_pimpl->createSnapshotPage.editSnapshotName->text();
    QString snapshot_display = snapshot_name;
    if (snapshot_display.length() > 32) {
        snapshot_display.truncate(32);
        snapshot_display.append("...");
    }
    m_pimpl->progressPage.labelHeader->setText(
        QString("<div style=\"font-size:xx-large;font-weight:bold\">Creating snapshot '%1'...</div>")
        .arg(snapshot_display));
    disconnect(m_pimpl->progressPage.buttonCancel, &QPushButton::clicked, nullptr, nullptr);
    connect(m_pimpl->progressPage.buttonCancel, &QPushButton::clicked,
            this, &MainWindow::onCancelFileProcessing);
    m_pimpl->progressPage.labelProgress1high->setText(tr("Processing..."));
    m_pimpl->progressPage.labelProgress1low->setText(tr(""));
    m_pimpl->progressPage.progress1->setMaximum(static_cast<int>(m_pimpl->createSnapshotPage.checked_files.size()));
    m_pimpl->progressPage.progress1->setValue(0);
    m_pimpl->progressPage.progress2->show();
    m_pimpl->progressPage.buttonCancel->setEnabled(true);
    m_pimpl->central->setCurrentWidget(m_pimpl->progressPage.widget);
    BlimpDB::SnapshotId const snapshot_id = m_pimpl->blimpdb->addSnapshot(snapshot_name.toStdString());
    m_pimpl->fileProcessor.startProcessing(snapshot_id,
                                           std::move(m_pimpl->createSnapshotPage.checked_files),
                                           std::move(m_pimpl->blimpdb));
}

void MainWindow::onCreateSnapshotCancel()
{
    m_pimpl->createSnapshotPage.checked_files.clear();
    m_pimpl->blimpdb.reset();
    m_pimpl->central->setCurrentWidget(m_pimpl->welcomePage.widget);
}

void MainWindow::onCancelFileProcessing()
{
    m_pimpl->progressPage.buttonCancel->setEnabled(false);
    m_pimpl->fileProcessor.cancelProcessing();
}

void MainWindow::onProcessingCanceled()
{
    statusBar()->showMessage(tr("Processing canceled."), 5000);
    GHULBUS_ASSERT(!m_pimpl->blimpdb);
    m_pimpl->blimpdb = m_pimpl->fileProcessor.joinProcessing();
}

void MainWindow::onProcessingUpdateNewFile(std::uint64_t current_file_indexed, std::uint64_t current_file_size)
{
    m_pimpl->progressPage.progress1->setValue(current_file_indexed);
    m_pimpl->progressPage.progress2->setMaximum(current_file_size >> 20);
    m_pimpl->progressPage.progress2->setValue(0);
    m_pimpl->progressPage.labelProgress1low->setText(tr("Indexing %1 of %2...")
        .arg(QString::number(current_file_indexed), QString::number(m_pimpl->progressPage.progress1->maximum())));
}

void MainWindow::onProcessingUpdateHashProgress(std::uint64_t current_file_bytes_processed)
{
    m_pimpl->progressPage.progress2->setValue(current_file_bytes_processed >> 20);
}

void MainWindow::onProcessingUpdateHashCompleted(std::uint64_t current_file_indexed, std::uint64_t current_file_size)
{
    m_pimpl->progressPage.progress2->setValue(0);
    m_pimpl->progressPage.labelProgress1low->setText(tr("Processing %1 of %2...")
        .arg(QString::number(current_file_indexed), QString::number(m_pimpl->progressPage.progress1->maximum())));
}

void MainWindow::onProcessingUpdateFileProgress(std::uint64_t current_file_bytes_processed)
{
    m_pimpl->progressPage.progress2->setValue(current_file_bytes_processed >> 20);
}

void MainWindow::onProcessingCompleted()
{
    m_pimpl->blimpdb = m_pimpl->fileProcessor.joinProcessing();
    m_pimpl->progressPage.labelHeader->setText("Done Processing.");
}

void MainWindow::onFileScanChecksumUpdate(std::uint64_t n_files)
{
    m_pimpl->statusBar.progressLabel->setText(tr("Scanning %1/%2")
        .arg(QString::number(n_files), QString::number(m_pimpl->numberOfFilesInIndex)));
    m_pimpl->statusBar.progressBar->setValue(n_files);
}

void MainWindow::onFileScanChecksumCompleted()
{
    m_pimpl->fileScanner.cancelScanning();
    m_pimpl->fileDiffPage.diffmodel->setFileIndexData(m_pimpl->fileScanner.getIndexList(), m_pimpl->fileScanner.getIndexDiff());
}

void MainWindow::onFileRetrievalRequested(FileElementId file_id)
{
    auto const storage_infos = m_pimpl->blimpdb->getFileStorageInfo(file_id);
    auto const file_hash = m_pimpl->blimpdb->getFileHash(file_id);
    auto const file_info = m_pimpl->blimpdb->getFileInfo(file_id);
    if ((!file_hash) || (!file_info) || (storage_infos.empty())) {
        GHULBUS_THROW(Exceptions::DatabaseError{}, "File not in database");
    }
    m_pimpl->fileProcessor.retrieveFile(boost::filesystem::path{"blimp_out_dir"} / file_info->path.filename(),
                                        *file_info, *file_hash, storage_infos);
}
