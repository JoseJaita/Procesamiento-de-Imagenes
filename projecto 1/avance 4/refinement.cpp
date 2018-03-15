#include "refinement.h"

Refinement::Refinement(const Calibration& calibration_,string path_images_,int type_){
  calibration = calibration_;
  path_images = path_images_;
  pattern_type = type_;
}


bool Refinement::canonical(const Mat& src, Mat& dst, int idx){
  Mat gray;
  cvtColor(src, gray, COLOR_BGR2GRAY);

  // params of pattern
  Size img_size = calibration.getImageSize();
  Size pattern_size= calibration.getPatternSize();

  //cout << img_size<<endl;
  //cout << pattern_size << endl;

  vector<Point2f> points2D;
  bool found;
  switch (pattern_type) {
  case CHESSBOARD:
    found = findChessboardCorners(gray,pattern_size,points2D,
                                  CALIB_CB_ADAPTIVE_THRESH +CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
    if (found) cornerSubPix(gray, points2D, Size(11, 11), Size(-1, -1),TermCriteria(TermCriteria::EPS +TermCriteria::COUNT, 30, 0.1 ));
    //cout << "ssssssssssssssssssssss: "<<found<<endl;
    break;
  case CIRCLE:
    found = findCirclesGrid(gray,pattern_size,points2D,CALIB_CB_ASYMMETRIC_GRID);
    break;
  case RING:{
    Engine engine;
    Size s = (idx==-1)? test_roi : calibration.getSizeIdx(idx);
    double th = (idx==-1)? test_th : calibration.getThresholdIdx(idx);
    int w = int(1.3*s.width);
    int h = int(1.3*s.height);
    if (w>= src.cols) w = s.width;
    if (h>= src.rows) h = s.height;
    //cout << idx << " "<<calibration.getThresholdIdx(idx)<<" "<<s<<endl;
    engine.setRoi(Rect(0,0,w,h), th);
    Mat frame;
    src.copyTo(frame);
    //namedWindow("frame", 0);
    //namedWindow("binary", 0);
    //namedWindow("T", 0);
    for(int i =0 ;i<4;i++){
      engine.processing(frame);
      //engine.drawResults(i,0.0);
      //engine.showImages({"frame","binary","T"});
      //waitKey();
      src.copyTo(frame);
    }
    //destroyAllWindows();

    if (engine.numRingsDetected()==pattern_size.width*pattern_size.height){
      points2D=engine.getPointsForCal();
      found = true;
    }
    else found = false;
    break;
  }
  default:
    found = false;
  }

  if (found){
    vector<Point2f> pts1,pts2;
    // pts1 are initialize points, 4 corners
    pts1.push_back(points2D[0]);
    pts1.push_back(points2D[pattern_size.width-1]);
    pts1.push_back(points2D[pattern_size.width*(pattern_size.height-1)]);
    pts1.push_back(points2D[pattern_size.width*pattern_size.height-1]);

    // pts2 are for canonical
    float w_s = float(img_size.width)/float(pattern_size.width+1);
    float h_s = float(img_size.height)/float(pattern_size.height+1);
    pts2.push_back(Point2f(w_s, h_s));
    pts2.push_back(Point2f(img_size.width-w_s,h_s));
    pts2.push_back(Point2f(w_s,img_size.height-h_s));
    pts2.push_back(Point2f(img_size.width-w_s,img_size.height-h_s));

    //for(auto p : pts1) cout << p << endl;
    //cout <<endl;
    //for (auto p : pts2) cout << p <<endl;

    Mat P = getPerspectiveTransform(pts1,pts2);
    warpPerspective(src, dst, P, img_size);
  }
  else cout << "no found pattern"<<endl;

  return found;
}

/*
void Refinement::unDistort(Mat& src, Mat& dst, Mat& camera_matrix,Mat& dist_coeffs){
  undistort(src, dst, camera_matrix, dist_coeffs);
}
*/

bool Refinement::findPointsCanonical(const Mat& src, vector<Point2f>& points2D){
  Mat gray;
  cvtColor(src, gray,COLOR_BGR2GRAY);
  Size pattern_size = calibration.getPatternSize();
  bool found;
  switch (pattern_type) {
  case CHESSBOARD:
    found = findChessboardCorners(gray,pattern_size,points2D,
                                  CALIB_CB_ADAPTIVE_THRESH +CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
    if (found) cornerSubPix(gray, points2D, Size(11, 11), Size(-1, -1),TermCriteria(TermCriteria::EPS +TermCriteria::COUNT, 30, 0.1 ));
    //cout << "ssssssssssssssssssssss: "<<found<<endl;
    break;
  case CIRCLE:
    found = findCirclesGrid(gray,pattern_size,points2D,CALIB_CB_ASYMMETRIC_GRID);
    break;
  case RING:{
    Engine engine;
    Mat frame;
    src.copyTo(frame);
    //namedWindow("frame", 0);
    //namedWindow("binary", 0);
    //namedWindow("T", 0);
    for(int i =0 ;i<3;i++){
      engine.processing(frame);
      //engine.showImages({"frame","binary","T"});
      //waitKey();
    }
    //destroyAllWindows();

    if (engine.numRingsDetected()==pattern_size.width*pattern_size.height){
      points2D=engine.getPointsForCal();
      found = true;
    }
    else found = false;
    break;
  }
  default:
    found = false;
  }

  if (found){

  }else{
    cout << "No found pattern in image canonical"<<endl;
  }
  return found;
}

void Refinement::distortCenters(const vector<Point2f> centers,
                                vector<Point2f>& centers_dist){
  centers_dist.clear();
  Mat A = calibration.getCameraMatrix();
  Mat dist_coeffs = calibration.getDistCoeffs();
  auto fx = A.at<double>(0,0);
  auto fy = A.at<double>(1,1);
  auto cx = A.at<double>(0,2);
  auto cy = A.at<double>(1,2);
  auto k1 = -1.*dist_coeffs.at<double>(0,0);
  auto k2 = -1.*dist_coeffs.at<double>(1,0);
  auto k3 = -1.*dist_coeffs.at<double>(4,0);
  auto p1 = -1.*dist_coeffs.at<double>(2,0);
  auto p2 = -1.*dist_coeffs.at<double>(3,0);

  for(auto& p : centers){
    double x = (p.x-cx)/fx;
    double y = (p.y-cy)/fy;
    double r = x*x+y*y;
    double xdist = x*(1 + k1*r + k2*r*r + k3*r*r*r);
    double ydist = y*(1 + k1*r + k2*r*r + k3*r*r*r);

    xdist += 2.*p1*x*y+p2*(r+2*x*x);
    ydist += p1 * (r + 2.*y*y) + 2.*p2*x*y;

    xdist = xdist*fx + cx;
    ydist = ydist*fy + cy;

    centers_dist.push_back(Point2f((float)xdist,(float)ydist));
  }

}
void Refinement::distortPoints(const vector<Point2f> &xy,vector<Point2f> &uv,
                                Mat rvec, Mat tvec,const Mat &M,const Mat &d)
{

    vector<cv::Point3f>  xyz;
    for (Point2f p : xy)xyz.push_back(cv::Point3f(p.x, p.y, 1));
    cv::projectPoints(xyz, rvec, tvec, M, d, uv);
}

void Refinement::normalizePoints(const vector<Point2f>& src, vector<Point2f>& dst){
  Size pattern_size = calibration.getPatternSize();
  Size img_size = calibration.getImageSize();
  //float lenght = calibration.getLenght();
  float lenght = 1.0;
  dst.clear();
  for (auto p : src){
    p.x *= lenght*(pattern_size.width+1)/float(img_size.width);
    p.y *= lenght*(pattern_size.height+1)/float(img_size.height);
    dst.push_back(p);
  }
}

void Refinement::unProject(const Mat& src, Mat& dst, Mat A, Mat rotate, Mat translate){
  Rodrigues(rotate,rotate);
  Mat tran_rotate;
  transpose(rotate, tran_rotate);
  Mat inv_A = A.inv();
  Mat hom_rot = A*tran_rotate*inv_A;

  Mat c = -1*tran_rotate*translate;
  Mat u = -1*A*c/c.at<double>(2,0);
  u.at<double>(2,0) *= -1.0;
  Mat hom_tra = (Mat_<double>(2,3)<<1,0,0,0,1,0);
  transpose(u,u);
  hom_tra.push_back(u);
  transpose(hom_tra,hom_tra);

  Mat h = hom_tra*hom_rot;

  warpPerspective(src, dst, h, src.size());
}

string Refinement::processing(){
  vector<int> idx_images;
  for(int i=0;i<calibration.getNumFrames();i++) idx_images.push_back(i);
  //namedWindow("hola");
  for (int n_iter = 0; n_iter<2; n_iter++){
    vector<vector<Point2f>> last_points2D = calibration.getImagePoints();
    vector<Mat> rvects = calibration.getRotations();
    vector<Mat> tvects = calibration.getTranslations();
    Mat A = calibration.getCameraMatrix();
    Mat dist_coeffs = calibration.getDistCoeffs();
    int n_frames = calibration.getNumFrames();

    //cout << rvects.size()<<endl;

    // reset image points in calibration
    calibration.resetImagePoints();

    /*
    Mat rotate =(Mat_<double>(3,1) << -0.09820391,-0.09745882,-0.00132841);
    Mat A = (Mat_<double>(3,3)<<817.08826542,0,316.48914459,
             0,          818.0395153,   273.7127745,
             0,            0,            1        );
    Mat translate = (Mat_<double>(3,1)<<-5.88037917,-4.6666591,23.96738469);
    */

    vector<int> idx_images_good;
    for(int i=0; i < n_frames; i++){
      Mat rotate, translate, img_src;
      Mat img_undistord, img_unproject, img_canonical;

      rotate = rvects[i];
      translate = tvects[i];
      //translate.at<double>(0,0) += 2.0;
      //translate.at<double>(1,0) +=2.0;

      img_src = imread(path_images+to_string(idx_images[i])+".jpg");

      undistort(img_src, img_undistord, A, dist_coeffs);
      unProject(img_undistord, img_unproject, A, rotate, translate);

      bool r = canonical(img_unproject, img_canonical, i);
      if (!r) {cout << i<<endl;continue;}


      //imshow("hola",img_canonical);
      //waitKey();

      //translate.at<double>(0,0) -= 2.0;
      //translate.at<double>(1,0) -=2.0;

      vector<Point2f> points_canonical;
      vector<Point2f> points_c_norm;
      r = findPointsCanonical(img_canonical, points_canonical);
      if (!r) continue;
      normalizePoints(points_canonical, points_c_norm);
      /*
      cout << "points canonicallllllllllllllllllll"<<endl;
      //for (auto & p:points_canonical) cout << p << endl;
      cout << "points canonica normalizado"<<endl;
      //for (auto & p:points_c_norm) cout << p << endl;
      */

      vector<Point3f> object_p_canonical;
      for (auto& p : points_c_norm)
        object_p_canonical.push_back(Point3f(p.x,p.y,0));

      vector<Point2f> new_points2D, new_points2D_distorted;
      projectPoints(object_p_canonical,rotate,translate,A,dist_coeffs,new_points2D);
      distortCenters(new_points2D,new_points2D_distorted);

      //vector<Point2f> l_p_u, l_p_d;
      //undistortPoints(last_points2D[i],l_p_u,A,dist_coeffs);

      //for(int j =0;j<l_p_u.size();j++)
        //l_p_u[j]=Point2f(l_p_u[j].x+last_points2D[i][j].x,l_p_u[j].y+last_points2D[i][j].y);
      //distortCenters(l_p_u,l_p_d);
      //distortPoints(l_p_u,l_p_d,rvects[i],tvects[i],A,dist_coeffs);

      cout << i <<endl;
      for (int j =0; j<new_points2D.size();j++)
        //cout << last_points2D[i][j] << " "<<new_points2D[j]<<endl;;
        //cout << last_points2D[i][j] <<" "<<l_p_u[j]<<" "<<l_p_d[j]<<" "<<
        //        new_points2D[j]<<" "<<new_points2D_distorted[j]<<endl;
      /*cout << "last points"<<endl;
      for (auto a : last_points2D[i]) cout << a <<endl;
      cout << "new points" << endl;
      for(auto a : new_points2D) cout << a <<endl;
      cout << endl;
      cout << "new points distorted"<<endl;
      for(auto a : new_points2D_distorted) cout << a <<endl;
      */
      drawCenters(img_src,last_points2D[i],Scalar(0,0,255));
      drawCenters(img_src,new_points2D,Scalar(0,255,0));
      imshow("comparacion",img_src);
      waitKey();


      // add image points for new calibration
      calibration.addImagePoints(new_points2D);

      //this image is good for calibration
      idx_images_good.push_back(i);

      //cout << endl << "new points" << endl;
      //for (auto & p:new_points2D) cout << p << endl;

    /*
      imshow("src",img_src);
      imshow("undistord",img_undistord);
      imshow("unproject",img_unproject);
      imshow("canonical",img_canonical);
      waitKey();
      destroyAllWindows();
    */
    }
    // new calibration
    bool r = calibration.processing();
    if (!r) break;
    calibration.showResults();

    idx_images = idx_images_good;
  }
  //save new caibration
  destroyWindow("comparacion");
  string s = path_images+"new_params.xml";
  calibration.saveParams(s);
  return s;

}

bool Refinement::testCanonical(const Mat& src, Mat& dst, Mat& cambio_centers,Engine& egn,Calibration cal){
  /*
  vector<Point2f> a{Point2f(2,3),Point2f(29,89),Point2f(500,500)};
  vector<Point2f> b;
  distortCenters(a,b);
  cout << "rrrrrrrrrrrr"<<endl;
  for(auto p: b) cout << p << endl;
  */

  calibration = cal;
  Mat A = calibration.getCameraMatrix();
  Mat dist_coeffs = calibration.getDistCoeffs();

  test_th = egn.getThreshold();
  test_roi = egn.getSizeRoi();
  vector<Point2f> last_points2D = egn.getPointsForCal();

  Mat img_undistord, img_unproject, img_canonical;
  undistort(src, img_undistord, A, dist_coeffs);

  // get rotation and translation
  Mat rotate, translate;
  solvePnP(calibration.getObjectPoint(), last_points2D, A,dist_coeffs,rotate,translate);


  unProject(img_undistord, img_unproject, A, rotate, translate);
  bool r = canonical(img_unproject, dst,-1);

  if (r){
    // ya tenemos la imagen canonical entonces tenemos que
    // primero ver si lo centros permanecen igual, por mas que el frame de entrada cambie
    // segundo ver si lo puntos re-proyetados de la canonical son parecidos a los originales
    Engine engine;
    vector<Point2f> points_c_norm, new_points2D;
    vector<Point3f> object_p_canonical;
    engine.processing(dst);
    normalizePoints(engine.getPointsForCal(), points_c_norm);
    for (auto& p : points_c_norm) object_p_canonical.push_back(Point3f(p.x,p.y,0));
    projectPoints(object_p_canonical,rotate,translate,A,dist_coeffs,new_points2D);

    src.copyTo(cambio_centers);
    drawCenters(cambio_centers,last_points2D,Scalar(0,0,255));
    drawCenters(cambio_centers,new_points2D,Scalar(0,255,0));

    /*
    cout << "last points" <<endl;
    for(auto&p : last_points2D) cout << p <<endl;
    cout << "new points" <<endl;
    for(auto&p : new_points2D) cout << p <<endl;
    */

    engine.drawCentersAndRect(dst);
    return true;
  }else
    return false;
}


void Refinement::drawCenters(Mat& img, vector<Point2f> centers,Scalar color){
  for (auto p : centers){
    Point p1(round(p.x),round(p.y));
    Point p2 = p1;
    p1.y -= 5; p2.y += 5;

    Point p3(round(p.x),round(p.y));
    Point p4 = p3;
    p3.x -= 5; p4.x += 5;

    if(p1.y<0) p1.y=0;
    if(p2.y>=img.rows) p2.y=img.rows-1;
    if(p3.x<0) p1.x=0;
    if(p4.x>=img.cols) p4.x=img.cols-1;

    line(img,p1,p2,color);
    line(img,p3,p4,color);
  }
}
