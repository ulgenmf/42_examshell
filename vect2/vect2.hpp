#ifndef VECT2_HPP
#define VECT2_HPP

#include <iostream>

class vect2 {
  int x;
  int y;

public:
  vect2() : x(0), y(0) {}
  vect2(int xx, int yy) : x(xx), y(yy) {}
  vect2(vect2 const &o) {
    x = o.x;
    y = o.y;
  }
  int operator[](int i) const {
    if (i == 0)
      return x;
    return y;
  }
  int &operator[](int i) {
    if (i == 0)
      return x;
    return y;
  }

  
};

#endif
