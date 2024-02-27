#include "widget.h"
#include <QDebug>
#include <cmath>

Widget::Widget(QWidget *parent)
    : QWidget(parent), t(this)
{
  auto seed=time(0);
  seed=1708983260;
  std::cout << "seed: " << seed << std::endl << std::flush;
  srand(seed);
  gen(2000);
  gen_rects(200);
  t.setInterval(16);
  connect(&t, SIGNAL(timeout()), this, SLOT(tick()));
  t.start();
}

Widget::~Widget()
{
}

void Widget::gen(int cnt)
{
    for(auto i=0; i!=cnt; i++) {
        int x=rand()%width();
        int y=rand()%height();
        int a=rand()%360;
        int vx=50*std::cos(a/360.0*2*M_PI);
        int vy=50*std::sin(a/360.0*2*M_PI);
        points.push_back(particle{QPointF(x, y), vx, vy});
    }
}

void Widget::gen_rects(int cnt)
{
    for(auto i=0; i!=cnt; i++) {
        int x=rand()%width();
        int y=rand()%height();
        int dx=rand()%30+ 10;
        int dy=rand()%30+ 10;
        rects.push_back(QRect(x, y, dx, dy));
    }
}
void Widget::paintEvent(QPaintEvent *e) {
    QPainter p(this);

    ph tr(1);
    for(auto pt: points) {
      ph::point pt2; pt2[0][0]=pt.pz.x(); pt2[1][0]=pt.pz.y();
      tr.insert(tr.encode(pt2));
    }

    ph2 tr2(1);
    for(auto rect: rects) {
      auto low = rect.topLeft(), top = rect.bottomRight();
      ph2::point rect2; rect2[0][0] = low.x(); rect2[1][0] = low.y(); rect2[2][0] = top.x(); rect2[3][0] = top.y();
      tr2.insert(tr2.encode(rect2));
    }

    p.setPen(QPen(QColor(0, 255, 0, 255), 2));
    for(auto it=tr2.begin(); !it.end(); it++) { auto pt=tr2.decode(*it); p.drawRect(pt[0][0], pt[1][0], pt[2][0]-pt[0][0], pt[3][0]-pt[1][0]); }
 
    p.setPen(QPen(QColor(0, 255, 0, 255), 3));
    for(auto it=tr.begin(); !it.end(); it++) { auto pt=tr.decode(*it); p.drawPoint(pt[0][0], pt[1][0]); }

    //knn query
    p.setPen(QPen(QColor(255, 0, 0, 255), 3));
    auto pivot = ph::point{}; pivot[0][0] = points[0].pz.x(); pivot[1][0] = points[0].pz.y();
    int i=0; for(auto it=tr.knnIterator(tr.encode(pivot)); !it.end() && i!=100; it++, i++) { auto pt=tr.decode(*it); p.drawPoint(pt[0][0], pt[1][0]); }
    p.setPen(QPen(QColor(0, 0, 255, 255), 5));

    p.drawPoint(pivot[0][0], pivot[1][0]);

    //points in rect
    p.setPen(QPen(QColor(255, 0, 0, 255), 3));
    pivot = ph::point{}; pivot[0][0] = points[3].pz.x(); pivot[1][0] = points[3].pz.y();
    auto low = ph::point{}; low[0][0] = pivot[0][0] - 50; low[1][0] = pivot[1][0] - 50;
    auto top = ph::point{}; top[0][0] = pivot[0][0] + 50; top[1][0] = pivot[1][0] + 50;
    i=0; for(auto it=tr.rectIterator(tr.encode(low), tr.encode(top)); !it.end(); it++, i++) { auto pt=tr.decode(*it); p.drawPoint(pt[0][0], pt[1][0]); }
    p.setPen(QPen(QColor(0, 0, 255, 255), 2));
    p.drawRect(low[0][0], low[1][0], 100, 100);
    p.setPen(QPen(QColor(0, 0, 255, 255), 5));
    p.drawPoint(pivot[0][0], pivot[1][0]);

    //rects intersect
    p.setPen(QPen(QColor(0, 0, 255, 255), 2));
    pivot = ph::point{}; pivot[0][0] = points[4].pz.x(); pivot[1][0] = points[4].pz.y();
    auto rect2 = ph2::point{}; 
    rect2[0][0] = pivot[0][0] - 50; rect2[1][0] = pivot[1][0] - 50; rect2[2][0] = pivot[0][0] + 50; rect2[3][0] = pivot[1][0] + 50;
    p.drawRect(rect2[0][0], rect2[1][0], rect2[2][0] - rect2[0][0], rect2[3][0] - rect2[1][0]); 
    p.setPen(QPen(QColor(255, 0, 0, 255), 2));
    for(auto it=tr2.intersectIterator(tr2.encode(rect2)); !it.end(); it++) {
      auto rect = tr2.decode(*it);
      p.drawRect(rect[0][0], rect[1][0], rect[2][0] - rect[0][0], rect[3][0] - rect[1][0]); 
    }

    //rects inside 
    p.setPen(QPen(QColor(0, 0, 255, 255), 2));
    pivot = ph::point{}; pivot[0][0] = points[5].pz.x(); pivot[1][0] = points[5].pz.y();
    rect2 = ph2::point{}; 
    rect2[0][0] = pivot[0][0] - 50; rect2[1][0] = pivot[1][0] - 50; rect2[2][0] = pivot[0][0] + 50; rect2[3][0] = pivot[1][0] + 50;
    p.drawRect(rect2[0][0], rect2[1][0], rect2[2][0] - rect2[0][0], rect2[3][0] - rect2[1][0]); 
    p.setPen(QPen(QColor(255, 0, 0, 255), 2));
    for(auto it=tr2.includeIterator(tr2.encode(rect2)); !it.end(); it++) {
      auto rect = tr2.decode(*it);
      p.drawRect(rect[0][0], rect[1][0], rect[2][0] - rect[0][0], rect[3][0] - rect[1][0]); 
    }

    //ray intersection
    ray[1] = std::sin(ticks*0.01); ray[0] = std::cos(ticks*0.01);
    double pt[2]; pt[0]=width()/2; pt[1]=height()/2;
    p.drawLine(pt[0], pt[1], pt[0]+ray[0]*1000, pt[1]+ray[1]*1000);
    for(auto it=tr2.rayIterator(pt, ray); !it.end(); it++) { 
      auto rect = tr2.decode(*it);
      p.drawRect(rect[0][0], rect[1][0], rect[2][0] - rect[0][0], rect[3][0] - rect[1][0]); 
    }
}

void Widget::tick() {
  ticks++;
  for(auto &part: points) {
    part.pz.rx() += part.vx*0.016;
    part.pz.ry() += part.vy*0.016;
    if(part.pz.x() < 0) { part.vx *= -1; }
    if(part.pz.y() < 0) { part.vy *= -1; }
    if(part.pz.x() >= width()) { part.vx *= -1; }
    if(part.pz.y() >= height()) { part.vy *= -1; }
  }

  update();
}
