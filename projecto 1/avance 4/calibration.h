#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

using namespace std;
using namespace cv;

class Calibration{
  int n_frames;
  bool asy;
  Size image_size;
  Size pattern_size;
  vector<double> thresholds;
  vector<Size> sizes;
  vector<vector<Point2f>> image_points;
  vector<vector<Point3f>> object_points;

  Mat camera_matrix;
  Mat dist_coeffs;
  vector<Mat> rvecs;
  vector<Mat> tvecs;
  double rms;

  void setObjectPoints();

  template<typename T>
  void print(T first);
  template<typename T, typename... Args>
  void print(T first, Args... args);

  float distanceToRect(Point2f p1, Point2f p2, Point2f x);

public:
  Calibration();
  Mat getCameraMatrix();
  Mat getDistCoeffs();
  vector<Mat> getRotations();
  vector<Mat> getTranslations();
  vector<vector<Point2f>> getImagePoints();
  Size getImageSize();
  Size getPatternSize();
  vector<Point3f> getObjectPoint();
  int getNumFrames();

  void setImageSize(Size size_);
  void setPatternSize(Size size_, bool asy_=false);
  void addThreshold(double t);
  void addSizeRoi(Size s);
  double getThresholdIdx(int idx);
  Size getSizeIdx(int idx);
  void addImagePoints(vector<Point2f> points_);
  void resetImagePoints();
  bool processing();
  void showResults();
  void saveParams(string file);
  bool readXml(string file);
  void drawCentersAndRect(Mat& frame_, vector<Point2f>& points);

};

#endif // CALIBRATION_H
