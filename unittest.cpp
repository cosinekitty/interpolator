#include "interpolator.hpp"
#include <cstdio>
#include <cmath>
#include <complex>
#include <string>
#include <functional>

using float_poly_t  = CosineKitty::Polynomial<float, float>;
using double_poly_t = CosineKitty::Polynomial<double, double>;

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
bool CheckPolynomial(
    const char *caller,
    const CosineKitty::Polynomial<domain_t, range_t>& poly,
    domain_t x,
    range_t yCorrect,
    double tolerance = 0.0)
{
    return Check<domain_t, range_t>(caller, x, yCorrect, poly(x), tolerance);
}


template <typename domain_t>
std::string to_string(std::complex<domain_t> z)
{
    return "(" + std::to_string(z.real()) + "," + std::to_string(z.imag()) + ")";
}


template <typename domain_t>
std::string to_string(const std::vector<domain_t>& list)
{
    using namespace std;

    string text = "[";
    bool first = true;
    for (domain_t c : list)
    {
        if (first)
            first = false;
        else
            text += ", ";
        text += to_string(c);
    }
    text += "]";
    return text;
}


template <typename range_t>
bool CompareCoeffs(
    const char *caller,
    const std::vector<range_t>& a,
    const std::vector<range_t>& b,
    double tolerance = 0.0)
{
    double maxdiff = 0.0;
    bool same = (a.size() == b.size());
    if (same)
    {
        for (size_t i = 0; i < a.size(); ++i)
        {
            double diff = std::abs(a[i] - b[i]);
            if (diff > maxdiff)
                maxdiff = diff;
            if (diff > tolerance)
                same = false;
        }
    }

    if (!same)
    {
        printf("%s(%s): MISMATCH FAILURE: a[%u], b[%u], maxdiff=%le\n",
            __func__,
            caller,
            static_cast<unsigned>(a.size()),
            static_cast<unsigned>(b.size()),
            maxdiff
        );

        std::string aText = to_string(a);
        printf("    a = %s\n", aText.c_str());

        std::string bText = to_string(b);
        printf("    b = %s\n", bText.c_str());
    }

    return same;
}


static bool PolynomialMult()
{
    // Create a simple binomial.
    double_poly_t prod {-1, 1};        // -1 + x

    // Verify we can evaluate the polynomial for different values of x.
    if (!CheckPolynomial(__func__, prod, 3.5, 2.5)) return false;
    if (!CheckPolynomial(__func__, prod, 7.2, 6.2)) return false;

    // Find the product (-1 + x)(-2 + x).
    // It should be (2 - 3x + x^2) = [2, -3, 1].
    prod *= double_poly_t{-2, 1};
    if (!CompareCoeffs(__func__, prod.coefficients(), {2.0, -3.0, 1.0})) return false;

    return Pass(__func__);
}


static bool VerifyZeroProduct(
    const char *caller,
    const float_poly_t& a,
    const float_poly_t& b)
{
    float_poly_t product = a * b;
    size_t n = product.coefficients().size();
    if (n != 0)
    {
        printf("%s(%s): FAIL: product should have 0 coefficients, but found %u\n",
            __func__,
            caller,
            static_cast<unsigned>(n)
        );

        return false;
    }
    return true;
}


static bool PolynomialMultZero()
{
    // Verify that we can multiply with empty Polynomial objects,
    // which confirms a bug fix where I wasn't handling this correctly.

    return (
        VerifyZeroProduct("0*0", float_poly_t{}, float_poly_t{}) &&
        VerifyZeroProduct("0*(x+1)", float_poly_t{}, float_poly_t{1.0f, 1.0f}) &&
        VerifyZeroProduct("(x+1)*0", float_poly_t{1.0f, 1.0f}, float_poly_t{}) &&
        Pass(__func__)
    );
}


static bool PolynomialMultScalar()
{
    float_poly_t a {7.0f, -3.0f, 5.0f};
    float_poly_t p = -2.0f * a;
    float_poly_t q = a * -2.0f;
    std::vector<float> c {-14.0f, 6.0f, -10.0f};

    return (
        CompareCoeffs(__func__, p.coefficients(), c) &&
        CompareCoeffs(__func__, q.coefficients(), c) &&
        Pass(__func__)
    );
}


static bool PolynomialAdd()
{
    double_poly_t a {3, -4, 5};        // 3 - 4x + 5x^2
    double_poly_t b {2, 7, 8, 1};      // 2 + 7x + 8x^2 + x^3
    double_poly_t c = a + b;
    double_poly_t d = c;
    d += a;

    return (
        CompareCoeffs(__func__, c.coefficients(), {5.0,  3.0, 13.0, 1.0}) &&
        CompareCoeffs(__func__, d.coefficients(), {8.0, -1.0, 18.0, 1.0}) &&
        Pass(__func__)
    );
}


