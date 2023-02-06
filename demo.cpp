#include <cstdio>
#include <cmath>
#include "interpolator.hpp"

using interp_t = CosineKitty::Interpolator<double, double>;
using poly_t = CosineKitty::Polynomial<double, double>;

void Print(const poly_t& poly)
{
    printf("f(x) = ");
    int k = 0;
    for (double c : poly.coefficients())
    {
        if (k > 0)
        {
            if (c < 0)
                printf(" - ");
            else
                printf(" + ");
            printf("%0.6lf*x", std::abs(c));
            if (k > 1)
                printf("^%d", k);
        }
        else
            printf("%0.6lf", c);
        ++k;
    }
    printf("\n");
}

int main()
{
    // Create an interpolator.
    interp_t interp;

    // Add four points that the curve must pass through.
    interp.insert(0.0, -3.0);
    interp.insert(1.0,  2.0);
    interp.insert(2.0,  8.0);
    interp.insert(3.0, -7.0);

    // Find the cubic polynomial that passes through the four points.
    poly_t poly = interp.polynomial();

    // Print the function f(x) represented by `poly`.
    Print(poly);

    // Evaluate the polynomial at several points.
    for (double x = -0.5; x <= 3.5; x += 0.5)
        printf("f(%4.1lf) = %6.2lf\n", x, poly(x));

    return 0;
}
