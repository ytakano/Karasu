#ifndef THUMBSCROLL_HPP
#define THUMBSCROLL_HPP

#include "loadthumbnails.hpp"

#include <vector>

#include <QScrollArea>
#include <QString>

class QWidget;

class MainWindow;
class Thumbnail;
class PreviewArea;

class ThumbArea : public QScrollArea {
        Q_OBJECT

public:
        ThumbArea(PreviewArea *preview, QLabel *status);
        virtual ~ThumbArea();

        void setCurrentPath(const QString &path);
        QString getCurrentPath() { return m_currentPath; }

signals:
        void currentPathChanged();

public slots:
        void loadImages();
        void loadImage(const QString &path);
        void changedFocus(Thumbnail *thumb);

protected:
        virtual void scrollContentsBy(int dx, int dy);
        virtual void resizeEvent(QResizeEvent *event);

private:
        static const int margin;
        static const int spacing;

        LoadThumbnails m_loadThumbs;

        PreviewArea *m_preview;
        QWidget *m_widget;
        QString  m_currentPath;
        QLabel  *m_status;

        std::vector<Thumbnail*> *m_pThumb;
        std::vector<Thumbnail*>  m_thumbPool;
        std::vector<Thumbnail*>  m_thumbByName;


        void repaintThumbnails();
        void clearThumbnails();
        void drawThumbnails(std::vector<Thumbnail*> &thumb);
};

#endif // THUMBSCROLL_HPP