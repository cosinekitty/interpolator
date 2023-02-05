# Interpolator
This is a polynomial interpolator implemented as a single-header C++ class template.

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
>>> def g(x):
...     return (17/2)*x*x - (69/2)*x + 33
... 
>>> g(1)
7.0
>>> g(2)
-2.0
>>> g(3)
6.0
```

