#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <chrono>


using namespace std;
using namespace cv;

int main(int argc, char *argv[]){
  VideoCapture video("calibration_mslifecam.avi");
  if (!video.isOpened()){
    cout << "no se puedo acceder al video" << "\n";
    return -1;
  }
  // variables
  Mat plantilla, frame;
  double angle;
  double area;
  int i_frame;
  Point p_from;
  Point p_to;
  double thresh;
  
  // TODO: calcular el template automaticamente
  plantilla = imread("t6.jpg",0);

  thresh = 0.6;
  area = 2700.5;
  i_frame = 0;

  namedWindow("frame",0);
  namedWindow("template",0);
  namedWindow("aux",0);
  
  while (1) {
    auto start = chrono::high_resolution_clock::now();
    // read next frame from video
    video >> frame;
    if (frame.empty()){
      cout << "Finish video" << "\n";
      break;
    }
    int w_t = plantilla.cols;
    int h_t = plantilla.rows;
    
    Mat gray;
    cvtColor(frame, gray, CV_BGR2GRAY);

    // first time the ROI is all image
    if (i_frame == 0){
      p_from = Point(0,0);
      p_to = Point(frame.cols-1, frame.rows-1);
    }

    // only work in ROI
    Rect roi(p_from.x, p_from.y, p_to.x-p_from.x+1, p_to.y-p_from.y+1);
    Mat gray_roi = gray(roi);

    // maching applied, res is the result
    Mat res;
    res.create(gray_roi.rows-plantilla.rows+1,
	       gray_roi.cols-plantilla.cols+1, CV_32FC1);
    matchTemplate(gray_roi, plantilla, res, TM_CCOEFF_NORMED);

    // binary and find contours
    Mat res_b;
    threshold(res, res_b, thresh, 255, THRESH_BINARY);
    res_b.convertTo(res_b,CV_8UC1);
      
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(res_b, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    cout <<"aaaaaa"  << "\n";
    // get central points in contours
    vector<Point> rr;
    vector<double> values;

    for (auto cnt : contours) {
      Rect rect= boundingRect(cnt);
      // search maximun
      double minVal; double maxVal; Point minLoc; Point maxLoc;
      minMaxLoc(res(rect), &minVal, &maxVal, &minLoc, &maxLoc, Mat());
      rr.push_back(maxLoc+Point(rect.x, rect.y));
      values.push_back(maxVal);
    }

    //update the template
    if (rr.size()>0){
      auto sorted = values;
      sort(sorted.begin(), sorted.end(), greater<int>());
      vector<int> idxs;
      for (auto v : sorted){
	auto ptr = find(values.begin(), values.end(), v);
	if (ptr == values.end()){cout << "Error no found item" << "\n";}
	auto idx = distance(values.begin(), ptr);
	idxs.push_back(idx);
      }
      Point p = rr[idxs[0]]; // choose the best matching //(x, y)
      int pad = int(0.07*max(w_t, h_t));
      // TODO: proof boundaries
      Point pi(p.x-pad, p.y-pad);
      Rect roi_t(pi.x, pi.y, w_t+2*pad, h_t+2*pad);
      Mat tmp_pad = gray_roi(roi_t);

      // no implement . because choose the best matching
      // proof boundaries of template about gray_roi
      //if (pi.x<0 or pi.y<0 or p.x+w_t+pad> or p.y+h_t+pad){
      //}

      // binary, find contours and zoom the template
      Mat tmp_pad_b;
      threshold(tmp_pad, tmp_pad_b, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
      findContours(tmp_pad_b, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
      //for (auto cnt: contours){
      //Rect rect boundingRect(cnt); 
      //}

      // choose what contour is for anillo by diference between last and actual
      vector<int> areas_diff;
      for (auto cnt : contours){
	Rect rect = boundingRect(cnt);
	areas_diff.push_back(abs( - area));
      }
      //encuentro el contorno que tenga mas similutud con el anterior (historial)
      auto ptr_idx = min_element(areas_diff.begin(), areas_diff.end());
      int idx_a = distance(areas_diff.begin(), ptr_idx);

      // ahora veamos si este contorno pertecence al anillo
      Rect rect = boundingRect(contours[idx_a]);
      int new_area = rect.width*rect.height;
      double porcentaje = abs(100.0-new_area*100.0/area);
      if (porcentaje < 30.0){
	area = new_area;
	int pad = int(max(rect.width, rect.height)*0.1);
	Point pf1(pi.x+rect.x-pad, pi.y+rect.y-pad);
	Point pf2(pi.x+rect.x+rect.width+pad, pi.y+rect.y+rect.height+pad);
	Rect new_roi(pf1.x, pf1.y, pf2.x-pf1.x, pf2.y-pf1.y);
	plantilla = gray(new_roi);
      }else{
	cout << "Desbordamiento de area" << "\n";
      }
    }

    // update roi
    if (rr.size()>0){
      // reestablecer posicion global de rr
      for (auto& p : rr) {
	p.x = p.x+p_from.x;
	p.y = p.y+p_from.y;
      }
      // find the windows for ROI (closure all matching)
      auto ptr_x = minmax_element(rr.begin(),rr.end(), [](auto p1, auto p2){
	  return p1.x<p2.x;});
      auto ptr_y = minmax_element(rr.begin(),rr.end(), [](auto p1, auto p2){
	  return p1.y<p2.y;});
      Point roi_p1 (ptr_x.first->x, ptr_y.first->y);
      Point roi_p2 (ptr_x.second->x, ptr_y.second->y);
      roi_p2 += Point(w_t,h_t);
      // add padding
      int pad = int(0.1*max(abs(roi_p1.x-roi_p2.x), abs(roi_p1.y-roi_p2.y)));
      roi_p1 -= Point(pad, pad);
      roi_p2 += Point(pad, pad);
      // proof boundaries
      if (roi_p1.x < 0) roi_p1.x = 0;
      if (roi_p1.y < 0) roi_p1.y = 0;
      if (roi_p2.x > frame.cols-1) roi_p2.x = frame.cols-1;
      if (roi_p2.y > frame.rows-1) roi_p2.y = frame.rows-1;
      
      // TODO : draw actual roi
      
      // update roi for next frame
      p_from = roi_p1;
      p_to = roi_p2;
    }    

    // draw matching


    // show images
    imshow("frame", frame);
    
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> delay = finish-start;
    cout << "time is: "<<delay.count() << "\n";

    char c = (char)waitKey(1);
    if (c == 27) break;
  }
  
  video.release();
  destroyAllWindows();
  
  return 0;
}




















