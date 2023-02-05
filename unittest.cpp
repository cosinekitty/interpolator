#include "interpolator.hpp"
#include <cstdio>

static bool LinearTestFloat()
{
    CosineKitty::Interpolator<double, double> interp;

    interp.insert(-1.0, 7.0);
    interp.insert( 0.0, 4.0);
    interp.insert(+1.0, 9.0);

    printf("LinearTestFloat: PASS\n");
    return true;
}

int main()
{
    return (
        LinearTestFloat()
    ) ? 0 : 1;
}
