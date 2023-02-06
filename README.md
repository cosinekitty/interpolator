# Interpolator
Here is a polynomial interpolator implemented as a C++ header file:
[interpolator.hpp](interpolator.hpp)

I recently learned this method of creating a generic polynomial $f(x)$
that passes through an arbitrary collection of points $(x_i, y_i)$.
In other words, $y_i=f(x_i)$ for each point in the collection.

The idea behind the algorithm is so simple and intuitive that
I thought it would be fun to implement and share.

# Theory

## Quadratic example problem

I will start with an easy but non-trivial example.
Suppose we have three points on the Cartesian plane:

| $x$ | $y$ |
| --- | --- |
| 1 | 7 |
| 2 | -2 |
| 3 | 6 |

We want to find the formula for a quadratic polynomial that generates these values.
When plotted, this function will result in a parabola that passes through the points.

Now let's imagine that we can express $f(x)$ using the 3 required $y$ values, like this:

$$
f(x) = 7A(x) - 2B(x) + 6C(x)
$$

All I've done here is use each of the desired $y$ values as coefficients for
some new functions of $x$ that I haven't defined yet: $A(x)$, $B(x)$, and $C(x)$.

The simple idea is, when $x=1$, we want $A(x)=1$, $B(x)=0$, and $C(x)=0$.
That way, we "select" the first value, $7$, and "ignore" the $-2$ and $6$. This results in

$$
f(1) = 7(1) - 2(0) + 6(0) = 7
$$

Likewise, when $x=2$, we want $A(x)=0$, $B(x)=1$, and $C(x)=0$, resulting in

$$
f(2) = 7(0) - 2(1) + 6(0) = -2
$$

Completing the pattern, we also need

$$
f(3) = 7(0) - 2(0) + 6(1) = 6
$$

## Deriving the coefficient functions

How do we find the functions $A(x)$, $B(x)$, and $C(x)$?

Let's start with $A(x)$. And let's focus on it needing to be
zero when $x=2$ or when $x=3$. Here is an easy way to accomplish
the zero cases:

$$
A(x) = (x-2)(x-3)
$$

However, there is a problem. If we evaluate $A(1)$, we find the value is

$$
A(1) = (1-2)(1-3) = 2
$$

We need the value to be 1, not 2, so the fix is to divide by 2 in the definition of $A(x)$:

$$
A(x) = \frac{(x-2)(x-3)}{2}
$$

And that's it! The remaining functions follow the same line of thinking.
This results in the desired polynomial solution:

$$
f(x) = (7)\frac{(x-2)(x-3)}{2} + (-2)\frac{(x-1)(x-3)}{-1} + (6)\frac{(x-1)(x-2)}{2}
$$

With a little algebra, the binomials can be expanded, terms can be collected and simplified, and the result is a tidy quadratic polynomial:

$$
f(x) = \frac{17}{2}x^2 - \frac{69}{2}x + 33
$$

A little sanity check in Python confirms we didn't make any mistakes:

```python
>>> def f(x):
...     return (17/2)*x*x - (69/2)*x + 33
...
>>> f(1)
7.0
>>> f(2)
-2.0
>>> f(3)
6.0
```

# General solution

The same approach generalizes to any number of points $n$,
resulting in a polynomial of order $(n-1)$.
Following the procedure outlined above, you can write a cubic function
that passes through 4 points, a quartic function that passes through
5 points, and so on. The beauty of this approach is that it doesn't
require solving systems of equations, finding the roots of polynomials,
or anything complicated like that.

Written in a general form, the polynomial curve passing through
$n$ points is

$$
\begin{align*}
f(x)
&= y_0 \frac{(x-x_1)\cdots(x-x_{n-1})}{(x_0-x_1)\cdots(x_0-x_{n-1})} \\
&+ y_1 \frac{(x-x_0)\cdots(x-x_{n-1})}{(x_1-x_0)\cdots(x_1-x_{n-1})} \\
&+ \cdots \\
&+ y_{n-1} \frac{(x-x_0)\cdots(x-x_{n-2})}{(x_{n-1}-x_0)\cdots(x_{n-1}-x_{n-2})} \\
\end{align*}
$$

Note that the fraction after each $y_j$ has a product of $(x-x_k)$
factors in the numerator, where $k$ ranges from $0$ to $(n-1)$, but
omitting the case $k=j$. Likewise, the denominator has a product
of $(x_j-x_k)$ terms, also omitting the $k=j$ case.

Written more compactly, the formula is

$$
f(x) = \sum_{j=0}^{n-1} \left( y_j \prod_{\substack{k=0 \\ k \ne j}}^{n-1} \frac{x-x_k}{x_j-x_k} \right)
$$

# Algebra optimization

The above formula is compact and easy to understand, but it's not the most
efficient way to evaluate $f(x)$ repeatedly. It's much better to expand the products,
collect terms, and rework as a polynomial in the following form:

$$
f(x) = \sum_{j=0}^{n-1} K_j x^j
$$

The C++ code in this project does exactly that. It consists of a class template
called `Interpolator`. After creating an instance of `Interpolator`, you can call
its `insert` method to insert as many points $(x_j, y_j)$ as you want.

Then call its `polynomial` method to return an object of type `Polynomial`.
The `Polynomial` class represents a polynomial in terms of a single independent
variable $x$. It holds the polynomial's numeric coefficients in ascending order
of the power of $x$. For example, the polynomial

$$
17 x^3 - 4 x^2 + 7 x - 11
$$

Is represented by the list `[-11, 7, -4, 17]`. Note that the coefficients go
from lowest order $x^0$ to the highest order $x^3$.

The `Polynomial` instance overloads the parentheses operator `()` to allow
treating it syntactically like a function of $x$ in your C++ code.

# Sample code

Here is a [sample C++ program](demo.cpp) that finds the cubic function passing through four points.
Once found, the program prints the polynomial and evaluates it at a few points.

Here is [the output from the demo program](correct/demo.txt).
