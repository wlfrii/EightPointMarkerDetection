#include "eight_point_marker_locator.h"
#include "eight_point_marker_util.h"
#include <cstdint>
#include <algorithm>

namespace epm{

constexpr uint8_t  MARKER_BG = 50;
constexpr uint16_t MARKER_AREA_MIN = 36*36;
constexpr uint16_t MARKER_PT_AREA_MIN = 100;
constexpr uint16_t MARKER_PT_AREA_MAX = 900;

bool isMarker(cv::Mat& bw, const cv::Mat& gray,
              MarkerPointLocations& pt_locations, MarkerPointAreas& pt_areas,
              const EightPointMarker* prev_marker);


EightPointMarkerLocator::EightPointMarkerLocator()
{
    _markers.clear();
}


EightPointMarkerLocator::~EightPointMarkerLocator()
{
}


MarkerLocations EightPointMarkerLocator::locateMarkers(const cv::Mat& image)
{
#if ENABLE_SHOW_RESULTS
    _image = image;
#endif
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 3);

    if(_markers.size() > 1){
#if DEBUG_MARKER_LOCATOR
        MTRACE("Track markers first\n");
#endif
        gray = trackMarkers(gray);
    }
#if DEBUG_MARKER_LOCATOR
        MTRACE("Find markers\n");
#endif
    findMarkers(gray);

    MarkerLocations marker_locations;
    for(auto& marker : _markers){
        marker_locations.push_back(marker.pts());
    }
    return marker_locations;
}


#if ENABLE_SHOW_RESULTS
void EightPointMarkerLocator::showResults(int wait_time)
{
    for(size_t i = 0; i < _markers.size(); i++){
        printf("\t#%ld marker:\n", i);
        auto& marker = _markers[i];
        cv::Scalar color = marker.color();
        for(size_t j = 0; j < marker.pts().size(); j++){
            cv::Point2f pt = marker.pts()[j];
            cv::circle(_image, pt, 3, color, 3);
            printf("\t\t Pt%ld:[%f,%f]\n", j, pt.x, pt.y);
        }
    }
    cv::imshow("Results", _image);
    cv::waitKey(wait_time);
}
#endif


void EightPointMarkerLocator::findMarkers(const cv::Mat& gray)
{
    cv::Mat BW = gray < MARKER_BG;
#if DEBUG_MARKER_LOCATOR
    show("findMarkers:BW", BW);
#endif

    cv::Mat labels, stats, centroids;
    int num = cv::connectedComponentsWithStats(BW, labels, stats, centroids);
    for(uint16_t i = 1; i < num; i++){
        int area = stats.at<int>(i, 4);
        if(area < MARKER_AREA_MIN) {
            continue;
        }
        cv::Mat bw = (labels == i);

        MarkerPointLocations pt_locations;
        MarkerPointAreas pt_areas;
        if(isMarker(bw, gray, pt_locations, pt_areas, nullptr)){
            EightPointMarker marker(pt_locations, pt_areas);
            // Refine the marker
            cv::Rect rect = marker.rect();
            cv::Mat roi_gray = gray(rect);
            cv::Mat roi_bw;
            cv::threshold(roi_gray, roi_bw, 100, 255, cv::THRESH_OTSU);
            roi_bw = ~roi_bw;
#if DEBUG_MARKER_LOCATOR
            MTRACE("Find a valid marker: pt.size:%ld, area.size:%ld\n",
                   pt_locations.size(), pt_areas.size());
            show("findMarker:isMarker", bw);
            show("findMarker:isMarker_refine", roi_bw); cv::waitKey(0);
#endif
            isMarker(roi_bw, roi_gray, pt_locations, pt_areas, nullptr);
            for(auto &pt:pt_locations){
                pt = cv::Point2f(pt.x + rect.x, pt.y + rect.y);
            }
            marker.update(pt_locations, pt_areas);
            _markers.push_back(marker);
        }
    }
}


