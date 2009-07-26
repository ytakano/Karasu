#ifndef IMGPROCESS_HPP
#define IMGPROCESS_HPP

#define NUM_COLOR (3 * 3 * 3)

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
        int alpha[NUM_COLOR];
        int beta[NUM_COLOR];
        int rank[NUM_COLOR];
};


QImage gaussianFilter(const QImage &image, double sigma);
void   ccv(const QImage &src, ccv_ret &ret);

#endif // IMGPROCESS_HPP
