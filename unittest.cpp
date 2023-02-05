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


static bool PolynomialMult()
{
    using namespace CosineKitty;

    // Confirm that we can represent multiplication of
    // binomials of the form (x - x_j) into a general polynomial.

    using poly_t = Polynomial<double>;

    poly_t prod {-1, 1};        // -1 + x

    return Pass("PolynomialMult");
}


static bool LinearTestDouble()
{
    CosineKitty::Interpolator<double, double> interp;

    if (!interp.insert(-5.0, 7.0) ||
        !interp.insert( 0.0, 4.0) ||
        !interp.insert(+3.0, 9.0))
    {
        printf("LinearTestDouble: FAIL: did not insert all points.\n");
        return false;
    }

    // Verify that the supplied points evaluate exactly (within tolerance).
    const double tolerance = 1.0e-14;

    return (
        Check("LinearTestDouble", interp, -5.0, 7.0, tolerance) &&
        Check("LinearTestDouble", interp,  0.0, 4.0, tolerance) &&
        Check("LinearTestDouble", interp, +3.0, 9.0, tolerance) &&
        Pass("LinearTestDouble")
    );
}


static bool FailDuplicate()
{
    CosineKitty::Interpolator<double, double> interp;

    if (!interp.insert(3.0, 4.0))
    {
        printf("FailDuplicate: should have inserted first point.\n");
        return false;
    }

    if (interp.insert(3.0, 5.0))
    {
        printf("FailDuplicate: should NOT have inserted second point.\n");
        return false;
    }

    return Pass("FailDuplicate");
}


int main()
{
    return (
        PolynomialMult() &&
        LinearTestDouble() &&
        FailDuplicate()
    ) ? 0 : 1;
}