cv::Mat EightPointMarkerLocator::trackMarkers(cv::Mat& gray)
{
    size_t i = 0;
    while(true){
        if(i >= _markers.size()) break;

        auto& marker = _markers[i];
        cv::Rect rect = marker.rect();
        cv::Mat roi_gray = gray(rect);
        cv::Mat roi_bw;
        cv::threshold(roi_gray, roi_bw, 100, 255, cv::THRESH_OTSU);
        roi_bw = ~roi_bw;
#if DEBUG_MARKER_LOCATOR
        show("trackMarker:roi_bw", roi_bw);
#endif
        MarkerPointLocations pt_locations;
        MarkerPointAreas pt_areas;
        if(isMarker(roi_bw, roi_gray, pt_locations, pt_areas, &marker)){
            for(auto &pt:pt_locations){
                pt = cv::Point2f(pt.x + rect.x, pt.y + rect.y);
            }
            marker.update(pt_locations, pt_areas);

            i++;
            cv::Mat white = cv::Mat(rect.height, rect.width, gray.type(), cv::Scalar(255));
            white.copyTo(gray(rect));
#if DEBUG_MARKER_LOCATOR
            MTRACE("Track marker successfully: pt.size:%ld, area.size:%ld\n",
                   pt_locations.size(), pt_areas.size());
            show("trackMarker:isMarker", roi_bw);
            show("trackMarker:", gray); cv::waitKey(0);
#endif
        }
        else{
            _markers = epm::rmElement(_markers, i);
        }
    }
    return gray;
}


/*****************************************************************************/
std::vector<int> filterFlags(std::vector<int>& flags, float mean_area, int count)
{
    for(uint16_t i = 0; i < flags.size(); i++){
        if(!flags[i]) continue;
#if DEBUG_CODES
        printf("area: %d\t", flags[i]);
#endif
        flags[i] = abs(flags[i] - mean_area) + 1;
#if DEBUG_CODES
        printf("delta area: %d\n", flags[i]);
#endif
    }

    for(uint16_t i = 0; i < count - 8; i++){
        flags[findMaxValueId(flags)] = 0;
    }

    return flags;
}


std::vector<int> filterIndices(std::vector<int>& indicies, const cv::Mat& stats)
{
    size_t i = 0;
    while(i < indicies.size()){
        int id = indicies[i];
        int w = stats.at<int>(id, 2);
        int h = stats.at<int>(id, 3);
        float ratio = 1.0 * MIN(w, h) / MAX(w, h);
        if (ratio < 0.35){
            indicies = rmElement(indicies, i);
        }
        else{
            i++;
        }
    }
    return indicies;
}


bool filterIndices(std::vector<int>& indices, const cv::Mat& stats,
                               const cv::Mat& centroids, const cv::Mat& gray)
{
    auto meanValue = [&gray](int x1, int x2, int y) -> float {
        float sum = 0;
        int count = 0;
        for(int x = x1; x <= x2; x++){
            sum += gray.at<uchar>(y, x);
            count++;
        }
        return sum / count;
    };

    std::vector<float> P_is_circle(indices.size());
    for(size_t i = 0; i < indices.size(); i++) {
        int id = indices[i];
        int xc = centroids.at<double>(id, 0);
        int yc = centroids.at<double>(id, 1);
        float center = gray.at<uchar>(yc, xc);

        int x = stats.at<int>(id, 0);
        int y = stats.at<int>(id, 1);
        int w = stats.at<int>(id, 2);
        int h = stats.at<int>(id, 3);
        int d = h*0.08;
        int y1 = MAX(y - d, 0);
        int y2 = MIN(y + h + d, gray.rows - 1);
        float surround = 0.5*meanValue(x, x+w, y1) +
                0.5 * meanValue(x, x+w, y2);
        P_is_circle[i] = center / surround;
#if DEBUG_MARKER_LOCATOR
        printf("\t#%ld Contrast: %f\n", i, P_is_circle[i]);
#endif
    }

    size_t i = 0;
    while(i < indices.size()){
        float p = P_is_circle[i];
        if (p < 1.5){
            P_is_circle = rmElement(P_is_circle, i);
            indices = rmElement(indices, i);
        }
        else{
            i++;
        }
    }
#if DEBUG_MARKER_LOCATOR
        printf("\tAfter filtering by contrast: %ld\n", indices.size());
#endif
    if (indices.size() < 8) return false;

    while(indices.size() > 8){
        size_t id = findMinValueId(P_is_circle);
        P_is_circle = rmElement(P_is_circle, id);
        indices = rmElement(indices, id);
    }
    return true;
}


