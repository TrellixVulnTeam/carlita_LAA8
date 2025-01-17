#include "assetgen.h"
#include <iostream>

struct ColorGen {
    RandGen *rand_gen;
    float rgb_start[3];
    float rgb_len[3];
    int rgb_choice[3];
    float p_rect;

    void roll() {
        for (int i = 0; i < 3; i++) {
            rgb_len[i] = rand_gen->rand01();
        }

        for (int i = 0; i < 3; i++) {
            rgb_start[i] = rand_gen->rand01() * (1 - rgb_len[i]);
        }

        p_rect = rand_gen->rand01();
    }

    QColor rand_color() {
        for (int i = 0; i < 3; i++) {
            rgb_choice[i] = int(255 * (rand_gen->rand01() * rgb_len[i] + rgb_start[i]));
        }

        return QColor(rgb_choice[0], rgb_choice[1], rgb_choice[2]);
    }
};

AssetGen::AssetGen(RandGen *rg) {
    rand_gen = rg;
}

QRectF AssetGen::choose_sub_rect(QRectF rect, float min_dim, float max_dim) {
    int w = rect.width();
    int h = rect.height();

    int smaller = (w > h) ? h : w;

    float del_dim = max_dim - min_dim;

    float rdx = (rand_gen->rand01() * del_dim + min_dim) * smaller;
    float rdy = (rand_gen->rand01() * del_dim + min_dim) * smaller;
    float rx_off = rand_gen->rand01() * (w - rdx);
    float ry_off = rand_gen->rand01() * (h - rdy);

    QRectF dst3 = QRectF(rx_off + rect.x(), ry_off + rect.y(), rdx, rdy);

    return dst3;
}

std::vector<QRectF> AssetGen::split_rect(QRectF rect, int num_splits, bool is_horizontal) {
    std::vector<QRectF> split_rects;

    float x = rect.x();
    float y = rect.y();
    float w = rect.width();
    float h = rect.height();

    float dw = w / num_splits;
    float dh = h / num_splits;

    for (int i = 0; i < num_splits; i++) {
        if (is_horizontal) {
            split_rects.push_back(QRectF(x + i * dw, y, dw, h));
        } else {
            split_rects.push_back(QRectF(x, y + i * dh, w, dh));
        }
    }

    return split_rects;
}

void AssetGen::paint_shape(QPainter &p, QRectF main_rect, ColorGen *cgen, int color_theme) {
    int k = rand_gen->randn(10);
    int num_splits = (k * k) / 50 + 1;
    std::vector<QRectF> split_rects = split_rect(main_rect, num_splits, rand_gen->randbool());

    bool use_rect = rand_gen->randbool();
    bool regen_colors = rand_gen->randbool();

    QColor c1 = get_rand_color(color_theme); //cgen->rand_color();
    QColor c2 = get_rand_color(color_theme); //cgen->rand_color();

    for (QRectF rect : split_rects) {
        if (regen_colors) {
            c1 = get_rand_color(color_theme); //cgen->rand_color();
            c2 = get_rand_color(color_theme); //cgen->rand_color();
        }

        if (use_rect) {
            p.fillRect(rect, c1);
        } else {
            QBrush brush(c1);
            QPen pen(c2);
            p.setBrush(brush);
            p.setPen(pen);
            p.drawEllipse(rect);
        }
    }
}

QColor AssetGen::get_rand_color(int color_themes) {
    QColor color;

    std::vector<int> colors;

    while (color_themes > 0)
    {
        int digit = color_themes % 10;
        color_themes /= 10;
        colors.push_back(digit);
    }
    int color_theme = colors.at(rand_gen->randn(colors.size()));

    if (color_theme==0) {
        color = QColor(rand_gen->randint(0,100), rand_gen->randint(0,100), rand_gen->randint(0,100));
    } else if (color_theme==1) {
        color = QColor(rand_gen->randint(0,100), rand_gen->randint(0,100), rand_gen->randint(175, 255));
    } else if (color_theme==2) {
        color = QColor(rand_gen->randint(175, 255), rand_gen->randint(0,100), rand_gen->randint(0,100));
    } else if (color_theme==3) {
        color = QColor(rand_gen->randint(0,100), rand_gen->randint(175,255), rand_gen->randint(0,100));
    } else if (color_theme==4) {
        color = QColor(rand_gen->randint(0,100), rand_gen->randint(175,255), rand_gen->randint(175,255));
    } else if (color_theme==5) {
        color = QColor(rand_gen->randint(175, 255), rand_gen->randint(0,100), rand_gen->randint(175, 255));
    } else if (color_theme==6) {
        color = QColor(rand_gen->randint(175, 255), rand_gen->randint(175,255), rand_gen->randint(175,255));
    } else if (color_theme==7) {
        color = QColor(rand_gen->randint(175, 255), rand_gen->randint(175,255), rand_gen->randint(0,100));
    };
    return color;
}

