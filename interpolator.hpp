#ifndef __COSINEKITTY_INTERPOLATOR_HPP
#define __COSINEKITTY_INTERPOLATOR_HPP

#include <vector>

namespace CosineKitty
{
    template<typename domain_t>
    class Polynomial
    {
    private:
        std::vector<domain_t> coeff;

    public:
        // Coefficients are given in increasing order of power of x.
        // That is: [C0, C1, ..., C[n-1]) represents the polynomial
        // C0 + C1*x + C2*x^2 + ... + C[n-1]*x^(n-1)
        Polynomial(std::initializer_list<domain_t> coefficients)
            : coeff(coefficients)
            {}

        Polynomial(const std::vector<domain_t>& coefficients)
            : coeff(coefficients)
            {}

        domain_t operator() (domain_t x) const
        {
            domain_t sum = 0;
            domain_t xpow = 1;
            for (domain_t c : coeff)
            {
                sum += c * xpow;
                xpow *= x;
            }
            return sum;
        }

        const std::vector<domain_t>& getCoefficients() const
        {
            return coeff;
        }

        Polynomial operator* (const Polynomial& other) const
        {
            using namespace std;

            // Special case: an empty polynomial [] represents zero.
            // If either polynomial is zero, return zero.
            if (coeff.size() == 0 || other.coeff.size() == 0)
                return Polynomial{};

            vector<domain_t> prod;
            // The length of each list of coefficients is one greater
            // than the highest power of x in the associated polynomial.
            // The highest power of x in the product is then
            // (L1-1) + (L2-1) = L1 + L2 - 2.
            // Then we need L1+L2-1 coefficients for the product.
            prod.resize(coeff.size() + other.coeff.size() - 1);

            // Calculate the product's coefficients.
            for (size_t i = 0; i < coeff.size(); ++i)
                for (size_t j = 0; j < other.coeff.size(); ++j)
                    prod[i+j] += coeff[i] * other.coeff[j];

            return Polynomial{prod};
        }
    };


    template<typename domain_t, typename range_t>
    struct DataPoint
    {
        domain_t x;
        range_t y;
    };


    template<typename domain_t, typename range_t>
    class Interpolator
    {
    public:
        using point_t = DataPoint<domain_t, range_t>;

    private:
        struct Coefficient
        {
            point_t point;
            std::vector<domain_t> diffs;
            domain_t denom;
        };

        std::vector<Coefficient> coeffs;

    public:
        bool insert(domain_t x, range_t y)
        {
            // The x value must never appear more than once,
            // otherwise there can be inconsistent y values for the same x.
            // Even if the y values are the same, a duplicate would cause
            // division by zero later.
            for (const Coefficient& c : coeffs)
                if (c.point.x == x)
                    return false;

            // Update all existing coefficients to evaluate to 0
            // at the new x value, while still evaluating to 1 at
            // their own dedicated x value.
            for (Coefficient& c : coeffs)
            {
                // Append another term to the numerator's polynomial.
                c.diffs.push_back(x);

                // Multiply the denominator by the new correction factor.
                c.denom *= (c.point.x - x);
            }

            // Create and append a new coefficient for the new (x, y) point.
            Coefficient d;
            d.point.x = x;
            d.point.y = y;
            d.denom = 1;
            for (const Coefficient &c : coeffs)
            {
                d.diffs.push_back(c.point.x);
                d.denom *= (x - c.point.x);
            }
            coeffs.push_back(d);

            // Success!
            return true;
        }

        range_t calc(domain_t x) const
        {
            range_t y {};
            for (const Coefficient &c : coeffs)
            {
                domain_t product = 1;
                for (domain_t d : c.diffs)
                    product *= (x - d);
                y += (product / c.denom)*c.point.y;
            }
            return y;
        }
    };
};

#endif // __COSINEKITTY_INTERPOLATOR_HPP
