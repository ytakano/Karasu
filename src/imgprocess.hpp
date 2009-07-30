#ifndef IMGPROCESS_HPP
#define IMGPROCESS_HPP

#define NUM_COLOR (3 * 3 * 3)

#include <boost/shared_array.hpp>

#include <QImage>

class rgb {
public:
        uchar r, g, b;

        int
        toIndex() {
                return NUM_COLOR - 1 - (3 * 3 * r + 3 * g + b);
        }

        void
        fromIndex(int i) {
                b = i % 3;
                i -= b;

                g = i % (3 * 3);
                i -= g * 3;

                r = i / (3 * 3);
        }
};

struct ccv_ret {
        double alpha[NUM_COLOR];
        double beta[NUM_COLOR];
};

struct hog_ret {
        int dim;
        boost::shared_array<double> feature;
};


QImage gaussianFilter(const QImage &image, double sigma);
void   ccv(const QImage &src, ccv_ret &ret);
void   histogramOrientedGradients(const QImage &src, int histogram_dimension,
                                  int cell_column, int cell_row,
                                  int block_column, int block_row,
                                  hog_ret &ret);

#endif // IMGPROCESS_HPP
