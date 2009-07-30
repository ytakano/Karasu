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
        QImage qimg;


        if (src.format() == QImage::Format_RGB32 ||
            src.format() == QImage::Format_ARGB32) {
                bits = src.bits();
        } else {
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

        for (int i = 0; i <= label; i++) {
                if (labelArea[i] > 0) {
                        int idx;
                        if (labelArea[i] > tau) {
                                idx = labelColor[i].toIndex();
                                ret.alpha[idx] += labelArea[i];
                        } else {
                                idx = labelColor[i].toIndex();
                                ret.beta[idx] += labelArea[i];
                        }
                }
        }

        double size = w * h;
        for (int i = 0; i < NUM_COLOR; i++) {
                ret.alpha[i] = (double)ret.alpha[i] / size;
                ret.beta[i]  = (double)ret.beta[i]  / size;
        }
}


// thanks:
// http://d.hatena.ne.jp/audioswitch/20090221/1235184623
//
// width:画像の幅
// height:画像の高さ
// histogram_dimension:ヒストグラムの要素数(だいたい9らしい)
// cell_column:セルの列数（横方向に画像を何分割するか）
// cell_row:セルの行数（縦方向の画像を何分割するか）
// block_column:１ブロックの列数（だいたい3らしい）
// block_row:１ブロックの行数（だいたい3らしい）
void
histogramOrientedGradients(const QImage &src, int histogram_dimension,
                           int cell_column, int cell_row, int block_column,
                           int block_row, hog_ret &ret)
{
        int x;
        int y;
        int bin;
        int width  = src.width();
        int height = src.height();
        int cell_width = width / cell_column;
        int cell_height = height / cell_row;
        int block_dimension = histogram_dimension * block_column * block_row;
        int feature_vector_dimension = (histogram_dimension * block_column *
                                        block_row * (cell_row - block_row) *
                                        (cell_column - block_column));
        double fu;
        double fv;
        double magnitude;
        double direction;
        double norm;
        double epsilon = 1.0;
        double *histogram;
        double *cell_feature_vector;
        double *block_feature_vector;
        boost::shared_array<double> image(new double[width * height]);
        boost::shared_array<double> feature_vector(new double[feature_vector_dimension]);
        const uchar *bits;
        QImage qimg;


        if (src.format() == QImage::Format_RGB32 ||
            src.format() == QImage::Format_ARGB32) {
                bits = src.bits();
        } else {
                qimg = src.convertToFormat(QImage::Format_RGB32);

                bits = qimg.bits();
        }


        for (int i = 0; i < width * height; i++) {
                const QRgb *pixel = (const QRgb*)&bits[i * 4];
                image[i] = 0.299 * (double)qRed(*pixel) +
                        0.587 * (double)qGreen(*pixel) +
                        0.114 * (double)qBlue(*pixel);
        }

    
        // ヒストグラムを計算する．
        histogram = new double [histogram_dimension];
        cell_feature_vector = new double [histogram_dimension * cell_row * cell_column];

        for (int i = 0; i < cell_row; i++) {
                for (int j = 0; j < cell_column; j++) {
                        // セルごとにヒストグラムを計算する.
                        memset(histogram, 0, histogram_dimension * sizeof(double));
                        for (int v = 0; v < cell_height; v++) {
                                y = i * cell_height + v;
                                for (int u = 0; u < cell_width; u++) {
                                        x = j * cell_width + u;
                                        // 勾配強度・勾配方向を求める．
                                        if ((x > 0) && (x < width - 1) && (y > 0) && (y < height - 1)) {
                                                fu = image[y * width + (x + 1)] - image[y * width + (x - 1)];
                                                fv = image[(y + 1) * width + x] - image[(y - 1) * width + x];
                                                magnitude = sqrt(fu * fu + fv * fv);
                                                direction = atan(fv / fu) + M_PI / 2.0;
                                                // ヒストグラムに投票する．
                                                bin = (int)floor( (direction * (180.0 / M_PI)) * ((double)(histogram_dimension - 1) / 180.0) );
                                                histogram[bin] += magnitude;
                                        }
                                }
                        }
                        for (int d = 0; d < histogram_dimension; d++) {
                                cell_feature_vector[(d * cell_row + i) * cell_column + j] = histogram[d];
                        }
                }
        }

        delete [] histogram;
        
        // ブロックごとに正規化する．
        block_feature_vector = new double [block_dimension];
        for (int i = 0; i < cell_row - block_row; i++) {
                for (int j = 0; j < cell_column - block_column; j++) {
                        for (int k = 0; k < block_row; k++) {
                                for (int l = 0; l < block_column; l++) {
                                        for (int d = 0; d < histogram_dimension; d++) {
                                                block_feature_vector[(d * block_row + k) * block_column + l] = cell_feature_vector[(d * cell_row + (i + k)) * cell_column + (j + l)];
                                        }
                                }
                        }
                        norm = 0.0;
                        for (int d = 0; d < block_dimension; d++) {
                                norm += block_feature_vector[d] * block_feature_vector[d];
                        }
                        for (int d = 0; d < block_dimension; d++) {
                                feature_vector[(d * (cell_row - block_row) + i) * (cell_column - block_column) + j] = block_feature_vector[d] / sqrt(norm + epsilon * epsilon);
                        }
                }
        }
    
        delete [] cell_feature_vector;
        delete [] block_feature_vector;

        ret.dim = feature_vector_dimension;
        ret.feature = feature_vector;
}
