#ifndef LIGHT_H
#define LIGHT_H

#include "../vmlib/vec4.hpp"

struct PointLight {

    Vec4f lPosition;
    Vec4f lColour;
    Vec4f lIntensity;
    //Vec4f lRelativePos;
};

#endif