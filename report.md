
# General description or introduction of the problem and your solution

This homework is very similar to hw01. Req01 is same as previous req02. Req02 is similar to previous req02 with modification on pixels transfering. It requires us
to create a buffer to store pixels, since pixels can be reused to save unnecessary transfering. We observed that by implementing buffer we can save bandwidth by three times.

# Implementation details (data structure, flows and algorithms)

## req01
The code is as same as previous req02.
### main.cpp
```
tb.o_r(r);
tb.o_g(g);
tb.o_b(b);
tb.i_r(i_r);
tb.i_g(i_g);
tb.i_b(i_b);
sobel_filter.i_r(r);
sobel_filter.i_g(g);
sobel_filter.i_b(b);
sobel_filter.o_r(i_r);
sobel_filter.o_g(i_g);
sobel_filter.o_b(i_b);
```
In main.cpp file, most of the code remain unchanged. I only add three sc_fifo: i_r, i_g i_b as fifo between filter and testbench. 
### testbench.cpp
```
for (y = 0; y != height; ++y) {
  for (x = 0; x != width; ++x) {
    adjustX = (MASK_X % 2) ? 1 : 0; // 1
    adjustY = (MASK_Y % 2) ? 1 : 0; // 1
    xBound = MASK_X / 2;            // 1
    yBound = MASK_Y / 2;            // 1

    for (v = -yBound; v != yBound + adjustY; ++v) {   //-1, 0, 1
      for (u = -xBound; u != xBound + adjustX; ++u) { //-1, 0, 1
        if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {
          R = *(source_bitmap +
                bytes_per_pixel * (width * (y + v) + (x + u)) + 2);
          G = *(source_bitmap +
                bytes_per_pixel * (width * (y + v) + (x + u)) + 1);
          B = *(source_bitmap +
                bytes_per_pixel * (width * (y + v) + (x + u)) + 0);
        } else {
          R = 0;
          G = 0;
          B = 0;
        }
        o_r.write(R);
        o_g.write(G);
        o_b.write(B);
        pixel_transfer_c++;
      }
    }
    red = i_r.read();
    green = i_g.read();
    blue = i_b.read();

    *(target_bitmap + bytes_per_pixel * (width * y + x) + 2) = std::min(std::max(int(_factor * red + _bias), 0), 255);
    *(target_bitmap + bytes_per_pixel * (width * y + x) + 1) = std::min(std::max(int(_factor * green + _bias), 0), 255);
    *(target_bitmap + bytes_per_pixel * (width * y + x) + 0) = std::min(std::max(int(_factor * blue + _bias), 0), 255);
  }
}

```
In Testbench.cpp, I alter do_sobel function by removing “total” variable and replacing it with three variables: o_r, o_g, o_b. Moreover, I apply min and max function in target_bitmap to avoid values over 255 or under 0. I made those modification so that my program can generate colored images rather than gray images. Besides, values from filter process will be added bias and be divided by factor to meet the requirements of gaussian blur. By using fifo my program can do the filtering in another systemC process. Additionaly, I add pixel_transfer_c to count the number of pixels transfering.

### sobelfilter.cpp
```
while (true) {
  for (unsigned int i = 0; i < MASK_N; ++i) {
    val[i] = 0;
    wait();
  }
  double  red = 0.0, green = 0.0, blue = 0.0;
  int  _red = 0.0, _green = 0.0, _blue = 0.0;
  for (unsigned int v = 0; v < MASK_Y; ++v) {
    for (unsigned int u = 0; u < MASK_X; ++u) {
      _red = int(i_r.read());
      _green = int(i_g.read());
      _blue = int(i_b.read());
      wait();
      red += _red * mask[u][v];
      green += _green * mask[u][v];
      blue += _blue * mask[u][v];
      wait();
    }
  }
  o_r.write(red);
  o_g.write(green);
  o_b.write(blue);
}

```
In SobelFilter.cpp, I changed the mask and the calculation process so that it can apply the mask on red, green, and blue separately, instead of merging those colors to gray in the previous process. 

## req02

I only make the changes I made in req01.

### main.cpp

```
sc_fifo<int> width;
tb.o_width(width);
sobel_filter.i_width(width);
```
In main.cpp, I add width fifo to transfer width variable to another module.

