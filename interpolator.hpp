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
    /// @brief Represents a polynomial `y = f(x)` in terms of an indepdendent variable `x`.
    /// @tparam domain_t
    /// The numeric type of the independent variable `x`.
    /// It may be a real-like type such as `float` or `double`,
    /// a complex-like type such as `std::complex<double>`, or even
    /// a custom type that has analytic properties.
    /// Not allowed to be integer-like.
    /// @tparam range_t
    /// The numeric type of the polynomial `y = f(x)`.
    /// It may be a real-like type such as `float` or `double`,
    /// a complex-like type such as `std::complex<double>`, or even
    /// a custom type that has analytic properties.
    /// Not allowed to be integer-like.
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
        /// @brief Creates a polynomial that represents the function f(x) = 0.
        Polynomial() {}

        /// @brief Create a polynomial with specified coefficients.
        /// @param coefficients
        /// Coefficients are given in increasing order of power of x.
        /// That is: [C0, C1, ..., C[n-1]) represents the polynomial
        /// C0 + C1*x + C2*x^2 + ... + C[n-1]*x^(n-1)
        Polynomial(std::initializer_list<range_t> coefficients)
            : coeff(coefficients)
        {
            truncate();
        }

        /// @brief Creates a polynomial with specified coefficients.
        /// @param coefficients
        /// Coefficients are given in increasing order of power of x.
        /// That is: [C0, C1, ..., C[n-1]) represents the polynomial
        /// C0 + C1*x + C2*x^2 + ... + C[n-1]*x^(n-1)
        Polynomial(const std::vector<range_t>& coefficients)
            : coeff(coefficients)
        {
            truncate();
        }

        /// @brief Evaluates the polynomial for a given value of x.
        /// @param x The value of the independent variable.
        /// @return The value of the polynomial f(x), given the value x.
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

        /// @brief Indicates whether the polynomial is the constant function f(x) = 0.
        /// @return `true` if this polynomial is equivalent to zero, otherwise `false`.
        bool isZero() const
        {
            return coeff.empty();
        }

        /// @brief Allows read-only access to the coefficients inside this polynomial.
        /// @return A reference to a vector of coefficients inside this polynomial.
        const std::vector<range_t>& coefficients() const
        {
            return coeff;
        }

        /// @brief The unary `+` operator, which has no effect. Provided for convenience.
        /// @return The value of this polynomial, unchanged.
        const Polynomial& operator+ () const
        {
            return *this;
        }

        /// @brief The unary `-` operator, which returns the negative of this polynomial.
        /// @return A new polynomial equal to the negative of this polynomial.
        Polynomial operator- () const
        {
            std::vector<range_t> neg;
            for (range_t c : coeff)
                neg.push_back(-c);
            return Polynomial{neg};
        }

        /// @brief Multiplies this polynomial by a scalar constant.
        /// @param scalar The scalar value to multiply by this polynomial.
        /// @return A new polynomial equal to this polynomial times the given scalar.
        Polynomial operator* (range_t scalar) const
        {
            std::vector<range_t> product;
            for (range_t d : coeff)
                product.push_back(scalar * d);
            return Polynomial{product};
        }

        /// @brief Multiplies two polynomials.
        /// @param other Another polynomial to multiply with this one.
        /// @return A new polynomial equal to the product of the two supplied polynomials.
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

        /// @brief Updates this polynomial by multiplying it with another polynomial.
        /// @param other The other polynomial to multiply this one with.
        /// @return A reference to this polynomial, which has been updated by multiplication.
        Polynomial& operator *= (const Polynomial& other)
        {
            *this = *this * other;
            return *this;
        }

        /// @brief Adds two polynomials.
        /// @param other Another polynomial to add this one with.
        /// @return A new polynomial equal to the sum of the two supplied polynomials.
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

        /// @brief Updates this polynomial by adding another polynomial to it.
        /// @param other The other polynomial to add to this one.
        /// @return A reference to this polynomial, which has been updated by addition.
        Polynomial& operator += (const Polynomial& other)
        {
            *this = *this + other;
            return *this;
        }

        /// @brief Subtracts two polynomials.
        /// @param other Another polynomial to subtract from this one.
        /// @return A new polynomial equal to the difference of the two supplied polynomials.
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

        /// @brief Updates this polynomial by subtracting another polynomial from it.
        /// @param other The other polynomial to subtract from this one.
        /// @return A reference to this polynomial, which has been updated as a side-effect of the subtraction.
        Polynomial& operator -= (const Polynomial& other)
        {
            *this = *this - other;
            return *this;
        }

        /// @brief Raises this polynomial to a non-negative integer power.
        /// @param exponent
        /// A non-negative integer power.
        /// This function throws `std::range_error` if `exponent` is negative.
        /// Raising any polynomial, including f(x) = 0, to the integer 0 power,
        /// results in the polynomial g(x) = 1.
        /// Although 0**0 is undefined in mathematics, this convention allows certain
        /// polynomial operations to be coded elegantly.
        /// @return A new polynomial equal to this polynomial raised to the `exponent` power.
        Polynomial pow(int exponent) const
        {
            if (exponent < 0)
                throw std::range_error("Cannot raise Polynomial to a negative power.");

            // The following two `if` statements are not strictly necessary.
            // If deleted, the code would still work correctly.
            // They are included as speed/memory optimizations.

            if (exponent == 0)
                return Polynomial{1};

            if (exponent == 1)
                return *this;

            // Square-and-accumulate algorithm.
            // Keep squaring this polynomial and select which
            // squares to include in the product from the set bits
            // inside the exponent. This approach iterates about
            // log2(exponent) times rather than `exponent` times.
            Polynomial product{1};
            Polynomial square = *this;
            for(;;)
            {
                if (exponent & 1)
                    product *= square;

                exponent >>= 1;
                if (exponent == 0)
                    break;

                square *= square;
            }

            return product;
        }

        /// @brief Takes the derivative of this polynomial with respect to its independent variable.
        /// @return A new polynomial equal to the derivative of this polynomial.
        Polynomial derivative() const
        {
            using namespace std;
            vector<range_t> deriv;
            const size_t n = coeff.size();
            for (size_t i = 1; i < n; ++i)
                deriv.push_back(static_cast<domain_t>(i) * coeff[i]);
            return Polynomial{deriv};
        }

        /// @brief Takes the indefinite integral, or antiderivative, of this polynomial.
        /// @param arbitraryConstant
        /// The value of the arbitrary constant term to be included in the resulting integral.
        /// @return A new polynomial equal to the integral of this polynomial.
        Polynomial integral(range_t arbitraryConstant = 0) const
        {
            using namespace std;
            vector<range_t> poly;
            poly.push_back(arbitraryConstant);
            const size_t n = coeff.size();
            for (size_t i = 0; i < n; ++i)
                poly.push_back(coeff[i] / static_cast<domain_t>(i+1));
            return poly;
        }
    };


    /// @brief Multiplies a scalar by a polynomial.
    /// @tparam domain_t The type of the polynomial's independent variable `x`.
    /// @tparam range_t The type of the polynomial itself: `y = f(x)`.
    /// @param scalar The scalar to multiply with.
    /// @param poly The polynomial to multiply with.
    /// @return A new polynomial equal to the product of the given scalar with the given polynomial.
    template<typename domain_t, typename range_t>
    Polynomial<domain_t, range_t> operator * (range_t scalar, const Polynomial<domain_t, range_t>& poly)
    {
        return poly * scalar;
    }


    /// @brief Composes two polynomials `f(x)` and `g(x)` to produce a polynomial `f(g(x))`.
    /// @remarks
    /// This function applies one polynomial function to another to produce a new polynomial.
    /// For example, if f(x) = 5x^2 + 2x, and g(x) = -3x + 7, then the composition is
    /// f(g(x)) = 5(-3x + 7)^2 + 2(-3x + 7) = 45x^2 - 216x + 259.
    /// @tparam domain_t The domain type of the second polynomial function `g`.
    /// @tparam inner_t The range type of the second polynomial, which must be the same as the domain type of the first polynomial.
    /// @tparam range_t The range type of the first polynomial.
    /// @param f The outer polynomial function.
    /// @param g The inner polynomial function.
    /// @return A new polynomial function representing the composition `(f*g)(x) = f(g(x))`.
    template<typename domain_t, typename inner_t, typename range_t>
    Polynomial<domain_t, range_t> compose(
        const Polynomial<inner_t, range_t>& f,
        const Polynomial<domain_t, inner_t>& g)
    {
        const std::vector<range_t>& fcoeff = f.coefficients();
        const int n = static_cast<int>(fcoeff.size());
        Polynomial<domain_t, range_t> sum;
        Polynomial<domain_t, inner_t> gpow{1};
        for (int i = 0; i < n; ++i)
        {
            if (i > 0)
                gpow *= g;
            sum += fcoeff[i] * gpow;
        }
        return sum;
    }


    /// @brief Derives a polynomial that passes through a given collection of points `(x, y)`.
    /// @tparam domain_t
    /// Given a collection of points `(x, y)`, the numeric type of the indepdendent variable `x`.
    /// It may be a real-like type such as `float` or `double`,
    /// a complex-like type such as `std::complex<double>`, or even
    /// a custom type that has analytic properties.
    /// Not allowed to be integer-like.
    /// @tparam range_t
    /// Given a collection of points `(x, y)`, the numeric type of the depdendent variable `y`.
    /// It may be a real-like type such as `float` or `double`,
    /// a complex-like type such as `std::complex<double>`, or even
    /// a custom type that has analytic properties.
    /// Not allowed to be integer-like.
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
        /// @brief Empties the collection of points inside this interpolator.
        void clear()
        {
            points.clear();
        }

        /// @brief Inserts another point `(x, y)` to this interpolator.
        /// @remarks
        /// A successful call that inserts a new point will return `true`.
        /// An interpolator requires a unique value for all `x` values supplied.
        /// If an `x` value has already been defined by a call to `insert`,
        /// and the same `x` value is passed again by later call(s) to `insert`,
        /// the later call(s) will have no effect and return `false`.
        /// @param x The value of the independent variable `x` for this point.
        /// @param y The value of the depdendent variable `y` for this point.
        /// @return If successful, `true`; otherwise `false`. See remarks.
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

        /// @brief Calculates the unique polynomial that passes through the supplied points.
        /// @return A polynomial whose value passes through all inserted points.
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
