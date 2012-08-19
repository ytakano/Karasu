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
                m_thumbByCorr.push_back(thumb);
        }

        m_loadThumbs.setEmitSignal(true);

        if (m_loadThumbs.readEverything()) {
                m_loadThumbs.stop();
                sortByCorr();
                drawThumbnails(m_thumbByCorr);
                emit loadFinished();
                return;
        }

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
        m_thumbByCorr.clear();

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

bool
ThumbArea::compareByName::operator() (const Thumbnail *lhs,
                                      const Thumbnail *rhs) const
{
        return lhs->getFile() < rhs->getFile();
}

bool
ThumbArea::compareByCorr::operator() (const Thumbnail *lhs,
                                      const Thumbnail *rhs) const
{
        int znum;
        boost::shared_array<uint32_t> z1, z2;

	if (lhs->getZnum() == 0 && rhs->getZnum() == 0) {
	        return false;
	} else if (lhs->getZnum() == 0) {
	        return false;
	} else if (rhs->getZnum() == 0) {
	        return true;
	}
	  

        z1 = lhs->getZ();
        z2 = rhs->getZ();

        znum = lhs->getZnum();

        for (int i = znum - 1; i >= 0; i--) {
                if (z1[i] < z2[i])
                        return true;
                else if (z1[i] > z2[i])
                        return false;
        }

        return true;

/*        double size1, size2;
        int alpha1[NUM_COLOR], alpha2[NUM_COLOR];
        int beta1[NUM_COLOR], beta2[NUM_COLOR];
        uint32_t z1[11], z2[11];
        boost::shared_ptr<ccv_ret> ccvr1, ccvr2;

        ccvr1 = lhs->getCCV();
        ccvr2 = rhs->getCCV();

        size1 = (double)(lhs->getWidth() * lhs->getHeight());
        size2 = (double)(rhs->getWidth() * rhs->getHeight());

        for (int i = 0; i < NUM_COLOR; i++) {
                alpha1[i] = (int)ceil((double)ccvr1->alpha[i] / size1 * 64);
                alpha2[i] = (int)ceil((double)ccvr2->alpha[i] / size2 * 64);
                beta1[i] = (int)ceil((double)ccvr1->beta[i] / size1 * 64);
                beta2[i] = (int)ceil((double)ccvr2->beta[i] / size2 * 64);
        }

        // map to Z-order curve
        memset(z1, 0, sizeof(z1));
        memset(z2, 0, sizeof(z2));

        for (int i = 0; i < 6; i++) {
                for (int j = 0; j < NUM_COLOR; j++) {
                        int shift;
                        int idx;

                        shift = i * NUM_COLOR - i + j * 2;
                        idx   = (shift + i) / 32;
                        shift = shift % 32;

                        z1[idx] += (alpha1[j] & (1 << i)) << shift;
                        z2[idx] += (alpha2[j] & (1 << i)) << shift;


                        shift = i * NUM_COLOR - i + j * 2 + 1;
                        idx   = (shift + i) / 32;
                        shift = shift % 32;

                        z1[idx] += (beta1[j]  & (1 << i)) << shift;
                        z2[idx] += (beta2[j]  & (1 << i)) << shift;
                }
        }

        for (int i = 10; i >= 0; i--) {
                if (z1[i] < z2[i])
                        return true;
                else if (z1[i] > z2[i])
                        return false;
        }

        return true;
*/
}

void
ThumbArea::sortByCorr()
{
        std::sort(m_thumbByCorr.begin(), m_thumbByCorr.end(),
                  compareByCorr());
}

void
ThumbArea::sortByName()
{
        std::sort(m_thumbByCorr.begin(), m_thumbByCorr.end(),
                  compareByName());
}
