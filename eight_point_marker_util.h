#ifndef EIGHT_POINT_MARKER_UTIL_H
#define EIGHT_POINT_MARKER_UTIL_H
#include <cstdint>
#include <vector>

namespace epm {

template <typename Tp>
void findMinMax(const std::vector<Tp> &areas, Tp& area_min, Tp& area_max)
{
    area_min = areas[0];
    area_max = areas[0];
    for(size_t i = 1; i < areas.size(); i++){
        if(areas[i] > area_max){
            area_max = areas[i];
        }
        else if(areas[i] < area_min){
            area_min = areas[i];
        }
    }
}


template <typename Tp>
size_t findMaxValueId(const std::vector<Tp>& values)
{
    size_t max_val_idx = 0;
    Tp max_val = values[0];
    for(uint16_t i = 1; i < values.size(); i++){
        if(values[i] > max_val) {
            max_val = values[i];
            max_val_idx = i;
        }
    }
    return max_val_idx;
}


template <typename Tp>
size_t findMinValueId(const std::vector<Tp>& values)
{
    int min_val_idx = 0;
    Tp min_val = values[0];
    for(size_t i = 1; i < values.size(); i++){
        if(values[i] < min_val) {
            min_val = values[i];
            min_val_idx = i;
        }
    }
    return min_val_idx;
}


template <typename Tp>
std::vector<Tp> rmElement(std::vector<Tp>& vec, size_t index)
{
    if (index < vec.size()){
        vec[index] = vec[vec.size()-1];
        vec.pop_back();
    }
    return vec;
}

}


#endif // EIGHT_POINT_MARKER_UTIL_H