void AssetGen::paint_rect_resource(QPainter &p, QRectF rect, int num_recurse, int blotch_scale, int color_theme, int background_noise_level) {
    // Paints background
    ColorGen cgen;
    cgen.rand_gen = rand_gen;
    cgen.roll();

    QColor bgcolor = get_rand_color(color_theme); //cgen.rand_color();

    p.fillRect(rect, bgcolor);

    float scale = .3 + .7 * rand_gen->rand01();

    float max_rand_dim = .5 * scale;
    float min_rand_dim = .05 * scale;
    int num_blotches = background_noise_level; //rand_gen->randint(blotch_scale, 2 * blotch_scale); // 1000 is alot
    float p_recurse = rand_gen->rand01() * .75;

    for (int j = 0; j < num_blotches; j++) {
        QRectF dst3 = choose_sub_rect(rect, min_rand_dim, max_rand_dim);

        if ((num_recurse > 0) && (rand_gen->rand01() < p_recurse)) {
            paint_rect_resource(p, dst3, num_recurse - 1, 10, color_theme, background_noise_level);
        } else {
            paint_shape(p, dst3, &cgen, color_theme);
        }
    }

    bgcolor.setAlpha(100); // transparency, less allows more shapes coming through
    p.fillRect(rect, bgcolor);
}

QRectF AssetGen::create_bar(QRectF rect, bool is_horizontal) {
    float k1 = (.45 + rand_gen->rand01() * .4);
    float k2 = (.45 + rand_gen->rand01() * .4);
    float w = rect.width() * k1 * k1;
    float h = rect.height() * k2 * k2;
    float pct = rand_gen->rand01();
    QRectF crect;

    if (is_horizontal == 0) {
        crect = QRectF(0, (rect.height() - h) * pct, rect.width(), h);
    } else {
        crect = QRectF((rect.height() - w) * pct, 0, w, rect.height());
    }

    return crect;
}

void AssetGen::paint_shape_resource(QPainter &p, QRectF rect, int color_theme) {
    ColorGen cgen;
    
    cgen.rand_gen = rand_gen;
    cgen.roll();

    bool horizontal_first = rand_gen->randbool();
    int nbar1 = rand_gen->randn(3) / 2 + 1;
    int nbar2 = rand_gen->randn(3) / 2 + 1;

    p.save();

    p.setCompositionMode(QPainter::CompositionMode_Source);

    //p.rotate(rand_gen->randrange(0, 6.28));

    p.fillRect(rect, QColor(0, 0, 0, 0));

    for (int i = 0; i < nbar1; i++) {
        QRectF c1 = create_bar(rect, horizontal_first);
        paint_shape(p, c1, &cgen, color_theme);
    }

    for (int i = 0; i < nbar2; i++) {
        QRectF c2 = create_bar(rect, !horizontal_first);
        paint_shape(p, c2, &cgen, color_theme);
    }

    int num_blotches = rand_gen->randint(1, 5);

    for (int j = 0; j < num_blotches; j++) {
        QRectF dst = choose_sub_rect(rect, 0.1f, 0.6f);

        paint_shape(p, dst, &cgen, color_theme);
    }

    p.restore();
}



void AssetGen::generate_resource(std::shared_ptr<QImage> img, int color_theme, int background_noise_level, int num_recurse, int blotch_scale, bool is_rect) {
    QPainter p(img.get());
    QRectF rect = QRectF(0, 0, img->width(), img->height());

    if (is_rect) {
        paint_rect_resource(p, rect, num_recurse, blotch_scale, color_theme, background_noise_level);
    } else {
        paint_shape_resource(p, rect, color_theme);
    }
}

void AssetGen::make_circle(std::shared_ptr<QImage> img, int num_recurse, int blotch_scale, bool is_rect) {
    QPainter p(img.get());
    QRectF rect = QRectF(0, 0, img->width(), img->height());

    p.save();
    QColor color = QColor(30, 10, 100); 
	p.setBrush(color);
    p.drawEllipse(rect);
    p.restore();
}