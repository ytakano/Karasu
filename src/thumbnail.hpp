#ifndef THUMBNAIL_HPP
#define THUMBNAIL_HPP

#include "imgprocess.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/unordered_set.hpp>

#include <QFrame>
#include <QLabel>
#include <QDateTime>

class QMouseEvent;
class QVBoxLayout;
class QPaintEvent;

#define Z_ORDER_NUM 11


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

        QString getPath() const  { return m_path; }
        QString getFile() const  { return m_file; }
        int     getWidth() const { return m_width; }
        int     getHeight() const { return m_height; }
        QString getDate() const  { return m_date.toString("yyyy/MM/dd hh:mm"); }
        qint64  getSize() const  { return m_size; }
        boost::shared_ptr<ccv_ret> getCCV() const { return m_ccvr; }
        boost::shared_array<uint32_t> getZ() const { return m_z; }
        int     getZnum() const { return m_znum; }

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
        boost::shared_ptr<ccv_ret> m_ccvr;
        boost::shared_ptr<hog_ret> m_hogr;
        boost::shared_array<uint32_t> m_z;
        int m_znum;

        void zOrder();
};

#endif // THUNBNAIL_HPP
