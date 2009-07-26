#ifndef LOADTHUMBNAILS_HPP
#define LOADTHUMBNAILS_HPP

#include "thumbnail.hpp"
#include "imgprocess.hpp"

#include <boost/shared_ptr.hpp>

#include <QHash>
#include <QThread>
#include <QImage>
#include <QDir>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <QDateTime>
#include <QQueue>

struct image {
        QString   path;
        QString   file;
        QImage    img;
        QDateTime date;
        int       width, height;
        bool      loaded;
        qint64    size;
        boost::shared_ptr<ccv_ret> ccvr;
};

struct imageCache {
        QDateTime date;
        QImage    img;
        int       width;
        int       height;
        boost::shared_ptr<ccv_ret> ccvr;
};

#include <map>

class LoadThumbnails : public QThread {
        Q_OBJECT

public:
        LoadThumbnails();
        virtual ~LoadThumbnails();

        void stop();
        void load(const QString &dir);
        void setEmitSignal(bool isEmit);
        image* next();
        bool readEverything();

signals:
        void imageLoaded(QString path);

protected:
        virtual void run();

private:
        static int max_cache;

        QString m_path;
        QDir    m_dir;

        QWaitCondition m_waitStopped;
        QMutex m_mutexStopped;
        QMutex m_mutexRun;
        QMutex m_mutexSignal;

        QHash<QString, imageCache> m_imageCache;
        QQueue<QString> m_cacheQueue;

        image *m_images;

        volatile int m_read;
        volatile int m_num;
        volatile int m_numLoaded;

        volatile bool m_emittedSignal;

        volatile bool m_stopped;
};

#endif // LOADTHUMBNAILS_HPP
