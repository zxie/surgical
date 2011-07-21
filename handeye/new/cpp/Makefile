LIBS = -L/usr/lib -lopencv_legacy -lopencv_core
INCLUDES = -I/usr/local/include -I/usr/local/include/opencv -I/usr/local/include/eigen2

target: optimize_handeye

optimize_handeye: optimize_handeye.cpp
	g++ $(INCLUDES) -o optimize_handeye optimize_handeye.cpp $(LIBS)

calibrate_camera: calibrate_camera.cpp
	g++ $(INCLUDES) -o calibrate_camera calibrate_camera.cpp $(LIBS)
