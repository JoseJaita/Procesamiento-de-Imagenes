#include "calibration.h"

int main(int argc, char *argv[]){
    if (argc < 3 ){cout << "Faltan argumentos" <<endl;return -1;}

    //---------------------- argv ------------------------------------
    string arg = argv[1];
    int n_f = atoi(argv[2]);

    //------------------- variables for calibration ------------------
    //const string file("others/calibration/circle/ps3/");
    const string file("others/calibration/circle/lifecam/");
    Calibration calibration;
    Size pattern_size(4,11);

    namedWindow("image", WINDOW_AUTOSIZE);
    //VideoCapture video("others/videos_final/PS3_asymmetric_circles.avi");
    VideoCapture video("others/videos_final/LifeCam_asymmetric_circles.avi");
    // if is -v mean that ist necessary get images
    if (arg == "-v"){
        Mat frame, frame_cal, gray;
        int i_cal = 0;
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
            calibration.setPatternSize(pattern_size, true);
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
    if (result) {
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
    else cout << "Fail Calibration" << endl;

    video.release();
    destroyAllWindows();
    return 0;
}
