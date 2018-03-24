#include "engine.h"

Engine::Engine(){
  // TODO: this should be atomatically
  last_threshold = 50;
  /*
  rect_roi = Rect(0,0,400,400);
  min_area = 2200;
  max_area = 2300;
  */

  min_area = 500;
  max_area = 7000;


  /*
  rect_roi = Rect(483,150,371,301);
  min_area = 2200;
  max_area = 2300;
  */

  /*
  rect_roi = Rect(672,192,654,510);
  min_area = 6500;
  max_area = 6700;
  */
  // transformation
  row_row = 1000.0;
  angle = 0.0;
  status_a = 0;
}
void Engine::setRoi(Rect r, double t){
  rect_roi = r;
  flag_roi = false;
  last_threshold = t;
}

Mat Engine::getGrayROI(){return gray_roi;}
Mat Engine::getFrameROI(){return frame_roi;}
double Engine::getThreshold(){return last_threshold;}
Size Engine::getSizeRoi(){return Size(rect_roi.width-1,rect_roi.height-1);}

int Engine::numRingsDetected(){return rect_contours.size();}

float Engine::distanceToRect(Point2f p1, Point2f p2, Point2f x) {
    return abs((p2.y - p1.y) * x.x - (p2.x - p1.x) * x.y + p2.x * p1.y - p2.y * p1.x) / sqrt(pow(p2.y - p1.y, 2) + pow(p2.x - p1.x, 2));
}

// draw 0 centers, rect and compute collineary
void Engine::drawCentersAndRect(Mat& frame_){
  vector<Point2f> points = getPointsForCal();

  /*for(auto p : points)
    cout << p << endl;*/

  if (points.size() == 20){
    float error = 0.0f;
    for (int i=0; i<4; i++){
      auto p1 = points[i*5];
      auto p2 = points[i*5+4];
      line(frame_,p1, p2,Scalar(0,0,255),1);
      // compute distance to rect collineary
      for(int j = 1; j < 4; j++){
        error += distanceToRect(p1,p2, points[i*5+j]);
      }
    }
    error /= 12.0;
    cout << error << endl;
    for(auto&p : points){
      circle(frame_,p,1,Scalar(0,255,0));

      //cout << p<<endl;
    }
  }
}

