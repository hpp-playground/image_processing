/********fileio.cpp  **************/

#include <stdio.h>
#include <stdlib.h>

/******** 画像の読み込み，保存 *****************/

typedef struct _image {
  int w;
  int h;
  int ch;
  unsigned char *data;
} Image;

// Imageの確保
Image *new_image(int w, int h, int ch) {
  Image *img = (Image *)malloc(sizeof(Image));
  img->w = w;
  img->h = h;
  img->ch = ch;
  int size = w * h * ch;
  img->data = (unsigned char *)malloc(sizeof(unsigned char) * size);
  //ゼロクリア
  for (int i = 0; i < size; i++) img->data[i] = 0;
  return img;
}

// Imageの開放
void delete_image(Image *img) {
  delete img->data;
  delete img;
}

#define BUFF_LEN 256
// PGM画像をロードする
Image *load_image(const char *fname) {
  FILE *fp = fopen(fname, "rb");
  if (fp == NULL) {
    fprintf(stderr, "ERROR : failed to open file.\n");
    return NULL;
  }

  char buff[BUFF_LEN];
  int count = 0;
  int type = 0;
  int w, h;
  int max;
  while (count < 3) {
    if (fgets(buff, BUFF_LEN, fp) == NULL) return NULL;
    if (buff[0] == '#')  //コメントはスキップ
      continue;
    switch (count) {
      case 0:  // type
        if (buff[0] != 'P') {
          fprintf(stderr, "ERROR : unknown format\n");
          return NULL;
        }
        type = buff[1] - '0';
        break;
      case 1:  // w h
        sscanf(buff, "%d %d", &w, &h);
        break;
      case 2:  // max
        sscanf(buff, "%d", &max);
        break;
    }
    count++;
  }

  if (type == 5) {  // pgm
    Image *img = new_image(w, h, 1);
    int size = w * h;
    for (int i = 0; i < size; i++) {
      int c = getc(fp);
      if (c < 0) {  //データがもうない．
        delete_image(img);
        fprintf(stderr, "ERROR : too short data\n");
        fclose(fp);
        return NULL;
      }
      img->data[i] = (unsigned char)c;
    }
    fclose(fp);
    return img;
  } else {
    //未対応
    fprintf(stderr, "ERROR : unknown format number : %d\n", type);
    fclose(fp);
    return NULL;
  }
}

void save_image(const char *fname, Image *img) {
  FILE *fp = fopen(fname, "wb");

  if (img->ch == 3) {  // PPM
    fprintf(fp, "P6\n");
  } else if (img->ch == 1) {  // PGM
    fprintf(fp, "P5\n");
  }
  fprintf(fp, "%d %d\n", img->w, img->h);
  fprintf(fp, "255\n");
  int size = img->w * img->h * img->ch;
  for (int i = 0; i < size; i++) putc(img->data[i], fp);
  fclose(fp);
}

/********** 画像の読み込み，保存　ここまで*******************************/

void bayer_normal(Image *img, Image *result) {
  printf("bayer normal\n");

  int w = img->w;
  int h = img->h;
  int ch = img->ch;

  Image *red = new_image(w, h, 1);
  Image *green = new_image(w, h, 1);
  Image *blue = new_image(w, h, 1);

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      int k = i * w + j;
      if ((i % 2) && (j % 2)) {
        blue->data[k] = img->data[k];
      } else if (((i % 2) && !(j % 2)) || (!(i % 2) && (j % 2))) {
        green->data[k] = img->data[k];
      } else {
        red->data[k] = img->data[k];
      }
    }
  }

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      int k = i * w + j;
      if ((i % 2) && (j % 2)) {
        blue->data[k + 1] = (blue->data[k] + blue->data[k + 2]) / 2;
        blue->data[k + w] = (blue->data[k] + blue->data[k + 2 * w]) / 2;
        blue->data[k + w + 1] =
            (blue->data[k] + blue->data[k + 2] + blue->data[k + 2 * w] +
             blue->data[k + 2 * w + 2]) /
            4;
      } else if (((i % 2) && !(j % 2)) || (!(i % 2) && (j % 2))) {
        green->data[k + 1] = (green->data[k] + green->data[k + 2] +
                              green->data[k - w + 1] + green->data[k + w + 1]) /
                             4;
      } else {
        red->data[k + 1] = (red->data[k] + red->data[k + 2]) / 2;
        red->data[k + w] = (red->data[k] + red->data[k + 2 * w]) / 2;

        red->data[k + w + 1] =
            (red->data[k] + red->data[k + 2] + red->data[k + 2 * w] +
             red->data[k + 2 * w + 2]) /
            4;
      }
    }
  }

  for (int i = 0; i < w * h; i++) {
    result->data[i * 3] = red->data[i];
    result->data[i * 3 + 1] = green->data[i];
    result->data[i * 3 + 2] = blue->data[i];
  }
  return;
}

int main(int argc, char *argv[]) {
  const char *fname = "input.pgm";
  if (argc == 2) fname = argv[1];
  //ファイルの読み込み
  Image *img = load_image(fname);
  if (img == NULL) {
    fprintf(stderr, "Failed to load image : %s\n", fname);
    return 1;
  }

  //結果保存用画像
  Image *result = new_image(img->w, img->h, 3);

  //ベイヤー補間
  bayer_normal(img, result);

  //結果の保存
  save_image("output.ppm", result);

  //後始末
  delete_image(img);
  delete_image(result);

  return 0;
}
