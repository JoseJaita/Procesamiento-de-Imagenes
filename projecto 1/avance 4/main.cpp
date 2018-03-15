#include "engine.h"
#include "calibration.h"
#include "refinement.h"

int main(int argc, char *argv[]){

  //-------------- get video--------------------------------------
  //VideoCapture video(0);
  //VideoCapture video("others/videos/calibration_mslifecam.avi");
  //VideoCapture video("others/videos/ellipses.avi");
  //VideoCapture video("others/videos/Kinect2_rgb.avi");
  //VideoCapture video("others/videos_final/ellipses.avi");
  VideoCapture video("others/videos_final/PS3_rings.avi");
  //VideoCapture video("others/videos_final/LifeCam_rings.avi");
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
  //const string file_cal("others/calibration/rings/lifecam/");
  const string file_cal("others/calibration/rings/ps3/");
  const string name_xml("params_ring_ps3.xml");
  //const string name_xml("params_ring_life.xml");
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

    char c = (char)waitKey(0);  // CAMBIAR A  1 PARA QUE CORRA SIN PARAR
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
        calibration.addThreshold(engine.getThreshold());
        calibration.addSizeRoi(engine.getSizeRoi());
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

  destroyAllWindows();

  // maybe we want use the params computed
  // read xml
  bool r = calibration.readXml(file_cal+name_xml);
  // if read correctly the params
  if (r && false){
    Mat frame_undistorted;
    Mat camera_matrix = calibration.getCameraMatrix();
    Mat dist_coeffs = calibration.getDistCoeffs();

    namedWindow("original",0);
    namedWindow("undistorted",0);

    while (1) {
      video >> frame;
      undistort(frame, frame_undistorted, camera_matrix, dist_coeffs);

      engine.processing(frame);
      engine.drawCentersAndRect(frame);

      engine.processing(frame_undistorted);
      engine.drawCentersAndRect(frame_undistorted);

      imshow("original", frame);
      imshow("undistorted", frame_undistorted);
      char c =(char)cvWaitKey(0);
      if (c == 27) break;
    }
    destroyAllWindows();
  }

  // refinement
  else{
    Calibration calibration1;
    Refinement refine(calibration,file_cal, RING);
    string s = refine.processing();
    calibration1.readXml(s);
    calibration.readXml(file_cal+name_xml);

    // tenemos una nueva calibration, a probar
    auto camera_matrix = calibration.getCameraMatrix();
    auto dist_coeffs = calibration.getDistCoeffs();
    auto camera_matrix1 = calibration1.getCameraMatrix();
    auto dist_coeffs1 = calibration1.getDistCoeffs();

    cout << camera_matrix << endl;
    cout << camera_matrix1 << endl;

    namedWindow("original",0);
    namedWindow("calibration 0",0);
    namedWindow("test canonical 0");
    namedWindow("calibration 1",0);
    namedWindow("cambio centers");
    while (1) {
      Mat image, image_undistorted, image_undistorted1;
      Mat image_test_canonical;
      Mat cambio_centers;

      video >> image;
      if (image.empty()){cout << "finish video"<<endl;break;}
      undistort(image, image_undistorted, camera_matrix, dist_coeffs);
      undistort(image, image_undistorted1, camera_matrix1, dist_coeffs1);

      Mat map1, map2;
      //initUndistortRectifyMap(camera_matrix, dist_coeffs, Mat(),
        //                      getOptimalNewCameraMatrix(camera_matrix, dist_coeffs,calibration.getImageSize(), 1,
          //                                              calibration.getImageSize(), 0),
            //                                            calibration.getImageSize(), CV_16SC2, map1, map2);
      //remap(image, image_undistorted, map1, map2, INTER_LINEAR);
      //initUndistortRectifyMap(camera_matrix1, dist_coeffs1, Mat(),
        //                      getOptimalNewCameraMatrix(camera_matrix1, dist_coeffs1,calibration1.getImageSize(), 1,
          //                                              calibration1.getImageSize(), 0),
            //                                            calibration1.getImageSize(), CV_16SC2, map1, map2);
      //remap(image, image_undistorted1, map1, map2, INTER_LINEAR);



      // comparemos la colinearidad de las dos calibraciones
      cout << "frame : "<<endl;
      engine.processing(image_undistorted);
      engine.drawCentersAndRect(image_undistorted);

      engine.processing(image_undistorted1);
      engine.drawCentersAndRect(image_undistorted1);
      cout << "-------"<<endl;

      engine.processing(image);
      // test canonical
      bool flag_test =false;
      if (engine.numRingsDetected()==pattern_size.height*pattern_size.width)
        flag_test = refine.testCanonical(image, image_test_canonical,cambio_centers,engine,calibration1);

      imshow("original",image);
      imshow("calibration 0", image_undistorted);
      if (flag_test){
        imshow("test canonical 0", image_test_canonical);
        imshow("cambio centers",cambio_centers);
      }
      imshow("calibration 1", image_undistorted1);
      char c =(char)waitKey(0);
      if (c == 27) break;
    }

  }

  destroyAllWindows();
  return 0;
}





