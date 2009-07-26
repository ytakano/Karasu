#include "previewarea.hpp"

#include "thumbnail.hpp"

#include <iostream>

#include <QLabel>
#include <QResizeEvent>

PreviewArea::PreviewArea()
        : m_fit(true), m_mustResize(false), m_resizing(false)
{
        m_picture = new picture(this);

        this->setWidget(m_picture);

        QObject::connect(&m_loadImage, SIGNAL(finished()),
                         this, SLOT(showImage()));
}

PreviewArea::~PreviewArea()
{

}

void
PreviewArea::showImage()
{
        QImage image = m_loadImage.getImage();

        m_resizing = false;

        if (m_mustResize) {
                m_mustResize = false;
                m_resizing   = true;
                m_loadImage.scale(viewport()->width(),
                                  viewport()->height());

                return;
        }

        if (m_fit &&
            (image.width() > viewport()->width() ||
             image.height() > viewport()->height())) {
                m_loadImage.scale(viewport()->width(), viewport()->height());
        } else {
                m_picture->setPixmap(QPixmap::fromImage(image));
                m_picture->adjustSize();
        }
}

PreviewArea::picture::picture(PreviewArea *preview)
{
        m_preview = preview;
}

PreviewArea::picture::~picture()
{

}

void
PreviewArea::picture::mousePressEvent(QMouseEvent *event)
{
        if (m_preview->m_fit) {
                QImage image = m_preview->m_loadImage.getOriginal();
                m_preview->m_picture->setPixmap(QPixmap::fromImage(image));
                m_preview->m_picture->adjustSize();
        } else {
                m_preview->m_loadImage.scale(
                        m_preview->viewport()->width(),
                        m_preview->viewport()->height());
        }

        m_preview->m_fit = ! m_preview->m_fit;
}

void
PreviewArea::resizeEvent(QResizeEvent *event)
{
        if (m_fit) {
                if (! m_resizing) {
                        m_resizing = true;
                        m_loadImage.scale(viewport()->width(),
                                          viewport()->height());
                } else {
                        m_nextResize = event->size();
                        m_mustResize = true;
                }
        } else {
                QScrollArea::resizeEvent(event);
        }
}

void
PreviewArea::changedFocus(Thumbnail *thumb)
{
        m_loadImage.load(thumb->getPath());
}
