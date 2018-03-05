os videos de prueba se deben descargar y poner en la carpeta "others/videos_final"



main.cpp -> contiene el codigo fuente para deteccion y calibracion de anillos

engine.cpp -> contiene todo el procesamiento para la deteccion de los anillos

calibration.cpp -> contiene un clase para la calibracion, es independiente del padron que se use

chessboard.cpp-> contiene la calibracion de chessboard

circle.cpp -> contiene la calibracion de los circulos

/----------------------------------------------------------------------------


Para calibrar usando circulos simetricos

g++ -std=c++14 circle.cpp calibration.cpp -o circle `pkg-config --libs opencv` -fopenmp

si es la primera vez entonces (-v) dice que primero guardaremos images (20), el video va a estar en stop
entonces apretar "n" para el siguiente frame y cuando se quiera usar cierto frame para la calibracion
presionar "s" 
./circle -v 20

si ya se elegio antes imagenes para la calibracion entonces:
./circle -i 20

Para chessboard es lo mismo
g++ -std=c++14 chessboard.cpp calibration.cpp -o chessboard `pkg-config --libs opencv` -fopenmp

./chessboard -v 20

./chessboard -i 20


Para Anillos

g++-7 -std=c++14 main.cpp engine.cpp calibration.cpp -o main `pkg-config --libs opencv` -fopenmp 

para ejecutar :

./main 

con "n" se pasa al siguiente frame, con "s" se guarda la imagen para la calibracion
y con "c" se pasa a la calibracion.





