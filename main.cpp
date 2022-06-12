#include "eight_point_marker_locator.h"
#include <string>


int main(int argc, char* argv[])
{
    std::string filepath;
    if(argc >= 2){
        filepath = argv[1];
    }
    else{
        printf("No image path was given. Program stop.\n");
        return -1;
    }

    epm::EightPointMarkerLocator locator;
    epm::MarkerLocations marker_locas;

    std::string fmt = filepath.substr(filepath.size()-3);
    // Process image
    if(fmt == "jpg" || fmt == "png" || fmt == "bmp") {
        cv::Mat image = cv::imread(filepath);
        if(image.empty()){
            printf("Cannot read the input image.\n");
            exit(-1);
        }
        marker_locas = locator.locateMarkers(image);
        printf("Detect done: \n\t%ld markers found.\n", marker_locas.size());
        locator.showResults(0);
    }
    else if(fmt == "avi" || fmt == "mp4"){
        cv::VideoCapture capture(filepath);
        if(!capture.isOpened()){
            printf("Cannot open the input video.\n");
            exit(-1);
        }
        size_t count = 0;
        cv::Mat frame;
        while(capture.read(frame)){
            ++count;
            if(count < 80) continue;
            printf("%04ld\n", count);

            marker_locas = locator.locateMarkers(frame);
            printf("%04ld Detect done: \n\t%ld markers found.\n",
                   count, marker_locas.size());
            locator.showResults(10);
        }
    }
    else{
        printf("Unsopported file.\n");
    }

    return 0;
}

