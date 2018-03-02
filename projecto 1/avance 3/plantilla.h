#ifndef PLANTILLA_H
#define PLANTILLA_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <omp.h>
#include <iostream>

#define INF 1234565
#define _INF -1000

using namespace cv;
using namespace std;

class Plantilla{
  Mat tmp_img;
  vector<Point> points;
  vector<double> values;
  vector<int> areas;
  int area;
  int idx_best_matching;
  //int width, height;
  void globalLocPoints(const Point& p_ref);
  void computeAreaAndCenter(const Mat& src_img, int dir);

  template<typename T>
  void print(vector<T> vec);

public:
  Plantilla(Mat img);
  Mat getImg()const;
  void setImg(Mat img);
  Size getSize();
  int getIdBestMatch()const;
  vector<Point> getPoints()const;
  void applyTemplate(const Mat& src_img);
  void updateTemplate(const Mat& src_img, const Point& p_ref, int dir);
  void drawPoints(Mat& img);
};

#endif // PLANTILLA_H
