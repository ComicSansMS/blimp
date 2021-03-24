#ifndef BLIMP_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP
#define BLIMP_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP

#include <QMainWindow>

#include <cstdint>
#include <memory>

class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;

public:
    MainWindow();

    ~MainWindow();

    void closeEvent(QCloseEvent* close_event) override;

public slots:
    void onNewDatabase();
    void onOpenDatabase();
    void onStartFileScan();
    void onCancelFileScan();
    void onFileScanIndexingUpdate(std::uint64_t n_files);
    void onFileScanIndexingCompleted(std::uint64_t n_files);
    void onFileScanDiffCompleted();
    void onFileScanChecksumUpdate(std::uint64_t n_files);
    void onFileScanChecksumCompleted();
    void onFileDiffApprove();
    void onProcessingUpdateNewFile(std::uint64_t current_file_indexed, std::uint64_t current_file_size);
    void onProcessingUpdateHashProgress(std::uint64_t current_file_bytes_processed);
    void onProcessingUpdateHashCompleted(std::uint64_t current_file_indexed, std::uint64_t current_file_size);
    void onProcessingUpdateFileProgress(std::uint64_t current_file_bytes_processed);
    void onProcessingCompleted();
    void onCreateSnapshotRequest();
    void onCreateSnapshotCancel();
    void onCancelFileProcessing();
    void onProcessingCanceled();


};

#endif
