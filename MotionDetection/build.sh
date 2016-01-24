LIBS="-lopencv_imgproc -lopencv_videoio -lopencv_video -lopencv_imgcodecs -lopencv_highgui -lopencv_core"
echo "Quick build script for OpenCV project..."
echo "Compiling source Motion_detection_safe.cpp..."
g++ -c -I/usr/local/include/opencv2 Motion_detection_safe.cpp
echo "Compiled successfully!"
echo "Linking object and OpenCV libs..."
g++ -L/usr/local/lib -o "Motion_detection" Motion_detection_safe.o $LIBS
