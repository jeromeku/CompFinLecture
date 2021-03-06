/*
    this is the code for the matrix product demonstration in the lecture,
    also in the curriculum, chapter 1
*/

#include "stdafx.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <string>

#include <stdlib.h>
#include <time.h>

#include "matrix.h"
/*
    matrix.h contains a simple, standard implementation of a matrix
    
    data is contained in an internal vector so it is contiguous in memory, 
    as customary
    
    operator[i] returns a pointer on the first element in row i, 
    so matrix[i][j] returns the element in row i, col j
    
    basic consistency with STL 
    (e.g. begin() and end() iterators) 
    are also implemented

    for more information, 
    see chapter 1 of the curriculum, 
    and section 2.3 for the complete implementation
*/

//  naive version of a matrix product, 
//  as seen in many libraries, 
//  including professional ones in investment banks
void matrixProductNaive(
    const matrix<double>& a, 
    const matrix<double>& b, 
    matrix<double>& c)
{
    const size_t rows = a.rows(), cols = b.cols(), n = a.cols();

    //  outermost loop on result rows
    for (size_t i = 0; i < rows; ++i)
    {
        const auto ai = a[i];
        auto ci = c[i];

        //  loop on result columns
        for (size_t j = 0; j < cols; ++j)
        {
            //  compute dot product
            double res = 0.0;
            for (size_t k = 0; k < n; ++k)
            {
                res += ai[k] * b[k][j];     
                //  note b[k][j] "jumps" through memory in the innermost loop 
                //  - cache inneficiency
            }   //  dot
            //  set result
            c[i][j] = res;
        }   //  columns
    }   //  rows
}

//  reorder loops to avoid cache inneficiency 
//  - same algorithm and calculations otherwise
void matrixProductSmartNoVec(
    const matrix<double>& a, 
    const matrix<double>& b, 
    matrix<double>& c)
{
    const size_t rows = a.rows(), cols = b.cols(), n = a.cols();

    //  zero result first
    for (size_t i = 0; i < rows; ++i)
    {
        auto ci = c[i];
        for (size_t j = 0; j < cols; ++j)
        {
            ci[j] = 0;
        }
    }

    //  loop on result rows as before
    for (size_t i = 0; i < rows; ++i)
    {
        const auto ai = a[i];
        auto ci = c[i];

        //  then loop not on result columns but on dot product
        for (size_t k = 0; k < n; ++k)
        {
            const auto bk = b[k];
            const auto aik = ai[k]; 
            //  we still jump when reading memory, 
            //  but not in the innermost loop

            //  and finally over columns in innermost loop
#pragma loop(no_vector)     
//  no vectorization to isolate impact of cache alone
            for (size_t j = 0; j < cols; ++j)
            {
                ci[j] += aik * bk[j];   //  no more jumping through memory
            }   //  columns
        }   //  dot
    }   //  rows
}

//  same but with vectorization
void matrixProductSmartVec(
    const matrix<double>& a, 
    const matrix<double>& b, 
    matrix<double>& c)
{
    const size_t rows = a.rows(), cols = b.cols(), n = a.cols();

    for (size_t i = 0; i < rows; ++i)
    {
        auto ci = c[i];
        for (size_t j = 0; j < cols; ++j)
        {
            ci[j] = 0;
        }
    }

    for (size_t i = 0; i < rows; ++i)
    {
        const auto ai = a[i];
        auto ci = c[i];

        for (size_t k = 0; k < n; ++k)
        {
            const auto bk = b[k];
            const auto aik = ai[k];

            //  the only difference is the absence of pragma: 
            //  the compiler is free to vectorize
            for (size_t j = 0; j < cols; ++j)
            {
                ci[j] += aik * bk[j];
            }
        }
    }
}

//  same with multi-threading over outermost loop
void matrixProductSmartParallel(
    const matrix<double>& a, 
    const matrix<double>& b, 
    matrix<double>& c)
{
    const size_t rows = a.rows(), cols = b.cols(), n = a.cols();

    for (int i = 0; i < rows; ++i)
    {
        auto ci = c[i];
        for (size_t j = 0; j < cols; ++j)
        {
            ci[j] = 0;
        }
    }

    //  only difference is this openMP pragma, 
    //  tells the compiler to send chunks of the loop 
    //  across threads executing on different CPUs
    //  note the extreme simplicity of this technique

    //  note: openMP support must be enabled in 
    //  project properties C/C++/Language

#pragma omp parallel for 
    for (int i = 0; i < rows; ++i)
    {
        const auto ai = a[i];
        auto ci = c[i];

        for (size_t k = 0; k < n; ++k)
        {
            const auto bk = b[k];
            const auto aik = ai[k];

            for (size_t j = 0; j < cols; ++j)
            {
                ci[j] += aik * bk[j];
            }
        }
    }
}

void main()
{
    const size_t na = 1000, ma = 1000, nb = 1000, mb = 1000;

    //  allocate
    matrix<double> a(na, ma), b(nb, mb), 
        c1(na, mb), c2(na, mb), c3(na, mb), c4(na, mb);

    //  randomly fill a and b
    srand(12345);
    for (size_t i = 0; i < na; ++i)
    {
        auto ai = a[i];

        for (size_t j = 0; j < ma; ++j)
        {
            ai[j] = double(rand()) / RAND_MAX;
        }
    }

    for (size_t i = 0; i < nb; ++i)
    {
        auto bi = b[i];

        for (size_t j = 0; j < mb; ++j)
        {
            bi[j] = double(rand()) / RAND_MAX;
        }
    }

    //  calculate and time
    {
        cout << "Naive calculation starting" << endl;
        time_t t1 = clock();
        matrixProductNaive(a, b, c1);
        time_t t2 = clock();
        cout << "Naive calculation complete, MS = " << t2 - t1 << endl;
    }

    string bogus;
    cout << "Press c+enter to continue: ";
    cin >> bogus;

    {
        cout << "Smart calculation starting" << endl;
        time_t t1 = clock();
        matrixProductSmartNoVec(a, b, c2);
        time_t t2 = clock();
        cout << "Smart calculation complete, MS = " << t2 - t1 << endl;
    }

    cout << "Press c+enter to continue: ";
    cin >> bogus;

    {
        cout << "Vectorized calculation starting" << endl;
        time_t t1 = clock();
        matrixProductSmartVec(a, b, c3);
        time_t t2 = clock();
        cout << "Vectorized calculation complete, MS = " << t2 - t1 << endl;
    }

    cout << "Press c+enter to continue: ";
    cin >> bogus;

    {
        cout << "Parallel calculation starting" << endl;
        time_t t1 = clock();
        matrixProductSmartParallel(a, b, c4);
        time_t t2 = clock();
        cout << "Parallel calculation complete, MS = " << t2 - t1 << endl;
    }

    //  check
    cout << "Check = " << (c1 == c2) << " , " << (c2 == c3) << " , " << (c3 == c4) << endl;
    cout << "Check2 = " << c1[99][98] << " , " << c2[99][98] << " , " << c3[99][98] << " , " << c4[99][98] << endl;

}