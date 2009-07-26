#include "mainwindow.hpp"

#include "thumbnail.hpp"
#include "thumbarea.hpp"
#include "loadthumbnails.hpp"
#include "previewarea.hpp"

#include <iostream>
#include <qtgui>


std::vector<QWidget*> p_wid;

MainWindow::MainWindow() : m_fit(true)
{
        QTextCodec::setCodecForTr(QTextCodec::codecForLocale()); 

        initWidget();
        initDir();

        setWindowTitle(tr("Karasu for Image Collectors"));

        readSettings();
}

MainWindow::~MainWindow()
{

}

void
MainWindow::closeEvent(QCloseEvent *event)
{
        writeSettings();
        event->accept();
}


void
MainWindow::readSettings()
{
        QRect rect;
        QSettings settings("Project Karasu", "karasu");

        rect = settings.value("mainwindow/geometry",
                              QRect(0, 0, 750, 700)).toRect();

        move(rect.topLeft());
        resize(rect.size());


        rect = settings.value("preview/geometry",
                              QRect(755, 0, 250, 350)).toRect();

        m_dock->move(rect.topLeft());
        m_dock->resize(rect.size());


        m_splitter->restoreState(settings.value("splitter/state").toByteArray());


        int width = settings.value("tree/width", 250).toInt();
        m_dirTree->setColumnWidth(0, width);



        QString path = settings.value("thumbnail/path", QDir::currentPath()).toString();

        QModelIndex index = m_dirModel->index(path);
        m_dirTree->expand(index);
        m_dirTree->scrollTo(index);


        QItemSelectionModel *selection = m_dirTree->selectionModel();
        selection->select(index, QItemSelectionModel::SelectCurrent);
        selection->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

void
MainWindow::writeSettings()
{
        QSettings settings("Project Karasu", "karasu");

        settings.setValue("mainwindow/geometry", geometry());
        settings.setValue("preview/geometry", m_dock->geometry());
        settings.setValue("splitter/state", m_splitter->saveState());
        settings.setValue("tree/width", m_dirTree->columnWidth(0));
        settings.setValue("thumbnail/path", m_thumbArea->getCurrentPath());
}

void
MainWindow::initDir()
{
        m_dirModel = new QDirModel;
        
        m_dirModel->setReadOnly(false);
        m_dirModel->setSorting(QDir::Name);
        m_dirModel->setFilter(QDir::AllDirs |
                              QDir::Drives  |
                              QDir::NoDotAndDotDot);

        m_dirTree->setModel(m_dirModel);
        m_dirTree->setColumnHidden(1, true);
        m_dirTree->setColumnHidden(2, true);
        m_dirTree->setExpandsOnDoubleClick(true);

        m_dirTree->header()->setStretchLastSection(true);
        m_dirTree->header()->setClickable(false);


        QItemSelectionModel *selection = m_dirTree->selectionModel();

        QObject::connect(selection,
                         SIGNAL(currentChanged(const QModelIndex &,
                                               const QModelIndex &)),
                         this,
                         SLOT(setCurrentPath(const QModelIndex &)));
}

void
MainWindow::initWidget()
{
        // status bar
        m_status = new QLabel(tr("ready"));

        statusBar()->addWidget(m_status, 1);


        // main
        m_mainWidget = new QWidget();
        m_mainLayout = new QVBoxLayout();

        m_mainWidget->setLayout(m_mainLayout);

        setCentralWidget(m_mainWidget);


        // top
        m_topLayout     = new QHBoxLayout;
        m_locationLabel = new QLabel(tr("場所："));
        m_locationEdit  = new QLineEdit;

        m_topLayout->addWidget(m_locationLabel);
        m_topLayout->addWidget(m_locationEdit);

        m_mainLayout->addLayout(m_topLayout);


        // bottom
        m_splitter = new QSplitter();

        m_mainLayout->addWidget(m_splitter);


        // bottom left
        m_dirTree = new QTreeView;

        m_splitter->addWidget(m_dirTree);


        // bottom right
        m_rightWidget = new QWidget;
        m_rightLayout = new QVBoxLayout;

        m_rightWidget->setLayout(m_rightLayout);

        m_splitter->addWidget(m_rightWidget);


        // bottom right top
        m_rightTopLayout = new QHBoxLayout;
        m_comboSort      = new QComboBox;

        m_rightTopLayout->addWidget(m_comboSort);

        m_rightLayout->addLayout(m_rightTopLayout);


        // bottom right bottom
        m_preview   = new PreviewArea;
        m_thumbArea = new ThumbArea(m_preview, m_status);

        //m_thumbArea->setCurrentPath(QDir::currentPath());

        m_rightLayout->addWidget(m_thumbArea);

        m_splitter->setStretchFactor(1, 1);


        // dock
        m_dock    = new QDockWidget;

        m_dock->setAllowedAreas(Qt::NoDockWidgetArea);
        m_dock->setWidget(m_preview);
        m_dock->setFeatures(QDockWidget::DockWidgetMovable |
                            QDockWidget::DockWidgetFloatable |
                            QDockWidget::DockWidgetClosable);
        m_dock->setMinimumSize(QSize(200, 200));

        addDockWidget(Qt::BottomDockWidgetArea, m_dock);

        m_dock->setFloating(true);
}

void
MainWindow::setCurrentPath(const QModelIndex &index)
{
        QString path = m_dirModel->filePath(index);

        m_locationEdit->setText(path);

        m_dirModel->refresh(index);

        m_thumbArea->setCurrentPath(path);
}

