#ifndef BLIMP_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP
#define BLIMP_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP

#include <QMainWindow>

#include <cstdint>
#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

    ~MainWindow();

public slots:
    void onButtonClicked();
    void onCancelClicked();
    void onFileScanIndexingCompleted(std::uintmax_t n_files);
    void onFileScanIndexingUpdate(std::uintmax_t n_files);
    void onFileScanChecksumUpdate(std::uintmax_t n_files);

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_pimpl;
};

#endif
