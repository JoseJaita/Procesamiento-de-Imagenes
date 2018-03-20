#include "calibration.h"
#include "refinement.h"

int main(int argc, char *argv[]){
    if (argc < 3 ){cout << "Faltan argumentos" <<endl;return -1;}

    //---------------------- argv ------------------------------------
    string arg = argv[1];
    int n_f = atoi(argv[2]);

    //------------------- variables for calibration ------------------
    const string file("others/calibration/circle/ps3/");
    const string name_xml("params_circle_ps3.xml");

    //const string file("others/calibration/circle/lifecam/");
    //const string name_xml("params_circle_lifecam.xml");

    Calibration calibration;
    Size pattern_size(4,11);
    const double length = 35.;

    namedWindow("image", WINDOW_AUTOSIZE);

    VideoCapture video("others/videos_final/ps3_circles.webm");
    //VideoCapture video("others/videos_final/LifeCam_asymmetric_circles.avi");

    // if is -v mean that ist necessary get images
    if (arg == "-v"){
        Mat frame, frame_cal, gray;
        int i_cal = 0;
        for(int i = 0;i<4500;i++) {video >> frame;imshow("image",frame);waitKey(1);}
        while (1) {
            video >> frame;
            if (frame.empty()){cout << "finish video"<<endl;return-1;}
            frame.copyTo(frame_cal);
            cvtColor(frame, gray, COLOR_BGR2GRAY);
            vector<Point2f> points2D;
            bool found = findCirclesGrid(gray, pattern_size, points2D, CALIB_CB_ASYMMETRIC_GRID);

            drawChessboardCorners(frame, pattern_size,Mat(points2D),found);

            imshow("image", frame);

            char c = (char)waitKey(0);
            // exit program
            if (c == 27) {destroyAllWindows();return 0;}
            if (c == 's'){
                // check if is a good frame
                if (found){
                    imwrite(file+to_string(i_cal)+".jpg", frame_cal, {CV_IMWRITE_JPEG_QUALITY,100});
                    i_cal++;
                    cout << "Image: "<<i_cal<<endl;
                }
            }
            //check number of images, yet saved
            if (i_cal == n_f) break;
        }
    }


    Mat image, gray;
    if (arg == "-v" || arg == "-i"){
    for (int i=0; i<n_f; i++){
        string dir = file+to_string(i)+".jpg";
        image = imread(dir);
        cvtColor(image, gray, COLOR_BGR2GRAY);
        vector<Point2f> points2D;
        bool found = findCirclesGrid(gray, pattern_size, points2D, CALIB_CB_ASYMMETRIC_GRID);

        // found all corner
        if (found){
          //cornerSubPix(gray, points2D, Size(11, 11), Size(-1, -1),TermCriteria(TermCriteria::EPS +TermCriteria::COUNT, 30, 0.1 ));
          drawChessboardCorners(image, pattern_size,Mat(points2D),found);

          if (i==0){
            calibration.setImageSize(image.size());
            calibration.setPatternSize(pattern_size, length, true);
          }
          calibration.addImagePoints(points2D);
        }
        imshow("image", image);
        waitKey(1500);
    }

    // yet have the point2D, now to calibration
    cout << "start to Calibration ..." << endl;
    bool result = calibration.processing();
    cout << "end Calibration"<<endl << endl;
    if (result){
      calibration.showResults();
      calibration.saveParams(file+name_xml);
    } else cout << "fail to Calibration"<<endl;
    }

    // read from xml
    bool r = calibration.readXml(file+name_xml);

    if (r && false) {
      calibration.showResults();
      auto camera_matrix = calibration.getCameraMatrix();
      auto dist_coeffs = calibration.getDistCoeffs();
      Mat image_undistorted;
      while (1) {
        video >> image;
        undistort(image, image_undistorted, camera_matrix, dist_coeffs);
        imshow("image", image_undistorted);
        char c =(char)waitKey(0);
        if (c == 27) break;
      }

    }
    else{
      destroyAllWindows();
      Calibration calibration1;
      Refinement refine(calibration,file, CIRCLE);
      string s = refine.processing();
      calibration1.readXml(s);
      calibration.readXml(file+name_xml);

      // tenemos una nueva calibration, a probar
      auto camera_matrix = calibration.getCameraMatrix();
      auto dist_coeffs = calibration.getDistCoeffs();
      auto camera_matrix1 = calibration1.getCameraMatrix();
      auto dist_coeffs1 = calibration1.getDistCoeffs();

      cout << camera_matrix << endl;
      cout << camera_matrix1 << endl;

      namedWindow("original",0);
      namedWindow("calibration 0",0);
      namedWindow("calibration 1",0);
      while (1) {
        Mat image, image_undistorted, image_undistorted1;
        video >> image;
        undistort(image, image_undistorted, camera_matrix, dist_coeffs);
        undistort(image, image_undistorted1, camera_matrix1, dist_coeffs1);

        // comparemos la colinearidad de las dos calibraciones
        cout << "frame: "<<endl;
        vector<Point2f> points2D,points2D1,points2D2;
        Mat g,g1,g2;
        cvtColor(image_undistorted,g,COLOR_BGR2GRAY);
        cvtColor(image_undistorted1,g1,COLOR_BGR2GRAY);
        cvtColor(image,g2,COLOR_BGR2GRAY);
        bool b = findCirclesGrid(g, pattern_size, points2D, CALIB_CB_ASYMMETRIC_GRID);

        bool b1 = findCirclesGrid(g1, pattern_size, points2D1, CALIB_CB_ASYMMETRIC_GRID);
        bool b2 = findCirclesGrid(g2, pattern_size, points2D2, CALIB_CB_ASYMMETRIC_GRID);

        if (b2){
          cout << "error sin calibrar: ";
          calibration.drawCentersAndRect(image,points2D2);}

        if (b) {
          cout << "error con una calibracion: ";
          calibration.drawCentersAndRect(image_undistorted,points2D);}

        if (b1){
          cout << "error con refinamiento: ";
          calibration.drawCentersAndRect(image_undistorted1,points2D1);}



        imshow("original",image);
        imshow("calibration 0", image_undistorted);
        imshow("calibration 1", image_undistorted1);
        char c =(char)waitKey(0);
        if (c == 27) break;
      }
    }

    video.release();
    destroyAllWindows();
    return 0;
}
