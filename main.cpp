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

    cv::Mat image = cv::imread(filepath);
    if(image.empty()){
        printf("Cannot read the input image.\n");
        exit(-1);
    }

    MarkerLocations marker_locas = locateEightPointMarkers(image);

    printf("Detect done: \n\t%ld markers found.\n", marker_locas.size());

    // Show results
    cv::RNG rng;
    for(size_t i = 0; i < marker_locas.size(); i++){
        printf("\t#%ld marker:\n", i);
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        cv::Scalar color = cv::Scalar(r, g, b);
        for(size_t j = 0; j < marker_locas[i].size(); j++){
            cv::Point2f pt = marker_locas[i][j];
            cv::circle(image, pt, 3, color, 3);
            printf("\t\t Pt%ld:[%f,%f]\n", j, pt.x, pt.y);
        }
    }
    cv::imshow("Results", image);
    cv::waitKey(0);

    return 0;
}
