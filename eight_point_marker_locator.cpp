#include "eight_point_marker_locator.h"
#include <cstdint>

namespace  {
constexpr uint8_t  MARKER_BG = 20;
constexpr uint16_t MARKER_AREA_MIN = 1000;
constexpr uint16_t MARKER_PT_AREA_MIN = 100;
constexpr uint16_t MARKER_PT_AREA_MAX = 500;

bool isMarker(const cv::Mat& roi_bw, MarkerPointLocations& mpt_locations);
}


MarkerLocations locateEightPointMarkers(const cv::Mat& image)
{
    MarkerLocations marker_locations;

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::Mat bw = gray < ::MARKER_BG;

    cv::Mat labels, stats, centroids;
    int num = cv::connectedComponentsWithStats(bw, labels, stats, centroids);
    for(uint16_t i = 1; i < num; i++){
        int area = stats.at<int>(i, 4);
        if(area < MARKER_AREA_MIN) continue;

        cv::Mat roi_bw = labels == i;
        MarkerPointLocations mpt_locations;
        if(::isMarker(roi_bw, mpt_locations)){
            marker_locations.push_back(mpt_locations);
        }
    }

    return marker_locations;
}

/*****************************************************************************/
namespace {

int findMaxValue(const std::vector<int>& values)
{
    int max_val_idx = 0;
    int max_val = 0;
    for(uint16_t i = 0; i < values.size(); i++){
        if(values[i] > max_val) {
            max_val = values[i];
            max_val_idx = i;
        }
    }
    return max_val_idx;
}


std::vector<int> filterFlags(std::vector<int>& flags, float mean_area, int count)
{
    for(uint16_t i = 0; i < flags.size(); i++){
        if(!flags[i]) continue;
        flags[i] = abs(flags[i] - mean_area) + 1;
    }

    for(uint16_t i = 0; i < count - 8; i++){
        flags[findMaxValue(flags)] = 0;
    }

    return flags;
}


bool isMarker(const cv::Mat& roi_bw, MarkerPointLocations& mpt_locations)
{
    mpt_locations.clear();
    mpt_locations.reserve(8);

    cv::Mat labels, stats, centroids;
    int num = cv::connectedComponentsWithStats(~roi_bw, labels, stats, centroids);
    int count = 0;
    size_t sum = 0;
    std::vector<int> flags(num-1);
    for(uint16_t i = 1; i < num; i++){
        int area = stats.at<int>(i, 4);
        if(area > ::MARKER_PT_AREA_MIN && area < ::MARKER_PT_AREA_MAX){
            count++;
            flags[i-1] = area;
            sum += area;
        }
        else{
            flags[i-1] = 0;
        }
    }
    if (count < 8){
        return false;
    }

    if (count > 8){
        flags = filterFlags(flags, 1.0 * sum / count, count);
    }

    int xc = 0, yc = 0;
    for(uint16_t i = 0; i < flags.size(); i++){
        if(flags[i]){
            xc = centroids.at<double>(i+1, 0);
            yc = centroids.at<double>(i+1, 1);
            mpt_locations.push_back(cv::Point2f(xc, yc));
        }
    }

    return true;
}

} // namespace
