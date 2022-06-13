#ifndef EIGHTPOINTMARKER_H
#define EIGHTPOINTMARKER_H
#include <cstdint>
#include <opencv2/opencv.hpp>

namespace epm{

/**
 * An array of centroid of circle in Eight-Point-Marker
 */
using MarkerPointLocations = std::vector<cv::Point2f>;
/**
 * An array of area of circle in Eight-Point-Marker
 */
using MarkerPointAreas = std::vector<uint16_t>;


/**
 * @brief The EightPointMarker class
 */
class EightPointMarker
{
public:
    EightPointMarker(const MarkerPointLocations& pts, const MarkerPointAreas& pt_areas);

    EightPointMarker operator=(EightPointMarker& marker);

    void update(const MarkerPointLocations& pts, const MarkerPointAreas &pt_areas);

    uint16_t ptAreaMin() const;
    uint16_t ptAreaMax() const;

    const MarkerPointLocations &pts() const;
    const cv::Scalar& color() const;

    cv::Rect rect(uint16_t w_limit, uint16_t h_limit);

private:
    const float _gap_ratio;
    const float _min_area_ratio;
    const float _max_area_ratio;

    uint16_t _pt_area_min;
    uint16_t _pt_area_max;

    MarkerPointLocations _pts;

    cv::Scalar _color;
};

} // namespace::epm
#endif // EIGHTPOINTMARKER_H
