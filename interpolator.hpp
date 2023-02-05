#ifndef __COSINEKITTY_INTERPOLATOR_HPP
#define __COSINEKITTY_INTERPOLATOR_HPP

#include <vector>

namespace CosineKitty
{
    template<typename numeric_t>
    class Polynomial
    {
    private:
        std::vector<numeric_t> coeff;

    public:
        // Create an empty list [] = 0.
        Polynomial() {}

        // Coefficients are given in increasing order of power of x.
        // That is: [C0, C1, ..., C[n-1]) represents the polynomial
        // C0 + C1*x + C2*x^2 + ... + C[n-1]*x^(n-1)
        Polynomial(std::initializer_list<numeric_t> coefficients)
            : coeff(coefficients)
            {}

        Polynomial(const std::vector<numeric_t>& coefficients)
            : coeff(coefficients)
            {}

        numeric_t operator() (numeric_t x) const
        {
            numeric_t sum = 0;
            numeric_t xpow = 1;
            for (numeric_t c : coeff)
            {
                sum += c * xpow;
                xpow *= x;
            }
            return sum;
        }

        const std::vector<numeric_t>& coefficients() const
        {
            return coeff;
        }

        Polynomial operator* (const Polynomial& other) const
        {
            using namespace std;

            const size_t a = coeff.size();
            const size_t b = other.coeff.size();

            vector<numeric_t> prod;
            prod.resize(a + b - 1);

            for (size_t i = 0; i < a; ++i)
                for (size_t j = 0; j < b; ++j)
                    prod[i+j] += coeff[i] * other.coeff[j];

            return Polynomial{prod};
        }

        Polynomial& operator *= (const Polynomial& other)
        {
            *this = *this * other;
            return *this;
        }

        Polynomial operator+ (const Polynomial& other) const
        {
            using namespace std;
            const size_t a = coeff.size();
            const size_t b = other.coeff.size();
            const size_t n = max(a, b);
            vector<numeric_t> sum;
            sum.resize(n);
            for (size_t i = 0; i < n; ++i)
            {
                if (i < a)
                    sum[i] += coeff[i];
                if (i < b)
                    sum[i] += other.coeff[i];
            }
            return Polynomial{sum};
        }

        Polynomial& operator += (const Polynomial& other)
        {
            *this = *this + other;
            return *this;
        }
    };


    template<typename domain_t, typename range_t>
    Polynomial<range_t> operator * (range_t scalar, const Polynomial<domain_t>& poly)
    {
        // Tricky: we have a polynomial whose coefficients are of type `domain_t`.
        // We want to multiply by a possibly different type `range_t` scalar
        // to produce a polynomial whose coefficients are of type `range_t`.
        // For example, domain_t might be float, range_t might be complex<float>.
        // It is up to the caller to ensure that multiplying a range_t by a domain_t
        // can produce a range_t, and that such multiplication makes sense.
        std::vector<range_t> product;
        for (domain_t d : poly.coefficients())
            product.push_back(scalar * d);
        return Polynomial<range_t>(product);
    }


    template<typename domain_t, typename range_t>
    class Interpolator
    {
    private:
        struct point_t
        {
            const domain_t x;
            const range_t  y;

            point_t(domain_t _x, range_t _y)
                : x(_x)
                , y(_y)
                {}
        };

        std::vector<point_t> points;

    public:
        void clear()
        {
            points.clear();
        }

        bool insert(domain_t x, range_t y)
        {
            // The x value must never appear more than once,
            // otherwise there can be inconsistent y values for the same x.
            // Even if the y values are the same, a duplicate would cause
            // division by zero later.
            for (const point_t& p : points)
                if (p.x == x)
                    return false;

            points.push_back(point_t{x, y});
            return true;
        }

        Polynomial<range_t> polynomial() const
        {
            using namespace std;
            const size_t n = points.size();

            Polynomial<range_t> sum;
            for (size_t j = 0; j < n; ++j)
            {
                Polynomial<domain_t> fraction { 1 };
                for (size_t k = 0; k < n; ++k)
                {
                    if (k != j)
                    {
                        // fraction *= (x - x_k)/(x_j - x_k)
                        domain_t denom = points[j].x - points[k].x;
                        fraction *= Polynomial<range_t>{-points[k].x / denom, 1 / denom};
                    }
                }
                sum += points[j].y * fraction;
            }

            return sum;
        }
    };
};

#endif // __COSINEKITTY_INTERPOLATOR_HPP
