#include "loadimage.hpp"

#include <iostream>

LoadImage::LoadImage() : m_sem(0)
{
        start();
}

LoadImage::~LoadImage()
{

}

void
LoadImage::load(const QString &path)
{
        loader l;

        l.m_loadImage = this;
        l.m_file      = path;

        QMutexLocker locker(&m_mutexQueue);

        m_funcs.enqueue(l);
        m_sem.release();
}

void
LoadImage::scale(int width, int height)
{
        scaler s;

        s.m_loadImage = this;
        s.m_width     = width;
        s.m_height    = height;


        QMutexLocker locker(&m_mutexQueue);

        m_funcs.enqueue(s);
        m_sem.release();
}

QImage
LoadImage::getImage()
{
        QMutexLocker locker(&m_mutexImg);
        QImage image = m_image.copy(QRect());

        return image;
}

QImage
LoadImage::getOriginal()
{
        QMutexLocker locker(&m_mutexImg);
        QImage image = m_original.copy(QRect());

        return image;
}

void
LoadImage::run()
{
        boost::function<void ()> func;

        for (;;) {
                m_sem.acquire();

                {
                        QMutexLocker locker(&m_mutexQueue);
                        func = m_funcs.dequeue();
                }

                func();
                emit finished();
        }
}

void
LoadImage::loader::operator() ()
{
        QImage img, img2;

        img.load(m_file);

        QMutexLocker (&m_loadImage->m_mutexImg);
        m_loadImage->m_original = img;
        m_loadImage->m_image    = img;
}

void
LoadImage::scaler::operator() ()
{
        QImage img1, img2;

        {
                QMutexLocker (&m_loadImage->m_mutexImg);
                img1 = m_loadImage->m_original;
                img1 = img1.copy(QRect());
        }

        if (img1.width() <= m_width && img1.height() <= m_height) {
                img2 = img1;
        } else {
                img2 = img1.scaled(m_width, m_height,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
        }

        {
                QMutexLocker (&m_loadImage->m_mutexImg);
                m_loadImage->m_image = img2;
        }
}
