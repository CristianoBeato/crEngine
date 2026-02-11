
#ifndef __GUI_COMMON_HPP__
#define __GUI_COMMON_HPP__

namespace gui
{
    struct Rect_t
    {
        int32_t     x = 0;
        int32_t     y = 0;
        uint32_t    width = 0;
        uint32_t    height = 0; 
    };
};

#include "controls/Base.hpp"

#endif //!__GUI_COMMON_HPP__