#ifndef ENGINE_H
#define ENGINE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <chrono>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace std;
using namespace cv;

const double pi = acos(-1);

typedef vector<Point> cnt_t;

class Engine {
  double last_threshold;
  Mat frame;                   // image rgb original
  Mat frame_roi;               // image rgb original apply ROI
  Mat gray;                    // image gray original
  Mat gray_roi;                // gray aplicandole el ROI
  Mat binary_roi;
  Rect rect_roi;               // ROI

  vector<Point2f> centers;     // centers of rings
  vector<cnt_t> contours_f;    // contains rings like contours
  vector<Rect> rect_contours;  // respective rect of contours (rings)
  vector<int> orden;           // correct orden of rings

  Mat img_T;                   // points in space T
  double angle;                // angle to transformation
  double row_row;              // distance between rows
  int status_a;                // O+, O- , On

  double min_area;//6800;
  double max_area;//7000;
  float r_max = 1.1;
  float r_min = 0.9;
  bool flag_roi = true;       // reset roi, first time

  void updateROI();
  Point computeMaxRect();
  void globalPositionRect();
  void findRings();
  void transformation(bool);
  void updateAngle();
  void computeCenters();
  float distanceToRect(Point2f p1, Point2f p2, Point2f x);

  template<typename T>  void print(T actual);
  template<typename T, typename... Args>
  void print(T actual, Args... args);

public:  
  Engine();
  void setRoi(Rect r, double t);
  bool processing(Mat& frame_);
  bool getStatus();
  Mat getGrayROI();
  Mat getFrameROI();
  double getThreshold();
  Size getSizeRoi();
  void drawResults(int n_frame, double time);
  vector<Point2f> getPointsForCal();
  int numRingsDetected();

  void drawCentersAndRect(Mat& frame_);
  void showImages(vector<string> names);
};

#endif // ENGINE_H
