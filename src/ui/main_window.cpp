
#include <ui/main_window.hpp>

MainWindow::MainWindow()
    :m_central(new QWidget(this)), m_treeview(new QTreeView), m_model(new FileSystemModel(this)),
     m_layout(new QBoxLayout(QBoxLayout::Direction::LeftToRight, m_central))
{
    m_model->setRootPath("D:");
    m_treeview->setModel(m_model);
    m_treeview->resize(width(), height());
    m_layout->addWidget(m_treeview);
    m_central->setLayout(m_layout);
    setCentralWidget(m_central);
    show();
}

MainWindow::~MainWindow()
{
}
