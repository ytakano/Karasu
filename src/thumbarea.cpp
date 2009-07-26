#include "thumbarea.hpp"

#include "mainwindow.hpp"
#include "thumbnail.hpp"
#include "previewarea.hpp"

#include <boost/foreach.hpp>

#include <iostream>

#include <QWidget>
#include <QScrollBar>
#include <QLabel>

const int ThumbArea::margin  = 8;
const int ThumbArea::spacing = 4;

ThumbArea::ThumbArea(PreviewArea *preview, QLabel *status)
        : m_preview(preview), m_status(status), m_pThumb(NULL)
{
        m_widget = new QWidget;

        setWidget(m_widget);

        QObject::connect(&m_loadThumbs, SIGNAL(imageLoaded(QString)),
                         this, SLOT(loadImage(QString)));

        QObject::connect(this, SIGNAL(currentPathChanged()),
                         this, SLOT(loadImages()));
}

ThumbArea::~ThumbArea()
{
        m_loadThumbs.stop();
}

void
ThumbArea::scrollContentsBy(int dx, int dy)
{
        QScrollArea::scrollContentsBy(dx, dy);

        if (m_pThumb == NULL)
                return;

        repaintThumbnails();
}

void
ThumbArea::resizeEvent(QResizeEvent *event)
{
        QScrollArea::resizeEvent(event);


        if (m_pThumb == NULL)
                return;

        drawThumbnails(*m_pThumb);
}

void
ThumbArea::drawThumbnails(std::vector<Thumbnail*> &thumb)
{
        QSize size;
        int x, y;
        int column;
        int width, height;


        if (thumb.size() == 0)
                return;


        size = thumb[0]->sizeHint();

        width  = size.width() + spacing;
        height = size.height() + spacing;

        column   = (viewport()->width() - margin * 2) / width;

        if (column < 1)
                column = 1;

        if (&thumb == m_pThumb) {
        }

        x = y = 0;
        BOOST_FOREACH(Thumbnail *t, thumb) {
                t->move(margin + width * x, margin + height * y);
                t->show();
                t->repaint();
                t->x = x;
                t->y = y;

                x++;
                if (x == column) {
                        x = 0;
                        y++;
                }
        }

        if (x > 0)
                y++;

        m_widget->resize(margin * 2 + width * column,
                         margin * 2 + height * y);
        m_widget->repaint();


        m_pThumb = &thumb;
}

void
ThumbArea::repaintThumbnails()
{
        std::vector<Thumbnail*> thumb(*m_pThumb);
        QSize size;
        int column;
        int width, height;


        m_widget->repaint();
        
        if (m_pThumb == NULL || thumb.size() == 0)
                return;


        size = thumb[0]->sizeHint();

        width  = size.width() + spacing;
        height = size.height() + spacing;

        column = (viewport()->width() - margin * 2) / width;

        if (column < 1)
                column = 1;


        int begin, end;

        begin = (-m_widget->pos().y() - margin) / height;
        end   = begin + viewport()->height() / height + 2;

        for (int i = begin; i < end; i++) {
                for (int j = 0; j < column; j++) {
                        int n = column * i + j;

                        if (n >= (int)thumb.size())
                                return;

                        thumb[n]->repaint();
                }
        }
}

void
ThumbArea::loadImage(const QString &path)
{
        Thumbnail *thumb;
        image     *img;

        if (m_currentPath != path)
                return;

        while ((img = m_loadThumbs.next()) != NULL) {
                if (m_thumbPool.size() == 0) {
                        thumb = new Thumbnail;
                        thumb->setParent(m_widget);

                        QObject::connect(thumb,
                                         SIGNAL(focused(Thumbnail *)),
                                         m_preview,
                                         SLOT(changedFocus(Thumbnail*)));

                        QObject::connect(thumb,
                                         SIGNAL(focused(Thumbnail *)),
                                         this,
                                         SLOT(changedFocus(Thumbnail*)));
                } else {
                        thumb = m_thumbPool.back();
                        m_thumbPool.pop_back();
                }

                thumb->setFocus(false);

                thumb->setImage(*img);

                m_thumbByName.push_back(thumb);
        }

        m_loadThumbs.setEmitSignal(true);

        if (m_loadThumbs.readEverything())
                m_loadThumbs.stop();

        drawThumbnails(m_thumbByName);
}

void
ThumbArea::setCurrentPath(const QString &path)
{
        QPixmap pixmap;

        if (m_currentPath == path)
                return;

        m_currentPath = path;

        m_loadThumbs.stop();

        BOOST_FOREACH(Thumbnail *thumb, m_thumbByName) {
                thumb->hide();
                thumb->setFocus(false);
                thumb->clearPixmap();

                m_thumbPool.push_back(thumb);
        }

        m_pThumb = NULL;
        m_thumbByName.clear();

        emit currentPathChanged();
}

void
ThumbArea::loadImages()
{
        m_loadThumbs.load(m_currentPath);
}

void
ThumbArea::changedFocus(Thumbnail *thumb)
{
        qint64  size;
        QString file;
        QString sizeStr;
        QString date;
        QString width;
        QString height;

        file = thumb->getPath();
        date = thumb->getDate();


        size = thumb->getSize();
        if (size > 1024 * 1024) {
                sizeStr.setNum(size / (1024 * 1024));
                sizeStr += "MB";
        } else if (size > 1024) {
                sizeStr.setNum(size / 1024);
                sizeStr += "KB";
        } else {
                sizeStr.setNum(size);
                sizeStr += "B";
        }

        width.setNum(thumb->getWidth());
        height.setNum(thumb->getHeight());

        m_status->setText(file + " - " +
                          width + "x" + height +
                          " (" + sizeStr + ") - " + date);
}
