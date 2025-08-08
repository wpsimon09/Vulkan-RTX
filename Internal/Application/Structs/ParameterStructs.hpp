#ifndef PARAMETERSTRUCTS_HPP

#define PARAMETERSTRUCTS_HPP

struct BilaterialFilterParameters
{
    float sigma  = 0.2f;
    float BSigma = 0.9f;
    float width;
    float height;
};

#endif