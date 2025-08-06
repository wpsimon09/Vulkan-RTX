#ifndef PARAMETERSTRUCTS_HPP

#define PARAMETERSTRUCTS_HPP

struct BilaterialFilterParameters
{
    float sigma  = 4.0f;
    float BSigma = 0.9f;
    float width;
    float height;
};

#endif