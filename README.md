My first project learning c++. This is an expression calculator that uses Boost Multiprecision MPFR backend to handle very long number calculations.
The given example is set to 999 internal bits, but can be set to 999999 or more which is definitely overkill and takes up resources.
The calculator is set to support nested expressions and handles most of errors.
For now, some single argument functions and constants like pi, e, phi are implemented.

I'm working on the project in a windows enviroment, so the windows api is included a little for test builds.

The project depends on Boost, GMP and MPFR libraries.

Due to the fact that the output numbers are very huge it may be necessary to set a larger console history.
