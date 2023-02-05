#include "interpolator.hpp"
#include <cstdio>
#include <cmath>

static bool Pass(const char *caller)
{
    printf("%s: PASS\n", caller);
    return true;
}


template <typename domain_t, typename range_t>
bool Check(
    const char *caller,
    CosineKitty::Interpolator<domain_t, range_t>& interp,
    domain_t x,
    range_t yCorrect,
    double tolerance)
{
    range_t yCalc = interp.calc(x);
    double diff = std::abs(yCalc - yCorrect);
    printf("%s(%lf): diff = %le\n", caller, static_cast<double>(x), diff);
    if (diff > tolerance)
    {
        printf("FAIL: excessive error!\n");
        return false;
    }
    return true;
}


static bool LinearTestDouble()
{
    CosineKitty::Interpolator<double, double> interp;

    interp.insert(-5.0, 7.0);
    interp.insert( 0.0, 4.0);
    interp.insert(+3.0, 9.0);

    // Verify that the supplied points evaluate exactly (within tolerance).
    const double tolerance = 1.0e-12;

    return (
        Check("LinearTestDouble", interp, -5.0, 7.0, tolerance) &&
        Check("LinearTestDouble", interp,  0.0, 4.0, tolerance) &&
        Check("LinearTestDouble", interp, +3.0, 9.0, tolerance) &&
        Pass("LinearTestDouble")
    );
}

int main()
{
    return (
        LinearTestDouble()
    ) ? 0 : 1;
}
