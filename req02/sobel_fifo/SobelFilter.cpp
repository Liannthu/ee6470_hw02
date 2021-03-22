#include <cmath>

#include "SobelFilter.h"

SobelFilter::SobelFilter(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

// sobel mask
int mask[MASK_X][MASK_Y] = 
{
  1,2,1,
  2,4,2,
  1,2,1
};

void SobelFilter::do_filter() {
  { wait(); }
  while (true) {
    double red = 0.0, green = 0.0, blue = 0.0;
    int  _red = 0.0, _green = 0.0, _blue = 0.0;
    int width;
    width = int(i_width.read());
    wait();
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < width; ++j){
        _red = int(i_r.read());
        _green = int(i_g.read());
        _blue = int(i_b.read());
        BUFFER[i][j][0] = _red;
        BUFFER[i][j][1] = _green;
        BUFFER[i][j][2] = _blue;
        wait();
      }
    }
    for (int i = 0; i < width; i++){
      for (int u = 0; u < 3; ++u) {
        for (int v = 0; v < 3; ++v) {
          red += BUFFER[u][i+v][0] * mask[v][u];
          green += BUFFER[u][i+v][1] * mask[v][u];
          blue += BUFFER[u][i+v][2] * mask[v][u];
        }
      }
    o_r.write(red);
    o_g.write(green);
    o_b.write(blue);
    red = 0; green = 0; blue = 0;
    }
    
  }
}
