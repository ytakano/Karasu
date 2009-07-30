#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <iostream>

#include <QMainWindow>
#include <QScrollArea>
#include <QPixmap>

#include "loadimage.hpp"

class QSplitter;
class QTreeView;
class QVBoxLayout;
class QHBoxLayout;
class QWidget;
class QLineEdit;
class QLabel;
class QScrollArea;
class QGridLayout;
class QComboBox;
class QDockWidget;
class QDirModel;
class QModelIndex;

class ThumbArea;
class PreviewArea;


class MainWindow : public QMainWindow {
        Q_OBJECT

public:
        MainWindow();
        virtual ~MainWindow();

public slots:
        void setCurrentPath(const QModelIndex &index);
        void enableSort();

protected:
        void closeEvent(QCloseEvent *event);

private:
        void initWidget();
        void initDir();

        void readSettings();
        void writeSettings();

        QDirModel *m_dirModel;
        LoadImage  m_loadImage;
        bool       m_fit;

        // main
        QWidget     *m_mainWidget;
        QVBoxLayout *m_mainLayout;

        // top
        QHBoxLayout *m_topLayout;
        QLabel      *m_locationLabel;
        QLineEdit   *m_locationEdit;
        
        // bottom
        QSplitter   *m_splitter;

        // bottom left
        QTreeView   *m_dirTree;

        // bottom right
        QWidget     *m_rightWidget;
        QVBoxLayout *m_rightLayout;

        // bottom right bottom
        ThumbArea   *m_thumbArea;

        // bottom right top
        QHBoxLayout *m_rightTopLayout;
        QComboBox   *m_comboSort;

        // dock
        PreviewArea *m_preview;
        QDockWidget *m_dock;

        // status bar
        QLabel      *m_status;
};

#endif // MAINWINDOW_HPP
