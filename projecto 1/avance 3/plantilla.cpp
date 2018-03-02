#include "plantilla.h"


Plantilla::Plantilla(Mat img):tmp_img(img){
  // TODO : compute area first time
  area = 2700;
}

void Plantilla::setImg(Mat img){tmp_img = img;}
Mat Plantilla::getImg()const{return tmp_img;}
int Plantilla::getIdBestMatch()const{return idx_best_matching;}

Size Plantilla::getSize(){
  return Size(tmp_img.cols, tmp_img.rows);
}

template<typename T>
void Plantilla::print(vector<T> vec){
  cout << "Areas: " <<endl;
  for(auto v : vec)
    cout << v <<endl;
  cout << endl;
}

vector<Point> Plantilla::getPoints()const { return points;}

void Plantilla::drawPoints(Mat& img){
  auto f = [&img, this](const Point& p){
    Rect rect(p.x, p.y, tmp_img.cols, tmp_img.rows);
    rectangle(img, rect, Scalar(0,0,255), 2);
  };
  for_each(points.begin(), points.end(), f);

}

// actualiza los points con respecto al global
// la referencia lo pasa engine
void Plantilla::globalLocPoints(const Point& p_ref){
  auto f = [&p_ref, this](Point& p){p += Point(p_ref.x, p_ref.y);};
  for_each(points.begin(), points.end(), f);
}

