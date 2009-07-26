#ifndef THUMBNAIL_HPP
#define THUMBNAIL_HPP

#include <boost/unordered_set.hpp>

#include <QFrame>
#include <QLabel>
#include <QDateTime>

class QMouseEvent;
class QVBoxLayout;
class QPaintEvent;


struct image;

class Thumbnail : public QFrame {
        Q_OBJECT

public:
        Thumbnail();
        virtual ~Thumbnail();

        static int getImgWidth()  { return imgWidth; }
        static int getImgHeight() { return imgHeight; }

        void setFocus(bool focus);
        void setImage(image &img);
        void clearPixmap();

        QString getPath()   { return m_path; }
        QString getFile()   { return m_file; }
        int     getWidth()  { return m_width; }
        int     getHeight() { return m_height; }
        QString getDate()   { return m_date.toString("yyyy/MM/dd hh:mm"); }
        qint64  getSize()   { return m_size; }

        // for debug
        int x, y;

signals:
        void focused(Thumbnail *thumb);

protected:
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void paintEvent(QPaintEvent * event);

private:
        static boost::unordered_set<Thumbnail*> focusing;


        static int imgWidth;
        static int imgHeight;


        class picture : public QLabel {
        public:
                picture(Thumbnail *thumb);
                virtual ~picture();

        protected:
                virtual void mousePressEvent(QMouseEvent *event);

        private:
                Thumbnail *m_thumb;
        };

        QVBoxLayout *m_layout;
        picture     *m_picture;
        QLabel      *m_nameLabel;
        QLabel      *m_sizeLabel;

        QString   m_path;
        QString   m_file;
        QDateTime m_date;
        int       m_width, m_height;
        qint64    m_size;
};

#endif // THUNBNAIL_HPP
