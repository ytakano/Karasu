#include "thumbnail.hpp"

#include "loadthumbnails.hpp"

#include <boost/foreach.hpp>

#include <iostream>

#include <QtGui>

boost::unordered_set<Thumbnail*> Thumbnail::focusing;

int Thumbnail::imgWidth  = 120;
int Thumbnail::imgHeight = 120;

Thumbnail::Thumbnail()
{
        setFrameStyle(QFrame::Panel | QFrame::Raised);

        m_layout = new QVBoxLayout;

        m_layout->setSpacing(2);
        m_layout->setContentsMargins(4, 4, 4, 4);

        setLayout(m_layout);


        m_picture = new picture(this);

        m_layout->addWidget(m_picture);

        m_picture->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        m_picture->setMinimumWidth(imgWidth);
        m_picture->setMinimumHeight(imgHeight);


        QFont font;
        font.setPointSize(9);

        m_nameLabel = new QLabel;
        m_nameLabel->setFixedWidth(imgWidth);
        m_nameLabel->setAlignment(Qt::AlignHCenter);
        m_nameLabel->setFont(font);
        m_layout->addWidget(m_nameLabel);

        m_sizeLabel = new QLabel;
        m_sizeLabel->setFixedWidth(imgWidth);
        m_sizeLabel->setAlignment(Qt::AlignHCenter);
        m_sizeLabel->setFont(font);
        m_layout->addWidget(m_sizeLabel);
}

Thumbnail::~Thumbnail()
{

}

void
Thumbnail::setFocus(bool focus)
{
        if (focus) {
                if (focusing.find(this) != focusing.end())
                        return;
         
                setFrameStyle(QFrame::Panel | QFrame::Sunken);
                setBackgroundRole(QPalette::Midlight);
                setAutoFillBackground(true);

                focusing.insert(this);
        } else {
                if (focusing.find(this) == focusing.end())
                        return;

                setFrameStyle(QFrame::Panel | QFrame::Raised);
                setBackgroundRole(QPalette::NoRole);
                setAutoFillBackground(false);
                
                focusing.erase(this);
        }
}

void
Thumbnail::mousePressEvent(QMouseEvent *event)
{
        if (event->modifiers() & Qt::ShiftModifier) {
                if (focusing.find(this) == focusing.end()) {
                        setFocus(true);
                        emit focused(this);
                } else {
                        setFocus(false);
                }
        } else {
                BOOST_FOREACH(Thumbnail *thumb, focusing) {
                        thumb->setFrameStyle(QFrame::Panel | QFrame::Raised);
                        thumb->setBackgroundRole(QPalette::NoRole);
                        thumb->setAutoFillBackground(false);
                }

                focusing.clear();

                setFocus(true);
                emit focused(this);
        }
}

Thumbnail::picture::picture(Thumbnail *thumb) : m_thumb(thumb)
{

}

Thumbnail::picture::~picture()
{

}

void
Thumbnail::picture::mousePressEvent(QMouseEvent *event)
{
        m_thumb->mousePressEvent(event);
}

void
Thumbnail::paintEvent(QPaintEvent *event)
{
        QFrame::paintEvent(event);
}

void
Thumbnail::setImage(image &img)
{
        QString name;
        QString width, height;

        m_picture->setPixmap(QPixmap::fromImage(img.img));

        name = img.file;
        if (name.size() > 12) {
                name.resize(12);
                name += tr("...");
        }
        m_nameLabel->setText(name);

        width.setNum(img.width);
        height.setNum(img.height);
        m_sizeLabel->setText(width + tr("x") + height);

        m_path   = img.path;
        m_file   = img.file;
        m_date   = img.date;
        m_width  = img.width;
        m_height = img.height;
        m_size   = img.size;
        m_ccvr   = img.ccvr;
        m_hogr   = img.hogr;

        zOrder();
}

void
Thumbnail::clearPixmap()
{
        QPixmap pixmap;

        m_picture->setPixmap(pixmap);
}

#define DIM_BITS 6

void
Thumbnail::zOrder()
{
        int i, j;
        int dim;
        int znum;
        uint32_t alpha[NUM_COLOR];
        uint32_t beta[NUM_COLOR];
        boost::shared_array<uint32_t> feature(new uint32_t[m_hogr->dim]);

        dim  = NUM_COLOR * 2 + m_hogr->dim;
        znum = DIM_BITS * dim / 32 + 1;

        for (i = 0; i < NUM_COLOR; i++) {
                alpha[i] = (uint32_t)ceil(m_ccvr->alpha[i] * (1 << DIM_BITS));
                beta[i] = (uint32_t)ceil(m_ccvr->beta[i]   * (1 << DIM_BITS));
        }

        for (i = 0; i < m_hogr->dim; i++) {
                feature[i] = (uint32_t)ceil(m_hogr->feature[i] *
                                            (1 << DIM_BITS));
        }


        m_z    = boost::shared_array<uint32_t>(new uint32_t[znum]);
        m_znum = znum;

        for (i = 0; i < znum; i++) {
                m_z[i] = 0;
        }


        for (i = 0; i < DIM_BITS; i++) {
                int shift, idx;

                for (j = 0; j < NUM_COLOR; j++) {
                        shift = i * dim - i + j * 2 + m_hogr->dim;
                        idx   = (shift + i) / 32;
                        shift = shift % 32;

                        m_z[idx] |= (alpha[j] & (1 << i)) << shift;


                        shift = i * dim - i + j * 2 + 1 + m_hogr->dim;
                        idx   = (shift + i) / 32;
                        shift = shift % 32;

                        m_z[idx] |= (beta[j]  & (1 << i)) << shift;
                }
/*
                for (j = 0; j < m_hogr->dim; j++) {
                        shift = i * dim - i + j;
                        idx   = (shift + i) / 32;
                        shift = shift % 32;

                        m_z[idx] |= (feature[j]  & (1 << i)) << shift;
                }
*/
        }
}