### testbench.cpp

```
for (y = 0; y != height; ++y) {
  int y_0, y_1, y_2;
  y_0 = y - 1;
  y_1 = y;
  y_2 = y + 1;
  if (y_0 == -1) y_0 = 511;
  if (y_2 == 512) y_2 = 0;
  BUFFER[0][0][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (511)) + 2)); 
  BUFFER[0][0][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (511)) + 1));
  BUFFER[0][0][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (511)) + 0));
  BUFFER[0][0][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (511)) + 2)); 
  BUFFER[0][0][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (511)) + 1));
  BUFFER[0][0][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (511)) + 0));
  BUFFER[0][0][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (511)) + 2)); 
  BUFFER[0][0][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (511)) + 1));
  BUFFER[0][0][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (511)) + 0));
  for (x = 0; x != width; ++x) {
    BUFFER[0][x + 1][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (x)) + 2)); 
    BUFFER[0][x + 1][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (x)) + 1));
    BUFFER[0][x + 1][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (x)) + 0));
    BUFFER[1][x + 1][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (x)) + 2));
    BUFFER[1][x + 1][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (x)) + 1));
    BUFFER[1][x + 1][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (x)) + 0));
    BUFFER[2][x + 1][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (x)) + 2));
    BUFFER[2][x + 1][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (x)) + 1));
    BUFFER[2][x + 1][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (x)) + 0));
  }
  BUFFER[0][513][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (0)) + 2)); 
  BUFFER[0][513][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (0)) + 1));
  BUFFER[0][513][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_0) + (0)) + 0));
  BUFFER[0][513][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (0)) + 2)); 
  BUFFER[0][513][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (0)) + 1));
  BUFFER[0][513][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_1) + (0)) + 0));
  BUFFER[0][513][0] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (0)) + 2)); 
  BUFFER[0][513][1] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (0)) + 1));
  BUFFER[0][513][2] = int(*(source_bitmap + bytes_per_pixel * (width * (y_2) + (0)) + 0));
  o_width.write(width);
  for (int j = 0; j < 3; j++){
    for (int j_ = 0; j_ < width; j_++){
      o_r.write(BUFFER[j][j_][0]);
      o_g.write(BUFFER[j][j_][1]);
      o_b.write(BUFFER[j][j_][2]);
      pixel_transfer_c++;
    }
  }
  for (i = 0; i < width; i++){
      red = i_r.read();
      green = i_g.read();
      blue = i_b.read();
      *(target_bitmap + bytes_per_pixel * (width * y + i) + 2) = std::min(std::max(int(_factor * red + _bias), 0), 255);
      *(target_bitmap + bytes_per_pixel * (width * y + i) + 1) = std::min(std::max(int(_factor * green + _bias), 0), 255);
      *(target_bitmap + bytes_per_pixel * (width * y + i) + 0) = std::min(std::max(int(_factor * blue + _bias), 0), 255);
  }
}

```
In testbench.cpp, I use BUFFER to store row of pixels for calculating. Then I tranfer whole BUFFER to systemC module. When calculation finished the result will be sent back to testbench for image creating. 

### sobelfilter.cpp

```
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

```
In sobelfilter.cpp, the logic is same as testbench. Store data in data and calculate it.

# Additional features of your design and models

none

# Experimental results (on different benchmarks and settings)

## origin image
![lena_std_short](https://user-images.githubusercontent.com/76727373/112020617-74aa2000-8b6b-11eb-9897-ef471ba26100.png)
## req01 generated img
![req01](https://user-images.githubusercontent.com/76727373/112020712-8ab7e080-8b6b-11eb-8f3e-42f7223407b5.png)
## req02 generated img
![req01](https://user-images.githubusercontent.com/76727373/112020712-8ab7e080-8b6b-11eb-8f3e-42f7223407b5.png)

As we can see the result is the same from req01 and req02.

## pixels transfer
req01 pixels transfer count : 589824
req02 pixels transfer count : 196608
Difference : 393216(3 times)

# Discussions and conclusions.

This homework is quite hard, but it is without a doubet a good practice for us to learn systemC. Besides, we have to write our report in English and submit our homework to GitHub, which are not familiar to me. Anyway, thank you for providing us this challenge.
