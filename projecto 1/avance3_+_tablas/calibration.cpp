#include "calibration.h"

Calibration::Calibration(){
  camera_matrix = Mat::eye(3,3,CV_64F);
  dist_coeffs = Mat::zeros(8,1,CV_64F);
  n_frames = 0;
}

bool Calibration::readXml(string file){
  return false;
}

void Calibration::saveParams(string filename){
  /*
  FileStorage fs( filename, FileStorage::WRITE );
  fs << "Calibrate_Accuracy" << RMS;
  fs << "Camera_Matrix" << cameraMatrix;
  fs << "Distortion_Coefficients" << distCoeffs;
  fs << "Rotation_Vector" << rvecs;
  fs << "Translation_vector" << tvecs;

  if( !rvecs.empty() && !tvecs.empty() ){

    CV_Assert(rvecs[0].type() == tvecs[0].type());
    Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
    for( int i = 0; i < (int)rvecs.size(); i++ ){
      Mat r = bigmat(Range(i, i+1), Range(0,3));
      Mat t = bigmat(Range(i, i+1), Range(3,6));

      CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
      CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);

      r = rvecs[i].t();
      t = tvecs[i].t();
      }
      cvWriteComment( *fs, "Rotation vector + Translation vector", 0 );
      fs << "extrinsic_parameters" << bigmat;
  }
  fs.release();*/
}

Mat Calibration::getCameraMatrix() {return camera_matrix;}
Mat Calibration::getDistCoeffs() {return dist_coeffs;}

void Calibration::setObjectPoints(){
  object_points.clear();
  vector <Point3f> tmp;
  if (asy){
    for (int y = 0; y < pattern_size.height; y++)
      for (int x = 0; x < pattern_size.width; x++)
        if (y%2 == 0) tmp.push_back(Point3f(2*x, y, 0));
        else tmp.push_back(Point3f(2*x+1,y,0));
  }
  else{
  for (int y = 0; y < pattern_size.height; y++)
    for (int x = 0; x < pattern_size.width; x++)
      tmp.push_back(Point3f(x, y, 0));
  }


  // now the same for all_images
  for(int i =0; i < n_frames;i++)
    object_points.push_back(tmp);
}

//---------- functions to print variables words number ---------------------
template<typename T>
void Calibration::print(T first){cout << first <<endl;}

template<typename T, typename... Args>
void Calibration::print(T first, Args... args){
  cout << first;
  print(args...);
}

//------------------------------------------------------------------

void Calibration::addImagePoints(vector<Point2f> points_){
  image_points.push_back(points_);
  n_frames++;
}

void Calibration::setPatternSize(Size size_, bool asy_){
  std::cout << asy_ << "\n";
  pattern_size = size_;
  asy = asy_;
}

void Calibration::setImageSize(Size size_){ image_size = size_;}

void Calibration::showResults(){
  print("rms: ", rms);
  print("Camera Matrix:\n", camera_matrix);
  print("Dist Coeffs:\n", dist_coeffs);
}

bool Calibration::processing(){
  setObjectPoints();

  // check all variables are correct
  int n_points = pattern_size.height*pattern_size.width;

  if (image_points.size()==object_points.size() &&
      image_points.size()>0 &&
      n_points == image_points[0].size()){
    //rms = calibrateCamera(object_points,image_points,image_size,camera_matrix,dist_coeffs,rvecs,tvecs,
    //                      CALIB_FIX_PRINCIPAL_POINT+CALIB_FIX_ASPECT_RATIO +CALIB_ZERO_TANGENT_DIST,
    //                      TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 30, 2.22e-16));
    rms = calibrateCamera(object_points,image_points,image_size,camera_matrix,dist_coeffs,rvecs,tvecs, CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5);
    return true;
  }
  else{
    print ("the parameters no match");
    return false;}
}