static bool PolynomialSubtract()
{
    double_poly_t a {3, -4, 5};        // 3 - 4x + 5x^2
    double_poly_t b {2, 7, 8, 1};      // 2 + 7x + 8x^2 + x^3
    double_poly_t c = a - b;
    double_poly_t d = c;
    d -= a;

    return (
        CompareCoeffs(__func__, c.coefficients(), {1.0, -11.0, -3.0, -1.0}) &&
        CompareCoeffs(__func__, d.coefficients(), {-2.0, -7.0, -8.0, -1.0}) &&
        Pass(__func__)
    );
}


static bool PolynomialPower()
{
    double_poly_t a {-1.0, +1.0};   // x - 1

    // Should throw an exception if an attempt is made to raise to a negative power.
    try
    {
        a.pow(-1);
        printf("%s: FAIL: Raising polynomial to a negative power should have failed!\n", __func__);
        return false;
    }
    catch (const std::range_error&)
    {
        // Correct behavior.
    }

    double_poly_t zero;     // f(x) = 0 is the default value of a polynomial.

    // Verify we recognize zero.
    if (!zero.isZero())
    {
        printf("%s: FAIL: did not recognize zero polynomial!\n", __func__);
        return false;
    }

    double_poly_t u = a.pow(0);     // should be 1
    double_poly_t v = zero.pow(0);  // should also be 1 (even if controversial!)
    double_poly_t p = a.pow(3);     // should be x^3 - 3*x^2 + 3*x - 1

    // Make a really big polynomial, to verify the new squaring algorithm.
    // First do it the dumb way, so we have a correct reference.
    const int exponent = 23;
    double_poly_t correct{1};
    for (int i = 0; i < exponent; ++i)
        correct *= a;
    printf("%s: p^%d = %s\n", __func__, exponent, to_string(correct.coefficients()).c_str());

    double_poly_t big = a.pow(exponent);

    return (
        CompareCoeffs(__func__, u.coefficients(), {1.0}) &&
        CompareCoeffs(__func__, v.coefficients(), {1.0}) &&
        CompareCoeffs(__func__, p.coefficients(), {-1.0, +3.0, -3.0, +1.0}) &&
        CompareCoeffs(__func__, big.coefficients(), correct.coefficients()) &&
        Pass(__func__)
    );
}


static bool PolynomialUnary()
{
    // Allow unary '+' and '-' operators on polynomials.
    const std::vector<double> posCoeff{+3.0, -4.0, +5.0, -6.0};
    const std::vector<double> negCoeff{-3.0, +4.0, -5.0, +6.0};
    double_poly_t a {posCoeff};
    double_poly_t pos = +a;
    double_poly_t neg = -a;
    return (
        CompareCoeffs(__func__, pos.coefficients(), posCoeff) &&
        CompareCoeffs(__func__, neg.coefficients(), negCoeff) &&
        Pass(__func__)
    );
}


static bool InterpTestDouble()
{
    using namespace CosineKitty;

    Interpolator<double, double> interp;

    if (!interp.insert(-5.0, 7.0) ||
        !interp.insert( 0.0, 4.0) ||
        !interp.insert(+3.0, 9.0))
    {
        printf("%s: FAIL: did not insert all points.\n", __func__);
        return false;
    }

    double_poly_t poly = interp.polynomial();
    std::string ptext = to_string(poly.coefficients());
    printf("%s: poly = %s\n", __func__, ptext.c_str());

    return (
        CheckPolynomial(__func__, poly, -5.0, 7.0) &&
        CheckPolynomial(__func__, poly,  0.0, 4.0) &&
        CheckPolynomial(__func__, poly, +3.0, 9.0) &&
        Pass(__func__)
    );
}


static bool InterpTestComplex()
{
    using namespace CosineKitty;

    using complex_t = std::complex<double>;

    Interpolator<double, complex_t> interp;

    if (!interp.insert(-5.0, complex_t{7.0, -3.0}) ||
        !interp.insert( 0.0, complex_t{4.0, +2.5}) ||
        !interp.insert(+3.0, complex_t{9.0, -1.5}))
    {
        printf("%s: FAIL: did not insert all points.\n", __func__);
        return false;
    }

    Polynomial<double, complex_t> poly = interp.polynomial();
    std::string ptext = to_string(poly.coefficients());
    printf("%s: poly = %s\n", __func__, ptext.c_str());

    return (
        CheckPolynomial(__func__, poly, -5.0, complex_t{7.0, -3.0}, 1.8e-15) &&
        CheckPolynomial(__func__, poly,  0.0, complex_t{4.0, +2.5}) &&
        CheckPolynomial(__func__, poly, +3.0, complex_t{9.0, -1.5}) &&
        Pass(__func__)
    );
}


