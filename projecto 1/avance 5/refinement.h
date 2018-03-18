#ifndef REFINEMENT_H
#define REFINEMENT_H

#include "calibration.h"
#include "engine.h"

#define CHESSBOARD 0
#define CIRCLE 1
#define RING 2

class Refinement{
  Calibration calibration;
  string path_images;
  int pattern_type;

  // to test canonical
  double test_th;
  Size test_roi;

  void unProject(const Mat& src, Mat& dst, Mat& H,Mat A, Mat rotate, Mat translate);
  bool canonical(const Mat& src, Mat& dst, Mat& H, int idx=-1);
  bool findPointsCanonical(const Mat& src, vector<Point2f>& points2D);
  void normalizePoints(const vector<Point2f>& src, vector<Point2f>& dst);
  void distortCenters(const vector<Point2f> centers,vector<Point2f>& centers_dist);
  void distortPoints(const vector<Point2f> &xy, vector<Point2f> &uv,Mat rvec,Mat tvec,
                     const Mat &M, const Mat &d);

public:
  Refinement(const Calibration& calibration_, string path_images_, int type_);
  string processing();
  bool testCanonical(const Mat& src, Mat& dst,Mat& cambio_centers,Engine& egn, Calibration cal);
  void drawCenters(Mat& img, vector<Point2f> centers,Scalar color);
};

#endif // REFINEMENT_H