bool isCentroidValid(std::vector<int>& indices, const cv::Mat& centroids)
{
    float min_xc = centroids.at<double>(indices[0], 0);
    float max_xc = min_xc;
    float min_yc = centroids.at<double>(indices[0], 1);
    float max_yc = min_yc;
    for(size_t i = 1; i < indices.size(); i++){
        int id = indices[i];
        float xc = centroids.at<double>(id, 0);
        float yc = centroids.at<double>(id, 1);
        if(xc > max_xc) { max_xc = xc; }
        else if(xc < min_xc) { min_xc = xc; }

        if(yc > max_yc) { max_yc = yc; }
        else if(yc < min_yc) { min_yc = yc; }
    }
    float max_range = MAX(max_xc - min_xc, max_yc - min_yc);
    float min_range = MIN(max_xc - min_xc, max_yc - min_yc);
    if(min_range / max_range < 0.3 || max_range > 360){
        return false;
    }
    return true;
}


bool isAreaValid(std::vector<int>& indices, const cv::Mat& stats)
{
    int area_min = stats.at<int>(indices[0], 4);
    int area_max = area_min;
    for(size_t i = 1; i < indices.size(); i++){
        int area = stats.at<int>(indices[i], 4);
        if(area > area_max){
            area_max = area;
        }
        else if(area < area_min){
            area_min = area;
        }
    }
    if(area_min - area_max > 400){
        return false;
    }
    return true;
}


bool isMarker(cv::Mat& bw, const cv::Mat& gray,
              MarkerPointLocations& pt_locations, MarkerPointAreas& pt_areas,
              const EightPointMarker* prev_marker)
{
    pt_locations.clear();
    pt_locations.reserve(8);
    pt_areas.clear();
    pt_areas.reserve(8);

    uint16_t pt_area_min = MARKER_PT_AREA_MIN;
    uint16_t pt_area_max = MARKER_PT_AREA_MAX;
    if(prev_marker){
        pt_area_min = prev_marker->ptAreaMin();
        pt_area_max = prev_marker->ptAreaMax();
    }

    bw = ~bw;
#if DEBUG_MARKER_LOCATOR
    printf("isMarker: a new roi_bw\n");
    show("isMarker::~roi_bw", bw); cv::waitKey(0);
#endif

    cv::Mat labels, stats, centroids;
    int num = cv::connectedComponentsWithStats(bw, labels, stats, centroids);
    std::vector<int> indices;
    for(uint16_t i = 1; i < num; i++){
        int area = stats.at<int>(i, 4);
        if(area > pt_area_min && area < pt_area_max){
            indices.push_back(i);
#if DEBUG_MARKER_LOCATOR
            printf("\t\tarea: %d\n", area);
            show("isMarker:current_bw", labels == i);
            cv::waitKey(0);
#endif
        }
    }
#if DEBUG_MARKER_LOCATOR
        printf("\tPossible circle: %ld\n", indices.size());
#endif
    if (indices.size() < 8){
        return false;
    }

    // Filter by local shape
    indices = filterIndices(indices, stats);
#if DEBUG_MARKER_LOCATOR
    printf("\tAfter filtering by shape: %ld\n", indices.size());
#endif
    if (indices.size() < 8) return false;
    // Filter by local contrast
    if(!filterIndices(indices, stats, centroids, gray))
        return false;

    // Filter the final centroid
    if(!isCentroidValid(indices, centroids))
        return false;

    // Filter the final area
    if(!isAreaValid(indices, stats))
        return false;


    // Get the centroid and area
    int xc = 0, yc = 0;
    for(size_t i = 0; i < indices.size(); i++){
        int id = indices[i];
        xc = centroids.at<double>(id, 0);
        yc = centroids.at<double>(id, 1);
        pt_locations.push_back(cv::Point2f(xc, yc));

        int area = stats.at<int>(id, 4);
        pt_areas.push_back(area);
    }
    return true;
}

} // namespace::epm
