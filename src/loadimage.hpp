#ifndef LOADIMAGE_HPP
#define LOADIMAGE_HPP

#include <boost/function.hpp>

#include <QThread>
#include <QImage>
#include <QQueue>
#include <QPixmap>
#include <QMutex>
#include <QSemaphore>

class LoadImage : public QThread {
        Q_OBJECT

public:
        LoadImage();
        virtual ~LoadImage();

        void load(const QString &path);
        void scale(int width, int height);
        QImage getImage();
        QImage getOriginal();

signals:
        void finished();

protected:
        virtual void run();

private:
        QQueue<boost::function<void ()> > m_funcs;

        QImage m_image;
        QImage m_original;
        QMutex m_mutexQueue;
        QMutex m_mutexImg;
        QSemaphore m_sem;
        

        class loader {
        public:
                loader() { }

                void operator() ();

                LoadImage *m_loadImage;

                QString m_file;
        };

        class scaler {
        public:
                scaler() { }

                void operator() ();

                LoadImage *m_loadImage;

                int m_width;
                int m_height;
        };
};

#endif // LOADIMAGE_HPP