void Engine::computeCenters(){
  centers.clear();
  Mat tmp_g, tmp_b;
  for (auto& r : rect_contours){
    vector<cnt_t> contours;
    vector<Vec4i> hierarchy;
    gray(r).copyTo(tmp_g);
    threshold(tmp_g, tmp_b, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

    findContours(tmp_b, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    // TODO proof size of contours
    //cout << contours.size()<<endl;
    auto cnt = contours[0];
    //auto cnt2 = contours[1];
    auto mu = moments(cnt,true);
    //auto mu2 = moments(cnt2,true);
    //cout << Point2f(mu.m10/mu.m00 + r.x, mu.m01/mu.m00 +r.y)<<" ";
    //cout << Point2f(mu2.m10/mu2.m00 + r.x, mu2.m01/mu2.m00 +r.y)<<endl;
    centers.push_back(Point2f( mu.m10/mu.m00 + r.x, mu.m01/mu.m00 +r.y));
  }
}

vector<Point2f> Engine::getPointsForCal(){
  /*vector<Point2f> tmp;
  tmp.resize(orden.size());
  for(int i= 0; i<orden.size();i++){
    int idx = orden[i];
    auto r = rect_contours[i];
    tmp[idx] = Point2f(r.x,r.y);
  }
  return tmp;
  */
  //cout << "ooooooooooooooooooooooooooooooooooooooooooooooo"<<endl;
  // we need the center of ellipse, but now we have the rect that contains the ellipse

  computeCenters();
  vector<Point2f> tmp;
  tmp.resize(orden.size());
  for(int i= 0; i<orden.size();i++){
    int idx = orden[i];
    tmp[idx] = centers[i];
  }

  //cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
  /*for(auto& a : tmp)
    cout << a << endl;
  cout << "oooooooooooooooooooooooooooooooooooooooooooooooo"<<endl;
  */

  //reverse
  reverse(tmp.begin(),tmp.end());

  return tmp;
}

template<typename T>
void Engine::print(T actual){
  cout << actual << endl;
}

template<typename T, typename... Args>
void Engine::print(T actual, Args... args){
  cout << actual << " ";
  print(args...);
}

void Engine::globalPositionRect(){
  auto f = [this](Rect& r){r.x += rect_roi.x; r.y += rect_roi.y;};
  for_each(rect_contours.begin(), rect_contours.end(), f);
}

void Engine::findRings(){
  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  findContours(binary_roi, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
  //cout << "contours found: " << contours.size() << endl;

  //----------------------filter contours---------------------------
  contours_f.clear();
  rect_contours.clear();
  vector<int> c_areas;
  vector<float> c_rate;
  vector<int> aux;
  //cout << min_area << " " << max_area << " " << r_min << " " << r_max << endl;
  //print(min_area, max_area, r_min, r_max);
  for (int i=0; i < contours.size(); i++){
    auto cnt = contours[i];
    Rect rect = boundingRect(cnt);
    float r = float(rect.width)/float(rect.height);

    //cout << i<<" "<<rect.area() << " "<< r << " " << hierarchy[i][2] << endl;

    // filter contours children
    if (hierarchy[i][2] == -1) continue;

    if (rect.area() < max_area*1.2 && rect.area() > min_area*0.8
        && r<r_max*1.21 && r>r_min*0.79){
      contours_f.push_back(cnt);     // contornos filtrados
      rect_contours.push_back(rect); // respectivo rectangulo de los contornos filtrados
      c_areas.push_back(rect.area());
      c_rate.push_back(r);
      aux.push_back(i);
    }
  }
  //cout << "filtered contours are :" << endl;
  //for (auto n : aux) cout << n << " ";
  //cout << endl;
  //cout << "# filtering contours: "<<c_areas.size()<<endl;

  //---------------------- update areas and ratio---------------------------
  // if first time no previus match
  if (flag_roi){
    if (c_areas.size()>10){
      sort(c_areas.begin(),c_areas.end());
      min_area = c_areas[c_areas.size()/2];
      max_area = min_area;
      flag_roi = false;
    }
    return;
  }

  // check if almenos se ideintificaron 5 circulos
  if (c_areas.size()>5){
    auto ptr_a = minmax_element(c_areas.begin(),c_areas.end());
    auto ptr_r = minmax_element(c_rate.begin(),c_rate.end());
    min_area = *ptr_a.first;
    max_area = *ptr_a.second;
    r_min = *ptr_r.first;
    r_max = *ptr_r.second;
    //idx_min = distance(c_areas.begin(), ptr_a.first);
    //idx_max = distance(c_areas.begin(), ptr_a.second);
  }else{
    //idx_max = 0;
    //idx_min = 0;
  }
}

bool Engine::processing(Mat& frame_){
  //set image rgb and gray
  frame = frame_;
  cvtColor(frame, gray, CV_BGR2GRAY);

  // select roi
  // check for roi if is the first time. so roi is all frame
  // bot now the roi is select in constructor, TODO: automatically
  if (flag_roi) {rect_roi = Rect(0,0, gray.cols-1, gray.rows-1);}

  gray(rect_roi).copyTo(gray_roi);
  frame(rect_roi).copyTo(frame_roi);

  // apply threshold
  double new_t = threshold(gray_roi, binary_roi, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);


  findRings();

  // if number of rings == 0, dont update roi, transformation, globalPosition
  if (contours_f.size()<8) {
    // bueno talvez el otzu funciono mal entonces usemos el ultimo threshold bueno
    threshold(gray_roi, binary_roi, last_threshold, 255, THRESH_BINARY_INV);
    findRings();
    if (contours_f.size()<8) {
      flag_roi = true;
      min_area = 100;
      max_area = 7000;
      r_max = 1.1;
      r_min = 0.9;
      angle = 0;
      return false;}

  }
  else{
    last_threshold = new_t;
  }


  // if hay mas de 20 rings deberiamos hacer transformacion, ni update angle
  transformation(false);
  updateAngle();
  globalPositionRect();
  //computeCenters();
  updateROI();

  //cout << "Threshold: "<<new_t<<endl;
  return true;
}


void Engine::showImages(vector<string> names){
  imshow(names[0], frame);
  imshow(names[1], binary_roi);

  // maybe, no found rings so img_T is empty
  if (img_T.empty()) return;
  Mat element2 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(7, 7));
  dilate(img_T,img_T,element2);
  dilate(img_T,img_T,element2);
  imshow(names[2], img_T);
}


void Engine::drawResults(int n_frame, double time){
  auto f = [this](const Rect& r){
    rectangle(frame, r, Scalar(0,255,0), 2);
  };
  // draw rect_contours
  for_each(rect_contours.begin(), rect_contours.end(), f);

  // draw roi
  rectangle(frame, rect_roi, Scalar(255,255,0), 2);
  // show what frame is
  putText(frame, "frame: "+to_string(n_frame), Point(30,30),
      FONT_HERSHEY_COMPLEX_SMALL, 2.0, cvScalar(0,0,255), 2, false);
  // show processing time
  putText(frame, to_string(int(time))+" ms", Point(30,70),
      FONT_HERSHEY_COMPLEX_SMALL, 2.0, cvScalar(0,0,255), 2, false);
  // show angle of transformation
  putText(frame, to_string(angle), Point(30,110),
      FONT_HERSHEY_COMPLEX_SMALL, 2.0, cvScalar(0,0,255), 2, false);
  // show the rings' orden
  for(int i=0; i<rect_contours.size(); i++){
    Point p(rect_contours[i].x, rect_contours[i].y);
    putText(frame, to_string(orden[i]), p,
            FONT_HERSHEY_COMPLEX_SMALL, 2.0, cvScalar(255,0,0), 2,false);
  }

}

// this return width and height like point structure
Point Engine::computeMaxRect(){
  auto t_w = max_element(rect_contours.begin(), rect_contours.end(),
                      [](const auto& r1,const auto& r2){return r1.width<r2.width;});
  auto t_h = max_element(rect_contours.begin(), rect_contours.end(),
                      [](const auto& r1,const auto& r2){
                      return r1.height<r2.height;});

  return Point(t_w->width, t_h->height);
}

void Engine::updateROI(){
  if (contours_f.empty()){
    cout << "reset ROI by default"<<endl;
    // TODO
    return;
  }
  // find the windows for ROI (closure all points) respect gray_roi
  auto ptr_x = minmax_element(rect_contours.begin(),rect_contours.end(),
                              [](auto p1, auto p2){return p1.x<p2.x;});
  auto ptr_y = minmax_element(rect_contours.begin(),rect_contours.end(),
                              [](auto p1, auto p2){return p1.y<p2.y;});
  Point roi_p1 (ptr_x.first->x, ptr_y.first->y);
  Point roi_p2 (ptr_x.second->x, ptr_y.second->y);

  roi_p2 += computeMaxRect(); // add size of rect
  // add padding
  int pad = int(0.1*max(abs(roi_p1.x-roi_p2.x), abs(roi_p1.y-roi_p2.y)));
  roi_p1 -= Point(pad, pad); //+ Point(rect_roi.x, rect_roi.y);
  roi_p2 += Point(pad, pad); //+ Point(rect_roi.x, rect_roi.y);
  // proof boundaries respect global position (src_size)
  if (roi_p1.x < 0) roi_p1.x = 0;
  if (roi_p1.y < 0) roi_p1.y = 0;
  if (roi_p2.x > gray.cols-1) roi_p2.x = gray.cols-1;
  if (roi_p2.y > gray.rows-1) roi_p2.y = gray.rows-1;
  // from two point we build the rect for roi
  int w = roi_p2.x-roi_p1.x;
  int h = roi_p2.y-roi_p1.y;
  rect_roi = Rect(roi_p1.x, roi_p1.y, w, h);
}


void Engine::updateAngle(){
  if (orden.size()<2){
    cout << "No hay dos match para calcular el nuevo angulo"<<endl;
    return;
  }

  double a_angle = angle;
  int a_s = status_a;
  // find ring 0 and ring 1
  auto ptr = find(orden.begin(),orden.end(),0);
  if (ptr == orden.end()){
    cout << "ERROR no find ring 0 in upodateAngle"<<endl;
    return;
  }
  int idx0 = distance(orden.begin(),ptr);
  ptr = find(orden.begin(),orden.end(),1);
  if (ptr == orden.end()){
    cout << "ERROR no find ring 1 in upodateAngle"<<endl;
    return;
  }
  int idx1 = distance(orden.begin(),ptr);

  Point p0(rect_contours[idx0].x, rect_contours[idx0].y);
  Point p1(rect_contours[idx1].x, rect_contours[idx1].y);
  int delta_x = p0.x-p1.x;
  int delta_y = p0.y-p1.y;
  //print(p0,p1);
  angle = atan2(delta_y, delta_x)*180/pi;

  //cout <<"--------------------angle----------------------"<<endl;
  //print(a_angle, angle, status_a);

  if (angle >= 90 && a_angle > 0) status_a = 1;
  if (status_a == 1 && angle < 0) angle = 360+angle;
  if (status_a == 1 && angle < 90) status_a = 0;

  if (angle <= -90 && a_angle < 0) status_a = -1;
  if (status_a == -1 && angle > 0) angle = -360+angle;
  if (status_a == -1 && angle > -90) status_a = 0;

  //print(a_angle, angle, status_a);
  // check angle overflow
  if ( abs(a_angle-angle)>10) {
    //cout << abs(a_angle-angle) << endl;
    //cout << "aaa"<<endl;
    angle =a_angle, status_a = a_s;
  }else{
    // como no hay overflow, para mejorar la tranformacion
    // volavamos a llamarla con el angulo correcto
    // TODO: deberiamos almacenar el row_row anterior
    transformation(true);
  }
}


// trabajamos en gray_roi, cada rect_contours es transformado
// usando un angulo(angle) y una distancia entre fila y fila(row_row)
void Engine::transformation(bool u_r_r){
  double cos_ = cos(-angle*pi/180);
  double sin_ = sin(-angle*pi/180);
  //definimos el centro normal y el de la transformacion
  int h = gray_roi.rows;
  int w = gray_roi.cols;
  int cx = w/2;
  int cy = h/2;
  int nW = w*abs(cos_)+ h*abs(sin_);
  int nH = w*abs(sin_)+ h*abs(cos_);
  int CX = nW/2;
  int CY = nH/2;
  // creamos la imagen para el espacio T
  img_T = Mat(nH, nW, CV_8UC1, Scalar(0));
  // tranformamos unicamente los points encotrados
  vector<Point> points_T;
  for(auto& r : rect_contours){
    int x_ = r.x-cx;
    int y_ = r.y-cy;
    double X_ = x_*cos_ - y_*sin_;
    double Y_ = x_*sin_ + y_*cos_;
    int X = int(X_)+CX;
    int Y = int(Y_)+CY;
    // ya tenemos la posicion en T, now fill
    img_T.at<unsigned char>(Y,X) = 255;
    //almacenamos la posicion en T para saber cual le corresponde del espacio original
    points_T.push_back(Point(X,Y));
  }
  // ya tenemos la imagen en T con los points transformados
  vector<cnt_t> contours;
  vector<Vec4i> hierarchy;
  findContours(img_T, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
  vector<Point> points_T_orden;
  for(auto& cnt : contours){
    Rect r = boundingRect(cnt);
    Point p(r.x+r.width/2, r.y+r.height/2);
    points_T_orden.push_back(p);
  }
  assert(points_T.size() == points_T_orden.size());
  //aun puede haber un desorden, ordenamos por el axis x
  int i = 0;
  vector<int> ys; // contiene las alturas de cada fila (pi)
  //cout << "row_row"<<row_row<<endl;

  while (i < points_T.size()) {
    Point pi = points_T_orden[i];
    int k = i; // ayuda a recordar cual es la pos de nuestro pi
    i++;
    int acc = 0;
    while (i < points_T.size() && acc < 4) {
      Point pn = points_T_orden[i];
      // comprobar si esta en la misma linea
      if (double(abs(pi.y-pn.y)) > 0.7*row_row)
        break;

      i++, acc++;
    }
    ys.push_back(pi.y);

    //for (auto& p : ys) cout << p << " ";
    //cout <<endl;

    // ordenamos segun el axis x, para los que pertenecen a la misma fila
    auto ptr = points_T_orden.begin()+k;

    /*
    cout << *ptr << " " <<acc<<endl;
    for (auto& p : points_T_orden)
      cout << p << " ";
    cout <<endl;
    */

    sort(ptr, ptr+acc+1, [](const auto& p1, const auto& p2){
      return p1.x > p2.x;
    });
  }
  // calcular el nuevo row_row
  if (u_r_r && ys.size()>1)
    row_row = double(abs(ys[0]-ys[1]));
  // ahora calculamos el orden real

  orden.clear();
  for (auto& p : points_T){
    auto ptr = find(points_T_orden.begin(),points_T_orden.end(), p);
    // check que el point este, sino error
    assert(ptr != points_T_orden.end());
    int idx = distance(points_T_orden.begin(), ptr);
    orden.push_back(idx);
  }
}




