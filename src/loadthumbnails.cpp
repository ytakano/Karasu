#include "loadthumbnails.hpp"

#include "thumbnail.hpp"

#include <iostream>

#include <QtGui>

int LoadThumbnails::max_cache = 4096;

LoadThumbnails::LoadThumbnails()
        : m_dir(QDir::root()),
          m_images(NULL),
          m_stopped(false)
{
        QList<QByteArray> formats = QImageReader::supportedImageFormats();
        QStringList filters;

        for (int i = 0; i < formats.size(); ++i) {
                QString format("*.");

                format.append(formats.at(i));

                filters << format;
        }

        m_dir.setNameFilters(filters);
        m_dir.setSorting(QDir::Name);
}

LoadThumbnails::~LoadThumbnails()
{

}

void
LoadThumbnails::load(const QString &dir)
{
        QDir d(dir);

        if (d.exists() == false)
                return;

        stop();

        {
                QMutexLocker locker(&m_mutexRun);
                m_stopped = false;
        }

        m_path = dir;
        m_dir.cd(dir);

        start();
}

void
LoadThumbnails::run()
{
        QMutexLocker locker(&m_mutexRun);

        QStringList files = m_dir.entryList();

        m_images    = new image[files.size()];
        m_read      = 0;
        m_num       = files.size();
        m_numLoaded = 0;


        setEmitSignal(true);

        for (int i = 0; i < files.size(); i++) {
                {
                        QMutexLocker locker(&m_mutexStopped);
                        if (m_stopped) {
                                m_stopped = false;
                                delete[] m_images;
                                return;
                        }
                }

                QString    path(m_dir.absoluteFilePath(files.at(i)));
                QFileInfo  fileInfo(path);
                QImage     img;
                imageCache cache;
                boost::shared_ptr<ccv_ret> ccvr(new ccv_ret);
                boost::shared_ptr<hog_ret> hogr(new hog_ret);

                if (! fileInfo.exists()) {
                        m_images[i].loaded = false;
                        goto err;
                }

                if (m_imageCache.contains(path) &&
                    fileInfo.lastModified() == m_imageCache[path].date) {
                        cache = m_imageCache[path];

                        m_images[i].img    = cache.img;
                        m_images[i].width  = cache.width;
                        m_images[i].height = cache.height;
                        m_images[i].ccvr   = cache.ccvr;
                        m_images[i].hogr   = cache.hogr;
                } else {
                        if (! img.load(path)) {
                                m_images[i].loaded = false;
                                goto err;
                        }

                        if (img.width() <= Thumbnail::getImgWidth() &&
                            img.height() <= Thumbnail::getImgHeight()) {
                                m_images[i].img = img;
                        } else {
                                m_images[i].img = img.scaled(
                                        Thumbnail::getImgWidth(),
                                        Thumbnail::getImgHeight(),
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation);
                        }

                        m_images[i].width  = img.width();
                        m_images[i].height = img.height();

                        QImage qimg = gaussianFilter(m_images[i].img, 3.0);

                        ccv(qimg, *ccvr);
                        m_images[i].ccvr   = ccvr;

                        histogramOrientedGradients(qimg, 9, 6, 6, 3, 3, *hogr);
                        m_images[i].hogr   = hogr;

                        // add to cache
                        cache.date   = fileInfo.lastModified();
                        cache.img    = m_images[i].img;
                        cache.width  = img.width();
                        cache.height = img.height();
                        cache.ccvr   = ccvr;
                        cache.hogr   = hogr;

                        m_imageCache[path] = cache;
                        m_cacheQueue.enqueue(path);

                        if (m_cacheQueue.size() == max_cache) {
                                QString rm = m_cacheQueue.dequeue();
                                m_imageCache.remove(rm);
                        }
                }

                m_images[i].file   = files.at(i);
                m_images[i].path   = path;
                m_images[i].size   = fileInfo.size();
                m_images[i].date   = fileInfo.lastModified();
                m_images[i].loaded = true;

        err:
                m_numLoaded++;

                {
                        QMutexLocker locker(&m_mutexSignal);
                        if (! m_emittedSignal) {
                                emit imageLoaded(m_path);
                                m_emittedSignal = true;
                        }
                }
        }

        {
                QMutexLocker locker(&m_mutexStopped);
                if (m_stopped) {
                        m_stopped = false;
                        delete[] m_images;
                        return;
                }

                m_waitStopped.wait(&m_mutexStopped);
                m_stopped = false;
                delete[] m_images;
        }
}

void
LoadThumbnails::stop()
{
        QMutexLocker locker(&m_mutexStopped);
        m_stopped = true;

        m_waitStopped.wakeAll();
}

image*
LoadThumbnails::next()
{
        image *img;

        if (m_read < m_numLoaded) {
                img = &m_images[m_read++];
        } else {
                img = NULL;
        }

        return img;
}

void
LoadThumbnails::setEmitSignal(bool isEmit)
{
        QMutexLocker locker(&m_mutexSignal);
        m_emittedSignal = ! isEmit;
}

bool
LoadThumbnails::readEverything()
{
        if (m_read >= m_num)
                return true;

        return false;
}
