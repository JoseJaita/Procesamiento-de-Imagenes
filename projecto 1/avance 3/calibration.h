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
  Size image_size;
  Size pattern_size;
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

public:
  Calibration();
  Mat getCameraMatrix();
  Mat getDistCoeffs();
  void setImageSize(Size size_);
  void setPatternSize(Size size_);
  void addImagePoints(vector<Point2f> points_);
  bool processing();
  void showResults();
  void saveParams(string file);
  bool readXml(string file);
};

#endif // CALIBRATION_H
