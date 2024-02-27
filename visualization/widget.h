#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "ph_tree.h"
#include"encode.h"
#include <vector>
#include <QPainter>
#include <cstdint>
#include <QMouseEvent>
#include <QTimer>

using u64=std::uint64_t;
using ph = PHTree<2, 32>;
using ph2 = PHTree<4, 32>;
using bstr = ph::bstr;

struct particle {
  QPointF pz;
  double vx, vy;
};

class Widget : public QWidget
{
    Q_OBJECT
    std::vector<particle> points;
    std::vector<QRect> rects;
    double ray[2];
    int ticks=0;
    QTimer t;
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void gen(int cnt);
    void gen_rects(int cnt);
    void paintEvent(QPaintEvent *e);
public slots:
    void tick();
};

#endif // WIDGET_H
