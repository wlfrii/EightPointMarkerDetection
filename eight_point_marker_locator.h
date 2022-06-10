#ifndef EIGHT_POINT_MARKER_LOCATOR_H_LF
#define EIGHT_POINT_MARKER_LOCATOR_H_LF
#include <vector>
#include <opencv2/opencv.hpp>

using MarkerPointLocations = std::vector<cv::Point2f>;
using MarkerLocations = std::vector<MarkerPointLocations>;

MarkerLocations locateEightPointMarkers(const cv::Mat& image);

#endif // EIGHT_POINT_MARKER_LOCATOR_H_LF
