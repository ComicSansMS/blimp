
#include <ui/main_window.hpp>

MainWindow::MainWindow()
    :m_central(new QWidget(this)), m_treeview(new QTreeView), m_model(new FileSystemModel(this)),
     m_layout(new QBoxLayout(QBoxLayout::Direction::LeftToRight, m_central))
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
    m_central->setLayout(m_layout);
    setCentralWidget(m_central);
    connect(m_treeview, &QTreeView::clicked, m_model, &FileSystemModel::itemClicked);
    show();
}

MainWindow::~MainWindow()
{
}
