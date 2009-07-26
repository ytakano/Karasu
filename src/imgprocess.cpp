#include "imgprocess.hpp"

#include <math.h>
#include <string.h>

#include <boost/shared_array.hpp>

#include <algorithm>
#include <iostream>

class histgram {
public:
        int color;
        int num;

        bool operator< (const histgram &rhs) const {
                return num > rhs.num;
        }
};

QImage
gaussianFilter(const QImage &src, double sigma)
{
        int x, y;
        int w = src.width();
        int h = src.height();
        int ww = (int)(ceil(3.0 * sigma) * 2 + 1);
        int wr = (ww - 1) / 2;
        const uchar *bits;
        double  div;
        QImage  dst(w, h, QImage::Format_RGB32);
        boost::shared_array<double> tmp(new double[w * h * 3]);
        boost::shared_array<double> filter(new double[ww]);


        if (src.format() == QImage::Format_RGB32 ||
            src.format() == QImage::Format_ARGB32) {
                bits = src.bits();
        } else {
                QImage qimg;
                qimg = src.convertToFormat(QImage::Format_RGB32);

                bits = qimg.bits();
        }


        sigma = 2 * sigma * sigma;
        div   = sqrt(sigma * M_PI);

        for (x = 0; x < ww; x++) {
                int p = (x - wr) * (x - wr);
                filter[x] = exp(-p / sigma) / div;
        }


        for (x = 0; x < w; x++) {
                for (y = 0; y < h; y++) {
                        double r, g, b;
                        r = g = b = 0.0;
                        for (int i = 0; i < ww; i++) {
                                int p = y + i - wr;
                                if (p < 0) {
                                        p = 0;
                                } else if (p >= h) {
                                        p = h - 1;
                                }

                                const QRgb *pixel = (const QRgb*)&bits[x * 4 + p * w * 4];
                                r += filter[i] * qRed(*pixel);
                                g += filter[i] * qGreen(*pixel);
                                b += filter[i] * qBlue(*pixel);
                        }
                        double *p_tmp = &tmp[x * 3 + y * w * 3];
                        p_tmp[0] = r;
                        p_tmp[1] = g;
                        p_tmp[2] = b;
                }
        }

        for(x = 0; x < w; x++){
                for(y = 0; y < h; y++){
                        double r, g, b;
                        r = g = b = 0.0;

                        for(int i = 0; i < ww; i++){
                                int p = x + i - wr;

                                if (p < 0) {
                                        p = 0;
                                } else if (p >= w) {
                                        p = w - 1;
                                }

                                int idx = p * 3 + y * w * 3;
                                r += filter[i] * tmp[idx];
                                g += filter[i] * tmp[idx + 1];
                                b += filter[i] * tmp[idx + 2];
                        }

                        dst.setPixel(x, y,
                                     qRgb(r > 255 ? 255 : r,
                                          g > 255 ? 255 : g,
                                          b > 255 ? 255 : b));
                }
        }

        return dst;
}

