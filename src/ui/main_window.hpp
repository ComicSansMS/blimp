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

public slots:
    void onNewDatabase();
    void onOpenDatabase();
    void onStartFileScan();
    void onCancelFileScan();
    void onFileScanIndexingUpdate(std::uintmax_t n_files);
    void onFileScanIndexingCompleted(std::uintmax_t n_files);
    void onFileScanDiffCompleted();
    void onFileScanChecksumUpdate(std::uintmax_t n_files);
    void onFileScanChecksumCompleted();
    void onFileDiffApprove();
    void onProcessingUpdateNewFile(std::uintmax_t current_file_indexed, std::uintmax_t current_file_size);
    void onProcessingUpdateFileProgress(std::uintmax_t current_file_bytes_processed);
    void onProcessingCompleted();


};

#endif
