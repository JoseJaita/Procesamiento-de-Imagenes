#include "engine.h"
#include "calibration.h"

int main(int argc, char *argv[]){
  //-------------- get video--------------------------------------
  //VideoCapture video(0);
  //VideoCapture video("others/videos/calibration_mslifecam.avi");
  //VideoCapture video("others/videos/ellipses.avi");
  //VideoCapture video("others/videos/Kinect2_rgb.avi");
  //VideoCapture video("others/videos_final/PS3_rings.avi");
  VideoCapture video("others/videos_final/LifeCam_rings.avi");
  if (!video.isOpened()){
    cout << "can't read video"<<endl;//
    return -1;
  }

  //--------------- Variables for engine ----------------------------
  namedWindow("frame", 0);
  namedWindow("binary", 0);
  namedWindow("T", 0);
  Mat frame;
  Engine engine;
  int n_frame = 0;

  //--------------- Variables for Calibration-------------------------
  Calibration calibration;
  const string file_cal("others/calibration/rings/lifecam/");
  //const string file_cal("others/calibration/rings/ps3/");
  const string name_xml("params.xml");
  const Size pattern_size(5,4);
  int i_cal = 0;
  Mat frame_cal;

  //-------------------------main loop--------------------------------

  while (1) {

    video >> frame;
    if (frame.empty()){cout <<"finish video"<<endl; break;}
    frame.copyTo(frame_cal);

    // current frame
    cout << "-------------------------------------" << endl;
    cout << "Frame: "<< n_frame << endl;
    auto start = chrono::high_resolution_clock::now();
    //auto start = chrono::high_resolution_clock::now();
    engine.processing(frame);

    // processing time
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> delay = finish-start;

    // draw match roi n_frame and time
    engine.drawResults(n_frame++, delay.count()*1000);

    cout << "time is: "<<delay.count()*1000 << "\n";

    // show results for engine
    engine.showImages({"frame","binary","T"});
    cout << "aaaaa"<<endl;

    char c = (char)waitKey(0);  // CAMBIAR A  1 PARA QUE CORRAR SIN PARAR
    // exit program
    if (c == 27){ destroyAllWindows(); return 0;}

    // save image for calibration
    if (c == 's'){
      // if number of ring is less that pattern size, discard current frame
      if (engine.numRingsDetected() == pattern_size.height*pattern_size.width){
        imwrite(file_cal+to_string(i_cal)+".jpg",frame_cal,{CV_IMWRITE_JPEG_QUALITY,100});
        if (i_cal == 0){
          calibration.setImageSize(frame_cal.size());
          calibration.setPatternSize(pattern_size);
        }
        calibration.addImagePoints(engine.getPointsForCal());
        i_cal++;
        cout << "Number of images for Calibration: " << i_cal << endl;
      }
      else cout << "this frame isn't good for calibration"<<endl;
    }

    // to calibration
    if (c == 'c'){
      cout << endl <<"start Calibration..."<<endl;
      // yet have the point2D, now to calibration
      bool result = calibration.processing();
      cout << "end Calibration"<<endl << endl;
      if (result) {
        calibration.showResults();
        calibration.saveParams(file_cal+name_xml);
      }
      else cout << "Fail Calibration" << endl;
      break;
    }
    //yet have the xml saves, so don't need calibration
    if (c == 'p') break;
  }

  // maybe we want use the params computed
  // read xml
  bool r = calibration.readXml(file_cal+name_xml);
  // if read correctly the params
  if (1){
    Mat frame_undistorted;
    Mat camera_matrix = calibration.getCameraMatrix();
    Mat dist_coeffs = calibration.getCameraMatrix();

    //Mat camera_matrix = (Mat_<double>(3, 3) << 703.2185641887843, 0, 320,
    //                      0, 703.2185641887843, 240,
    //                      0, 0, 1);
    //Mat dist_coeffs = (Mat_<double>(1, 5) << -0.4197524126377384,
    //                   0.5789603642873149,
    //                   0,
    //                   0,
    //                   -0.8661831870389252);

    /*
    Mat camera_matrix = (Mat_<double>(3,3) << 626.3507056720025, 0, 320,
     0, 626.3507056720025, 240,0, 0, 1);
    Mat dist_coeffs = (Mat_<double>(1,5) << 0.2340347552292534, -4.64363932541269,
        0,
        0,
     29.20878137254296);*/

    while (1) {
      video >> frame;
      undistort(frame, frame_undistorted, camera_matrix, dist_coeffs);
      imshow("image", frame_undistorted);
      char c =(char)cvWaitKey(0);
      if (c == 27) break;
    }
  }
  return 0;
}