void
ccv(const QImage &src, ccv_ret &ret)
{
        int x, y;
        int w   = src.width();
        int h   = src.height();
        int label;
        int tau = 4;
        const uchar *bits;
        bool flagLeft;
        bool flagRight;
        struct histgram hist[NUM_COLOR];
        std::vector<rgb> labelColor;
        std::vector<int> labelArea;
        boost::shared_array<int>   bufferNew(new int[w]);
        boost::shared_array<int>   bufferOld(new int[w]);
        boost::shared_array<uchar> image(new uchar[3 * w * h]);
        QImage qimg;
        rgb c;

        memset(ret.alpha, 0, sizeof(ret.alpha));
        memset(ret.beta,  0, sizeof(ret.beta));

        if (src.format() == QImage::Format_RGB32 ||
            src.format() == QImage::Format_ARGB32) {
                bits = src.bits();
        } else {
                qimg = src.convertToFormat(QImage::Format_RGB32);

                bits = qimg.bits();
        }

        for (int i = 0; i < w * h; i++) {
                const QRgb *pixel = (const QRgb*)&bits[i * 4];
                uchar r, g, b;

                r = qRed(*pixel);
                g = qGreen(*pixel);
                b = qBlue(*pixel);

                r = r < 85 ? 0 : r < 170 ? 1 : 2;
                g = g < 85 ? 0 : g < 170 ? 1 : 2;
                b = b < 85 ? 0 : b < 170 ? 1 : 2;

                uchar *p = &image[i * 3];
                p[0] = r;
                p[1] = g;
                p[2] = b;
        }


        label = 0;
        c.r = image[0];
        c.g = image[1];
        c.b = image[2];
        labelColor.push_back(c);
        labelArea.push_back(1);
        bufferNew[0] = label;

        for (x = 1; x < w; x++) {
                if (image[x * 3    ] == image[(x - 1) * 3    ] &&
                    image[x * 3 + 1] == image[(x - 1) * 3 + 1] &&
                    image[x * 3 + 2] == image[(x - 1) * 3 + 2]) {
                        bufferNew[x] = bufferNew[x - 1];
                        labelArea[bufferNew[x]]++;
                } else {
                        label++;

                        c.r = image[x * 3];
                        c.g = image[x * 3 + 1];
                        c.b = image[x * 3 + 2];

                        labelColor.push_back(c);
                        labelArea.push_back(1);
                        bufferNew[x] = label;
                }
        }

        for (y = 1; y < h; y++) {
                for (x = 0; x < w; x++) {
                        bufferOld[x] = bufferNew[x];
                }

                int idx1, idx2, idx3;

                for (x = 0; x < w; x++) {
                        idx1 = y * w * 3 + x * 3;
                        idx2 = (y - 1) * w * 3 + x * 3;
                        if (image[idx1    ] == image[idx2    ] &&
                            image[idx1 + 1] == image[idx2 + 1] &&
                            image[idx1 + 2] == image[idx2 + 2]) {
                                bufferNew[x] = bufferOld[x];
                                labelArea[bufferNew[x]]++;
                        } else {
                                flagLeft  = false;
                                flagRight = false;

                                if (x > 0) {
                                        idx1 = y * w * 3 + x * 3;
                                        idx2 = (y - 1) * w * 3 + (x - 1) * 3;
                                        idx3 = y + w * 3 + (x - 1) * 3;
                                        if (image[idx1    ] == image[idx2    ] &&
                                            image[idx1 + 1] == image[idx2 + 1] &&
                                            image[idx1 + 2] == image[idx2 + 2]) {
                                                bufferNew[x] = bufferOld[x - 1];
                                                labelArea[bufferNew[x]]++;
                                                flagLeft = true;
                                        } else if (image[idx1    ] == image[idx3    ] &&
                                                   image[idx1 + 1] == image[idx3 + 1] &&
                                                   image[idx1 + 2] == image[idx3 + 2]) {
                                                bufferNew[x] = bufferNew[x - 1];
                                                labelArea[bufferNew[x]]++;
                                                flagLeft = true;
                                        }
                                }

                                if (x < w - 1) {
                                        idx1 = y * w * 3 + x * 3;
                                        idx2 = (y - 1) * w * 3 + (x + 1) * 3;
                                        if (image[idx1] == image[idx2] &&
                                            image[idx1 + 1] == image[idx2 + 1] &&
                                            image[idx1 + 2] == image[idx2 + 2]) {
                                                if (! flagLeft) {
                                                        bufferNew[x] = bufferOld[x + 1];
                                                        labelArea[bufferNew[x]]++;
                                                } else if (bufferNew[x] != bufferOld[x + 1]) {
                                                        labelArea[bufferNew[x]] += labelArea[bufferOld[x + 1]];
                                                        labelArea[bufferOld[x + 1]] = 0;
                                                }
                                                flagRight = true;
                                        }
                                }

                                if (! flagLeft && ! flagRight) {
                                        label++;

                                        idx1 = y * w * 3 + x * 3;
                         
                                        c.r = image[idx1];
                                        c.g = image[idx1 + 1];
                                        c.b = image[idx1 + 2];

                                        labelColor.push_back(c);
                                        labelArea.push_back(1);
                                        bufferNew[x] = label;
                                }
                        }
                }
        }

        for (int i = 0; i < NUM_COLOR; i++) {
                hist[i].color = i;
                hist[i].num = 0;
        }

        for (int i = 0; i <= label; i++) {
                if (labelArea[i] > 0) {
                        int idx;
                        if (labelArea[i] > tau) {
                                idx = labelColor[i].toIndex();
                                ret.alpha[idx] += labelArea[i];
                                hist[idx].num += labelArea[i];
                        } else {
                                idx = labelColor[i].toIndex();
                                ret.beta[idx] += labelArea[i];
                                hist[idx].num += labelArea[i];
                        }
                }
        }

        std::sort(hist, &hist[NUM_COLOR]);

        for (int i = 0; i < NUM_COLOR; i++) {
                ret.rank[i] = hist[i].color;
        }
}
