#ifndef PREVIEWAREA_HPP
#define PREVIEWAREA_HPP

#include "loadimage.hpp"

#include <QScrollArea>
#include <QLabel>

class Thumbnail;

class PreviewArea : public QScrollArea {
        Q_OBJECT

public:
        PreviewArea();
        virtual ~PreviewArea();

public slots:
        void showImage();
        void changedFocus(Thumbnail *thumb);

protected:
        virtual void resizeEvent(QResizeEvent *event);

private:
        class picture : public QLabel {
        public:
                picture(PreviewArea *preview);
                virtual ~picture();

        protected:
                virtual void mousePressEvent(QMouseEvent *event);

        private:
                PreviewArea *m_preview;
        };

        LoadImage m_loadImage;
        bool m_fit;

        QSize m_nextResize;
        bool  m_mustResize;
        bool  m_resizing;

        picture *m_picture;
};

#endif // PREVIEWAREA_HPP