static bool FailDuplicate()
{
    CosineKitty::Interpolator<double, double> interp;

    if (!interp.insert(3.0, 4.0))
    {
        printf("%s: should have inserted first point.\n", __func__);
        return false;
    }

    if (interp.insert(3.0, 5.0))
    {
        printf("%s: should NOT have inserted second point.\n", __func__);
        return false;
    }

    return Pass(__func__);
}


static double TestEval(std::function<double(double)> func, double x)
{
    return func(x);
}


static bool PassAsFunction()
{
    // Verify that a polynomial can be passed as a C++ std::function,
    // just like a lambda can.
    double_poly_t poly {17.0, 5.0, -3.0};      // -3x^2 + 5x + 17
    double yCalc = TestEval(poly, 2.0);
    double yCorrect = -3.0*(2.0*2.0) + 5.0*(2.0) + 17.0;
    double diff = std::abs(yCalc - yCorrect);
    printf("%s: diff = %le\n", __func__, diff);
    return Pass(__func__);
}


static bool TruncateTrailingZeroCoeffs()
{
    // It is wasteful to retain high-order coefficients that are zero.
    // For example, [3, 7, 0, 0] represents f(x) = 0*x^3 + 0*x^2 + 7*x + 3.
    // This should automatically be converted to [3, 7] = 7*x + 3.
    // Test this in a yucky case where range_t is complex.
    using range_t = std::complex<float>;
    using poly_t = CosineKitty::Polynomial<float, range_t>;

    poly_t poly {3, 7, 0, 0};
    std::vector<range_t> check {3, 7};

    return (
        CompareCoeffs(__func__, poly.coefficients(), check) &&
        Pass(__func__)
    );
}


static bool PolynomialDerivative()
{
    using complex_t = std::complex<double>;
    using poly_t = CosineKitty::Polynomial<complex_t, complex_t>;
    poly_t poly {{2.0, 1.0}, {-3.0, 7.0}, {4.0, -2.0}, {-5.0, 8.0}};
    const std::vector<complex_t> correct {{-3.0, 7.0}, {8.0, -4.0}, {-15.0, 24.0}};
    poly_t deriv = poly.derivative();
    return (
        CompareCoeffs(__func__, deriv.coefficients(), correct) &&
        Pass(__func__)
    );
}


static bool PolynomialIntegral()
{
    using complex_t = std::complex<double>;
    using poly_t = CosineKitty::Polynomial<complex_t, complex_t>;
    poly_t poly {{2.0, 1.0}, {-3.0, 7.0}, {6.0, -3.0}, {-5.0, 8.0}};
    const complex_t arbitraryConstant{6.0, 5.0};
    const std::vector<complex_t> correct {arbitraryConstant, {2.0, 1.0}, {-1.5, 3.5}, {2.0, -1.0}, {-1.25, 2.0}};
    poly_t integral = poly.integral(arbitraryConstant);
    poly_t check = integral.derivative();   // verify we can take derivative and obtain original function
    return (
        CompareCoeffs(__func__, integral.coefficients(), correct) &&
        CompareCoeffs(__func__, check.coefficients(), poly.coefficients()) &&
        Pass(__func__)
    );
}


static bool PolynomialCompose()
{
    double_poly_t f{7.5, -1, 1};        // x^2 - x + 7.5
    double_poly_t g{100, 3};            // 3x + 100
    double_poly_t h = compose(f, g);
    // h(x) = f(g(x))
    //      = (3x + 100)^2 - (3x + 100) + 7.5
    //      = (9x^2 + 600x + 10000) - (3x + 100) + 7.5
    //      = 9x^2 + 597x + 9907.5

    // Another example, directly from the doxygen comments in interpolator.hpp.
    double_poly_t a{0, 2, 5};
    double_poly_t b{7, -3};
    double_poly_t c = compose(a, b);

    return (
        CompareCoeffs(__func__, h.coefficients(), {9907.5, 597, 9}) &&
        CompareCoeffs(__func__, c.coefficients(), {259, -216, 45}) &&
        Pass(__func__)
    );
}


int main()
{
    return (
        PolynomialMult() &&
        PolynomialMultZero() &&
        PolynomialMultScalar() &&
        PolynomialAdd() &&
        PolynomialSubtract() &&
        PolynomialPower() &&
        PolynomialUnary() &&
        PolynomialDerivative() &&
        PolynomialIntegral() &&
        PolynomialCompose() &&
        InterpTestDouble() &&
        InterpTestComplex() &&
        FailDuplicate() &&
        PassAsFunction() &&
        TruncateTrailingZeroCoeffs()
    ) ? 0 : 1;
}