// por cada match calcula el area del match y su center
void Plantilla::computeAreaAndCenter(const Mat& src_img, int dir){
  areas.clear();
  for (auto p : points) {
    // compute rect
    int w_t = tmp_img.cols;
    int h_t = tmp_img.rows;
    int pad = int(0.10*max(w_t, h_t));
    p -= Point(pad, pad);
    Rect rec_t (p.x, p.y, w_t+2*pad, h_t+2*pad);
    // proof boundaries
    if (rec_t.x<0 || rec_t.y<0 || rec_t.x+rec_t.width>src_img.cols ||
        rec_t.y+rec_t.height>src_img.rows){
      cout << "this match is out of boundaries"<<endl;
      areas.push_back((dir==-1)? _INF : INF);
      continue;
    }

    Mat img_new_t = src_img(rec_t);
    // binary find contorus and add padding
    Mat img_new_b_t;
    threshold(img_new_t, img_new_b_t, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(img_new_b_t, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    // choose what contours is for ring
    if (contours.empty()){
      cout << "Match found, but no found contours in new template"<<endl;
      return ;
    }
    vector<int> areas_diff;
    for (auto cnt : contours){
      Rect rect = boundingRect(cnt);
      areas_diff.push_back(abs(int(rect.area())));
    }
    // find contours with minumun diference of area
    auto ptr = max_element(areas_diff.begin(), areas_diff.end());
    int idx = distance(areas_diff.begin(), ptr);
    // now check, that contours in for ring
    Rect rect = boundingRect(contours[idx]);
    int new_area = rect.area();
    double porcentaje = abs(100.0-new_area*100.0/area);
    if (porcentaje < 30.0){
      areas.push_back(new_area);
    } else {
      areas.push_back((dir==-1)? _INF : INF); // this match will not be used
      cout << "area overflow" << endl;
    }
  }
}


// la actualizacion del template se hace con respecto a la imagen global
// no con respeto a la roi
// por eso previamente se debe poner lo puntos con respecto ala posicion global
void Plantilla::updateTemplate(const Mat& src_img, const Point& p_ref, int dir){
  cout << "start update template" <<endl;
  if (points.empty()) return;

  // points to location global
  globalLocPoints(p_ref);

  /*
  // get the best matching
  auto sorted = values;
  sort(sorted.begin(), sorted.end(), greater<double>());
  vector<int> idxs;
  for (auto v : sorted){
    auto ptr = find(values.begin(), values.end(), v);
    if (ptr == values.end()){cout << "Error no found item" << "\n";}
    auto idx = distance(values.begin(), ptr);
    idxs.push_back(idx);
  }*/

  // compute areas of all match
  computeAreaAndCenter(src_img, dir);
  print(areas);
  // sort about areas, can be max or min area, depend dir
  int d;
  auto ptr_a = minmax_element(areas.begin(), areas.end());
  if (dir == -1) d = distance(areas.begin(), ptr_a.second);
  else {
    d = distance(values.begin(),max_element(values.begin(), values.end()));
    //d = distance(areas.begin(), ptr_a.first);
  }
  // select point
  Point p = points[d];
  // set what is the match selected
  idx_best_matching = d;

  // build the new template
  int w_t = tmp_img.cols;
  int h_t = tmp_img.rows;
  int pad = int(0.07*max(w_t, h_t));
  p -= Point(pad, pad);
  Rect rec_t(p.x, p.y, w_t+2*pad, h_t+2*pad);
  /*
  // save what points will be used like template
  int pos;
  if (dir == -1) pos = points.size()-1;
  //idx_best_matching = idxs[pos];
  //idx_best_matching = pos;
  // choose matching
  //Point p = points[idxs[0]];
  //Point p = points[pos];
  // build the new template
  int w_t = tmp_img.cols;
  int h_t = tmp_img.rows;
  int pad = int(0.07*max(w_t, h_t));
  // apply pad
  //p -= Point(pad, pad);
  Rect rec_t;
  Point p;
  // TODO: proof boundaries

  bool find_pattern = false;
  for(int i=0; i<points.size();i++){
    int k = (dir==-1)? pos-i : i;
    p = points[k];
    p -= Point(pad, pad);
    rec_t = Rect(p.x, p.y, w_t+2*pad, h_t+2*pad);
    // proof boundaries
    if (rec_t.x<0 || rec_t.y<0 || rec_t.x+rec_t.width>src_img.cols ||
        rec_t.y+rec_t.height>src_img.rows){
      cout << "next pattern "<<endl;
    }else{find_pattern = true; idx_best_matching = k; break;}
  }
  if (!find_pattern) {cout << "can't find pattern"<<endl;return;}
  */

  cout << src_img.cols<<" "<<src_img.rows<<endl;
  cout << rec_t <<endl;
  Mat img_new_t = src_img(rec_t);
  // binary find contorus and add padding
  Mat img_new_b_t;
  threshold(img_new_t, img_new_b_t, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(img_new_b_t, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
  // choose what contours is for ring
  if (contours.empty()){
    cout << "Match found, but no found contours in new template"<<endl;
    return ;
  }
  vector<int> areas_diff;
  for (auto cnt : contours){
    Rect rect = boundingRect(cnt);
    areas_diff.push_back(abs(int(rect.area())-area));
  }
  // find contours with minumun diference of area
  auto ptr = min_element(areas_diff.begin(), areas_diff.end());
  int idx = distance(areas_diff.begin(), ptr);
  // now check, that contours in for ring
  Rect rect = boundingRect(contours[idx]);
  int new_area = rect.area();
  double porcentaje = abs(100.0-new_area*100.0/area);
  if (porcentaje < 30.0){
    area = new_area;
    pad = int(max(rect.width, rect.height)*0.1);
    // TODO: proof boundaries
    // extract template from gray image (roi)
    int x = p.x+rect.x-pad;
    int y = p.y+rect.y-pad;
    int w = rect.width+2*pad;
    int h = rect.height+2*pad;
    Rect new_rect (x, y, w, h);
    //cout << new_rect << endl;
    src_img(new_rect).copyTo(tmp_img);
  } else {
    cout << "area overflow" << endl;
  }

  cout << "end update template" <<endl;
}


void Plantilla::applyTemplate(const Mat& src_img){
  Mat match_img, m_b_i, m_b_255_i;
  // apply template matching
  matchTemplate(src_img, tmp_img, match_img, TM_CCOEFF_NORMED);
  // binary
  threshold(match_img, m_b_i, 0.6, 1, THRESH_BINARY);
  m_b_i.convertTo(m_b_255_i, CV_8UC1, 255);
  // find contours
  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(m_b_255_i, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
  // find points ans its values
  points.clear();
  values.clear();
  for (auto cnt : contours){
    Rect rect = boundingRect(cnt);
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    minMaxLoc(match_img(rect), &minVal, &maxVal, &minLoc, &maxLoc, Mat());
    points.push_back(maxLoc+Point(rect.x, rect.y));
    values.push_back(maxVal);
  }
  //for(auto p : points){
  //  cout <<p<<endl;
  //}
  //cout << endl;
}


