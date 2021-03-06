#include <cassert>
#include <cstdio>
#include <cstdlib>
using namespace std;

#include "Testbench.h"

unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};

Testbench::Testbench(sc_module_name n)
    : sc_module(n), output_rgb_raw_data_offset(54) {
  SC_THREAD(do_sobel);
  sensitive << i_clk.pos();
  dont_initialize();
}

int Testbench::read_bmp(string infile_name) {
  FILE *fp_s = NULL; // source file handler
  fp_s = fopen(infile_name.c_str(), "rb");
  if (fp_s == NULL) {
    printf("fopen %s error\n", infile_name.c_str());
    return -1;
  }
  // move offset to 10 to find rgb raw data offset
  fseek(fp_s, 10, SEEK_SET);
  assert(fread(&input_rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));

  // move offset to 18 to get width & height;
  fseek(fp_s, 18, SEEK_SET);
  assert(fread(&width, sizeof(unsigned int), 1, fp_s));
  assert(fread(&height, sizeof(unsigned int), 1, fp_s));

  // get bit per pixel
  fseek(fp_s, 28, SEEK_SET);
  assert(fread(&bits_per_pixel, sizeof(unsigned short), 1, fp_s));
  bytes_per_pixel = bits_per_pixel / 8;

  // move offset to input_rgb_raw_data_offset to get RGB raw data
  fseek(fp_s, input_rgb_raw_data_offset, SEEK_SET);

  source_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (source_bitmap == NULL) {
    printf("malloc images_s error\n");
    return -1;
  }

  target_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (target_bitmap == NULL) {
    printf("malloc target_bitmap error\n");
    return -1;
  }

  assert(fread(source_bitmap, sizeof(unsigned char),
               (size_t)(long)width * height * bytes_per_pixel, fp_s));
  fclose(fp_s);
  return 0;
}

int Testbench::write_bmp(string outfile_name) {
  FILE *fp_t = NULL;      // target file handler
  unsigned int file_size; // file size

  fp_t = fopen(outfile_name.c_str(), "wb");
  if (fp_t == NULL) {
    printf("fopen %s error\n", outfile_name.c_str());
    return -1;
  }

  // file size
  file_size = width * height * bytes_per_pixel + output_rgb_raw_data_offset;
  header[2] = (unsigned char)(file_size & 0x000000ff);
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  // width
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  // height
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  // bit per pixel
  header[28] = bits_per_pixel;

  // write header
  fwrite(header, sizeof(unsigned char), output_rgb_raw_data_offset, fp_t);

  // write image
  fwrite(target_bitmap, sizeof(unsigned char),
         (size_t)(long)width * height * bytes_per_pixel, fp_t);

  fclose(fp_t);
  return 0;
}

void Testbench::do_sobel() {
  int x, y, i;        // for loop counter
  unsigned char BUFFER[3][520][3];
  double red = 0.0,blue = 0.0,green = 0.0;
  int pixel_transfer_c = 0;

  o_rst.write(false);
  wait(5);
  o_rst.write(true);
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
  std::cout << "pixel_transfer_c = " << pixel_transfer_c << std::endl;
  sc_stop();
}
