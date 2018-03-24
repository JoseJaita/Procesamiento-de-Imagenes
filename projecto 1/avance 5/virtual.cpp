#include "calibration.h"
#include "engine.h"

void drawCube(Mat& image, vector<Point2f> points);

//const string file_cal = "others/calibration/rings/ps3/new_params.xml";
//const string file_video = "others/videos_final/ellipses.avi";

int main(int argc, char *argv[]){
  if(argc < 3){
    cout << "indique la ruta del video y el xml donde se encuentra"
            " los parametros de la calibracion previamente hecha "
            " por ejemplo: ./virtual others/videos_final/ellipses.avi others/calibration/rings/ps3/new_params.xml"<<endl;
    return 0;
  }
  string file_video = argv[1];
  string file_cal = argv[2];

  Calibration calibration;
  calibration.readXml(file_cal);
  Size pattern_size = calibration.getPatternSize();
  Mat A = calibration.getCameraMatrix();
  Mat dist_coeffs = calibration.getDistCoeffs();
  double lenght = calibration.getLength();
  // points 3D for cube
  vector<Point3f> points3D_cube;
  points3D_cube.push_back(Point3f(lenght,lenght,0));
  points3D_cube.push_back(Point3f(lenght,3*lenght,0));
  points3D_cube.push_back(Point3f(3*lenght,3*lenght,0));
  points3D_cube.push_back(Point3f(3*lenght,lenght,0));

  points3D_cube.push_back(Point3f(lenght,lenght,-2*lenght));
  points3D_cube.push_back(Point3f(lenght,3*lenght,-2*lenght));
  points3D_cube.push_back(Point3f(3*lenght,3*lenght,-2*lenght));
  points3D_cube.push_back(Point3f(3*lenght,lenght,-2*lenght));

  Engine engine;

  VideoCapture video(file_video);
  namedWindow("original");
  while (1) {
    Mat frame;
    video >> frame;
    if (frame.empty()){cout << "finish video"<<endl;break;}

    engine.processing(frame);

    if (engine.numRingsDetected() == pattern_size.height*pattern_size.width){
      Mat rotate, translate;
      vector<Point2f> points2D = engine.getPointsForCal();

      solvePnP(calibration.getObjectPoint(), points2D, A,dist_coeffs,rotate,translate);

      vector<Point2f> points2D_cube;
      projectPoints(points3D_cube,rotate,translate,A,dist_coeffs,points2D_cube);

      drawCube(frame, points2D_cube);

    }


    imshow("original",frame);
    char c = (char)waitKey(1);
    if (c == 27) break;
  }

  destroyAllWindows();
  return 0;
}

void drawCube(Mat& image, vector<Point2f> points){
  for(int i=0;i<4;i++){
    line(image,points[i],points[(i+1)%4],Scalar(0,255,0),3);
    line(image,points[i],points[i+4],Scalar(255,0,0),3);
    line(image,points[i+4],points[(i+1)%4+4],Scalar(0,0,255),3);
  }

}


