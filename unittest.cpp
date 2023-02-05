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
    domain_t x,
    range_t yCorrect,
    range_t yCalc,
    double tolerance)
{
    double diff = std::abs(yCalc - yCorrect);
    printf("%s(%lf): diff = %le\n", caller, static_cast<double>(x), diff);
    if (diff > tolerance)
    {
        printf("FAIL: excessive error!\n");
        return false;
    }
    return true;
}


template <typename domain_t, typename range_t>
bool CheckInterpolator(
    const char *caller,
    const CosineKitty::Interpolator<domain_t, range_t>& interp,
    domain_t x,
    range_t yCorrect,
    double tolerance)
{
    return Check(caller, x, yCorrect, interp.calc(x), tolerance);
}


template <typename domain_t>
bool CheckPolynomial(
    const char *caller,
    const CosineKitty::Polynomial<domain_t>& poly,
    domain_t x,
    domain_t yCorrect,
    double tolerance)
{
    return Check(caller, x, yCorrect, poly(x), tolerance);
}


static bool PolynomialMult()
{
    using namespace CosineKitty;
    using poly_t = Polynomial<double>;
    const double tolerance = 1.0e-14;

    // Create a simple binomial.
    poly_t prod {-1, 1};        // -1 + x

    // Verify we can evaluate the polynomial for different values of x.
    if (!CheckPolynomial("PolynomialMult", prod, 3.5, 2.5, tolerance)) return false;
    if (!CheckPolynomial("PolynomialMult", prod, 7.2, 6.2, tolerance)) return false;

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
        CheckInterpolator("LinearTestDouble", interp, -5.0, 7.0, tolerance) &&
        CheckInterpolator("LinearTestDouble", interp,  0.0, 4.0, tolerance) &&
        CheckInterpolator("LinearTestDouble", interp, +3.0, 9.0, tolerance) &&
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
