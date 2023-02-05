#include "interpolator.hpp"
#include <cstdio>
#include <cmath>
#include <string>

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


template <typename domain_t>
std::string to_string(const std::vector<domain_t>& list)
{
    std::string text;
    bool first = true;
    for (domain_t c : list)
    {
        if (first)
        {
            text += "[";
            first = false;
        }
        else
            text += ", ";
        text += std::to_string(c);
    }
    text += "]";
    return text;
}


template <typename domain_t>
bool CompareCoeffs(
    const char *caller,
    const std::vector<domain_t>& a,
    const std::vector<domain_t>& b,
    double tolerance)
{
    bool same = (a.size() == b.size());
    if (same)
    {
        for (size_t i = 0; i < a.size(); ++i)
        {
            double diff = std::abs(a[i] - b[i]);
            if (diff > tolerance)
                same = false;
        }
    }

    if (!same)
    {
        printf("CompareCoeffs(%s): MISMATCH FAILURE:\n", caller);

        std::string aText = to_string(a);
        printf("    a = %s\n", aText.c_str());

        std::string bText = to_string(b);
        printf("    b = %s\n", bText.c_str());
    }

    return same;
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

    // Find the product (-1 + x)(-2 + x).
    // It should be (2 - 3x + x^2) = [2, -3, 1].
    prod *= poly_t{-2, 1};
    if (!CompareCoeffs("PolynomialMult", prod.coefficients(), {2.0, -3.0, 1.0}, tolerance)) return false;

    return Pass("PolynomialMult");
}


static bool PolynomialAdd()
{
    using namespace CosineKitty;
    using poly_t = Polynomial<double>;
    const double tolerance = 1.0e-14;

    poly_t a {3, -4, 5};        // 3 - 4x + 5x^2
    poly_t b {2, 7, 8, 1};      // 2 + 7x + 8x^2 + x^3
    poly_t c = a + b;

    if (!CompareCoeffs("PolynomialMult", c.coefficients(), {5.0, 3.0, 13.0, 1.0}, tolerance)) return false;

    return Pass("PolynomialAdd");
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
        PolynomialAdd() &&
        LinearTestDouble() &&
        FailDuplicate()
    ) ? 0 : 1;
}
