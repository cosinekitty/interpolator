/*
https://github.com/cosinekitty/interpolator

MIT License

Copyright (c) 2023 Don Cross

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __COSINEKITTY_INTERPOLATOR_HPP
#define __COSINEKITTY_INTERPOLATOR_HPP

#include <vector>
#include <stdexcept>

namespace CosineKitty
{
    template<typename domain_t, typename range_t>
    class Polynomial
    {
    private:
        std::vector<range_t> coeff;

        void truncate()
        {
            // It is wasteful to retain high-order coefficients that are zero.
            // For example, [3, 7, 0, 0] represents f(x) = 0*x^3 + 0*x^2 + 7*x + 3.
            // This should automatically be converted to [3, 7] = 7*x + 3.
            const range_t zero = 0;
            std::size_t i = coeff.size();
            while (i>0 && coeff[i-1]==zero)
                --i;
            coeff.resize(i);
        }

    public:
        // Create an empty list [] = 0.
        Polynomial() {}

        // Coefficients are given in increasing order of power of x.
        // That is: [C0, C1, ..., C[n-1]) represents the polynomial
        // C0 + C1*x + C2*x^2 + ... + C[n-1]*x^(n-1)
        Polynomial(std::initializer_list<range_t> coefficients)
            : coeff(coefficients)
        {
            truncate();
        }

        Polynomial(const std::vector<range_t>& coefficients)
            : coeff(coefficients)
        {
            truncate();
        }

        range_t operator() (domain_t x) const
        {
            std::size_t i = coeff.size();
            if (i == 0)
                return 0;
            range_t sum = coeff[--i];
            while (i > 0)
                sum = x*sum + coeff[--i];
            return sum;
        }

        bool isZero() const
        {
            return coeff.empty();
        }

        const std::vector<range_t>& coefficients() const
        {
            return coeff;
        }

        const Polynomial& operator+ () const
        {
            return *this;
        }

        Polynomial operator- () const
        {
            std::vector<range_t> neg;
            for (range_t c : coeff)
                neg.push_back(-c);
            return Polynomial{neg};
        }

        Polynomial operator* (range_t scalar) const
        {
            std::vector<range_t> product;
            for (range_t d : coeff)
                product.push_back(scalar * d);
            return Polynomial{product};
        }

        Polynomial operator* (const Polynomial& other) const
        {
            using namespace std;

            const size_t a = coeff.size();
            const size_t b = other.coeff.size();

            // If either polynomial is zero (empty coefficient list),
            // the product is zero. This must be handled as a special
            // case to avoid trying to create a coefficient list below
            // whose length is -1.
            if (a == 0 || b == 0)
                return Polynomial{};

            vector<range_t> prod;
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
            vector<range_t> sum;
            sum.resize(n);
            for (size_t i = 0; i < n; ++i)
            {
                if (i < a)
                    sum[i] = coeff[i];
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

        Polynomial operator- (const Polynomial& other) const
        {
            using namespace std;
            const size_t a = coeff.size();
            const size_t b = other.coeff.size();
            const size_t n = max(a, b);
            vector<range_t> diff;
            diff.resize(n);
            for (size_t i = 0; i < n; ++i)
            {
                if (i < a)
                    diff[i] = coeff[i];
                if (i < b)
                    diff[i] -= other.coeff[i];
            }
            return Polynomial{diff};
        }

        Polynomial& operator -= (const Polynomial& other)
        {
            *this = *this - other;
            return *this;
        }

        Polynomial pow(int exponent) const
        {
            if (exponent < 0)
                throw std::range_error("Cannot raise Polynomial to a negative power.");

            if (exponent == 0 && isZero())
                throw std::range_error("Cannot raise zero Polynomial to the power zero.");

            Polynomial product{1};

            if (exponent > 0)
            {
                Polynomial square = *this;

                // Square-and-accumulate algorithm.
                // Keep squaring this polynomial and select which
                // squares to include in the product from the set bits
                // inside the exponent. This approach iterates about
                // log2(exponent) times rather than `exponent` times.
                for(;;)
                {
                    if (exponent & 1)
                        product *= square;

                    exponent >>= 1;
                    if (exponent == 0)
                        break;

                    square *= square;
                }
            }

            return product;
        }
    };

    template<typename domain_t, typename range_t>
    Polynomial<domain_t, range_t> operator * (range_t scalar, const Polynomial<domain_t, range_t>& poly)
    {
        return poly * scalar;
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

        Polynomial<domain_t, range_t> polynomial() const
        {
            using namespace std;
            const size_t n = points.size();

            Polynomial<domain_t, range_t> sum;
            for (size_t j = 0; j < n; ++j)
            {
                Polynomial<domain_t, range_t> fraction { 1 };
                for (size_t k = 0; k < n; ++k)
                {
                    if (k != j)
                    {
                        // fraction *= (x - x_k)/(x_j - x_k)
                        domain_t denom = points[j].x - points[k].x;
                        fraction *= Polynomial<domain_t, range_t>{-points[k].x / denom, 1 / denom};
                    }
                }
                sum += points[j].y * fraction;
            }

            return sum;
        }
    };
};

#endif // __COSINEKITTY_INTERPOLATOR_HPP
