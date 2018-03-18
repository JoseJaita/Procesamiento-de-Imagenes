#include "calibration.h"

Calibration::Calibration(){
  camera_matrix = Mat::eye(3,3,CV_64F);
  dist_coeffs = Mat::zeros(8,1,CV_64F);
  n_frames = 0;
}

bool Calibration::readXml(string filename){
  FileStorage fs(filename, FileStorage::READ);
  fs["Calibrate_Accuracy"] >> rms;
  fs["Camera_Matrix"] >> camera_matrix;
  fs["Distortion_Coefficients"] >> dist_coeffs;
  fs["Rotation_Vector"] >> rvecs;
  fs["Translation_vector"] >> tvecs;

  fs["image_size"] >> image_size;
  fs["pattern_size"] >> pattern_size;
  fs["n_frames"] >> n_frames;
  fs["asy"] >> asy;
  fs["thresholds"] >>thresholds;
  fs["sizes"] >> sizes;
  fs["image_points"] >> image_points;
  fs["lenght"] >> length;

  //cout << rms << endl;
  //cout << camera_matrix << endl;

  return true;
}

void Calibration::saveParams(string filename){

  FileStorage fs(filename, FileStorage::WRITE);
  fs << "Calibrate_Accuracy" << rms;
  fs << "Camera_Matrix" << camera_matrix;
  fs << "Distortion_Coefficients" << dist_coeffs;
  fs << "Rotation_Vector" << rvecs;
  fs << "Translation_vector" << tvecs;

  fs << "image_size"<<image_size;
  fs << "pattern_size"<<pattern_size;
  fs << "n_frames" << n_frames;
  fs << "asy" << asy;
  fs << "thresholds" << thresholds;
  fs << "sizes" << sizes;
  fs << "image_points" << image_points;
  fs << "lenght" << length;

  if(!rvecs.empty() && !tvecs.empty()){

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
  fs.release();
}

vector<vector<Point2f>> Calibration::getImagePoints() {return image_points;}
double Calibration::getLength(){return length;}
Mat Calibration::getCameraMatrix() {return camera_matrix;}
Mat Calibration::getDistCoeffs() {return dist_coeffs;}
vector<Mat> Calibration::getRotations() {return rvecs;}
vector<Mat> Calibration::getTranslations() {return tvecs;}
int Calibration::getNumFrames(){return n_frames;}

Size Calibration::getImageSize() {return image_size;}
Size Calibration::getPatternSize() {return pattern_size;}
vector<Point3f> Calibration::getObjectPoint(){
  if (object_points.empty()) setObjectPoints();
  return object_points[0];}

void Calibration::setObjectPoints(){
  object_points.clear();
  vector <Point3f> tmp;
  if (asy){
    for (int y = 0; y < pattern_size.height; y++)
      for (int x = 0; x < pattern_size.width; x++)
        if (y%2 == 0) tmp.push_back(Point3f((2*x+1.)*length, (y+1.)*length, 0));
        else tmp.push_back(Point3f((2*x+1+1)*length, (y+1)*length, 0));
  }
  else{
  for (int y = 0; y < pattern_size.height; y++)
    for (int x = 0; x < pattern_size.width; x++)
      tmp.push_back(Point3f((x+1.)*length, (y+1)*length, 0));
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

void Calibration::resetImagePoints(){
  image_points.clear();
  n_frames = 0;
}

void Calibration::addImagePoints(vector<Point2f> points_){
  image_points.push_back(points_);
  n_frames++;
}
void Calibration::addThreshold(double t){
  thresholds.push_back(t);
}
void Calibration::addSizeRoi(Size s){sizes.push_back(s);}
double Calibration::getThresholdIdx(int idx){return thresholds[idx];}
Size Calibration::getSizeIdx(int idx){return sizes[idx];}

void Calibration::setPatternSize(Size size_, double length_,bool asy_){
  //std::cout << asy_ << "\n";
  pattern_size = size_;
  asy = asy_;
  length = length_;
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
    rms = calibrateCamera(object_points,image_points,image_size,camera_matrix,dist_coeffs,rvecs,tvecs,
                          CV_CALIB_ZERO_TANGENT_DIST);

    cout << "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu"<<endl;
    //for(auto p: object_points) cout << p<<endl;
    //cout << object_points[0]<<endl;
    //cout << image_points[0] <<endl;
    //for(auto p:image_points) cout << p<<endl;
    cout <<"uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu"<<endl;

    return true;
  }
  else{
    print ("the parameters no match");
    return false;}
}


float Calibration::distanceToRect(Point2f p1, Point2f p2, Point2f x) {
    return abs((p2.y - p1.y) * x.x - (p2.x - p1.x) * x.y + p2.x * p1.y - p2.y * p1.x) / sqrt(pow(p2.y - p1.y, 2) + pow(p2.x - p1.x, 2));
}
void Calibration::drawCentersAndRect(Mat& frame_, vector<Point2f>& points){
  if (points.size() == pattern_size.width*pattern_size.height){
    float error = 0.0f;
    for (int i=0; i<pattern_size.height; i++){
      auto p1 = points[i*pattern_size.width];
      auto p2 = points[i*pattern_size.width+pattern_size.width-1];
      line(frame_,p1, p2,Scalar(0,0,255),1);
      // compute distance to rect collineary
      for(int j = 1; j < pattern_size.width-1; j++){
        error += distanceToRect(p1,p2, points[i*pattern_size.width+j]);
      }
    }
    error /= (pattern_size.height*(pattern_size.width-2));
    cout << "ERROR OF COLLINEARY: " << error << endl;
    for(auto&p : points)
      circle(frame_,p,1,Scalar(0,255,0));
  }
}

