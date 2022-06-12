#include "eight_point_marker.h"
#include "eight_point_marker_util.h"
#include <cstdlib>

namespace epm{

EightPointMarker::EightPointMarker(const MarkerPointLocations &pts,
                                   const MarkerPointAreas &pt_areas)
    : _gap_ratio(1.8)
    , _min_area_ratio(0.72)
    , _max_area_ratio(1.25)
    , _pts(pts)
{
    uint16_t area_min, area_max;
    findMinMax(pt_areas, area_min, area_max);
    _pt_area_min = area_min * _min_area_ratio;
    _pt_area_max = area_max * _max_area_ratio;

    uchar r = std::rand() % 256;
    uchar g = std::rand() % 256;
    uchar b = std::rand() % 256;
    _color = cv::Scalar(r, g, b);
}


EightPointMarker EightPointMarker::operator=(EightPointMarker &marker)
{
    this->_pt_area_max = marker._pt_area_max;
    this->_pt_area_min = marker._pt_area_min;
    this->_pts = marker._pts;
    this->_color = marker._color;
    return *this;
}


void EightPointMarker::update(const MarkerPointLocations &pts,
                              const MarkerPointAreas &pt_areas)
{
    _pts = pts;

    uint16_t area_min, area_max;
    findMinMax(pt_areas, area_min, area_max);
    _pt_area_min = area_min * _min_area_ratio;
    _pt_area_max = area_max * _max_area_ratio;
}


uint16_t EightPointMarker::ptAreaMin() const
{
    return  _pt_area_min;
}


uint16_t EightPointMarker::ptAreaMax() const
{
    return _pt_area_max;
}


const MarkerPointLocations &EightPointMarker::pts() const
{
    return _pts;
}


const cv::Scalar &EightPointMarker::color() const
{
    return _color;
}


cv::Rect EightPointMarker::rect()
{
    float min_x = _pts[0].x;
    float max_x = min_x;
    float min_y = _pts[0].y;
    float max_y = min_y;
    for(size_t i = 1; i < _pts.size(); i++){
        float x = _pts[i].x;
        float y = _pts[i].y;
        if(x > max_x) { max_x = x; }
        else if(x < min_x) { min_x = x; }

        if(y > max_y) { max_y = y; }
        else if(y < min_y) { min_y = y; }
    }

    float x_range = max_x - min_x;
    float y_range = max_y - min_y;
    // Create new Rect with gap
    float x_gap = x_range * (_gap_ratio - 1) * 0.5;
    uint16_t xs = MAX((int)(min_x - x_gap + 0.5), 0);
    uint16_t xe = MIN((int)(max_x + x_gap + 0.5), 1920-1);
    float y_gap = y_range * (_gap_ratio - 1) * 0.5;
    uint16_t ys = MAX((int)(min_y - y_gap + 0.5), 0);
    uint16_t ye = MIN((int)(max_y + y_gap + 0.5), 1080-1);

    return cv::Rect(xs, ys, xe - xs, ye - ys);
}

} // namespace::epm
