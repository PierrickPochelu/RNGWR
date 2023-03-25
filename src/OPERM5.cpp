#include <cstdint>
#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <iomanip>

#include "OPERM5.h"

using namespace std;

double gamma_p(double a, double x)
{
    double result, term, prev;
    int n;

    result = term = std::pow(x, a) * std::exp(-x) / std::tgamma(a + 1.0);

    for (n = 1; n < 1000; n++)
    {
        term *= x / (a + n);
        prev = result;
        result += term;
        if (result == prev)
            return result;
    }

    return result;
}


double chi2_to_pvalue(double chi2, int degrees_of_freedom) {
    // Compute the cumulative distribution function (CDF) of the chi-squared distribution.
    // The CDF gives the probability of observing a value less than or equal to the chi2 value.
    double cdf = gamma_p(static_cast<double>(degrees_of_freedom) / 2.0, chi2 / 2.0);

    // The p-value is the probability of observing a value equal to or more extreme than the chi2 value.
    // For a one-sided test, this is simply 1 - CDF.
    double p_value = 1.0 - cdf;

    // For a two-sided test, we need to compute the CDF at both tails of the distribution and add them together.
    // This assumes that the chi2 value is positive; if it is negative, we need to reverse the order of the tails.
    if (p_value > 0.5) {
        p_value = 2.0 * (1.0 - p_value);
    } else {
        p_value = 2.0 * p_value;
    }

    return p_value;
}

float OPERM5Test(uint64_t *rnd, uint64_t n)
{
    int permCount[120] = {};

    for (uint64_t i = 0; i < n - 5; i++)
    {
        uint64_t t[5];
        std::memcpy(t, rnd + i, 5 * sizeof(uint64_t));

        // compute which permutation is it
        int pIndex = 0;
        int pMul = 1;
        for (uint32_t k = 5; k > 1; k--)
        {
            uint64_t min = t[0];
            uint32_t index = 0;

            // argmin
            for (uint32_t j = 1; j < k; j++)
            {
                uint64_t r = t[j];
                if (r < min)
                {
                    min = r;
                    index = j;
                }
            }

            pIndex += pMul * index;
            pMul *= k;

            // remove minimum found
            for (uint32_t j = 1; j < k; j++)
            {
                if (j > index)
                    t[j - 1] = t[j];
            }
        }
        permCount[pIndex]++;
    }

    double chi2 = 0.f;
    for (uint32_t i = 0; i < 120; i++)
    {
        double d = permCount[i] - (n - 5) / 120.f;
        chi2 += (d * d) / ((n - 5) / 120.f);
    }


    double pvalue = chi2_to_pvalue(chi2, 119);
    return pvalue;
}

double chi_squared_test(const std::vector<double> &hist,
                        const std::vector<double> &theory_hist,
                        uint64_t NBINS)
{
    uint64_t nb_samples = std::accumulate(hist.begin(), hist.end(), 0);
    double chi_squared = 0;
    for (size_t i = 0; i < NBINS; ++i)
    {
        chi_squared += std::pow(hist[i] - theory_hist[i], 2.) / theory_hist[i];
    }
    return chi_squared;
}

// Function to construct histogram
std::vector<double> constructHistogram(std::vector<uint64_t> samples, uint64_t min_val, uint64_t max_val, int num_bins)
{
    // For arithmetic precision, we first compute a histogram containing the number of occurences (integer)
    std::vector<uint64_t> histogram_int(num_bins, 0);
    double bin_width = ((double)max_val - (double)min_val) / (double)num_bins;
    for (int i = 0; i < samples.size(); i++)
    {
        double bin_index_double = ((double)samples[i] - (double)min_val) / (double)bin_width;
        int bin_index = (int)bin_index_double;
        histogram_int[bin_index]++;
    }

    // Normalize the histogram
    std::vector<double> histogram(num_bins, 0);
    for (int i = 0; i < num_bins; i++)
    {
        histogram[i] = (double)histogram_int[i] / (double)samples.size();
    }
    return histogram;
}

double uniform(std::vector<uint64_t> samples, uint64_t min_val, uint64_t max_val)
{
    const uint64_t NBINS = 128;

    std::vector<double> theory_hist(NBINS, 1. / (double)NBINS);
    std::vector<double> hist = constructHistogram(samples, min_val, max_val, NBINS);

    double chi_squared = chi_squared_test(hist, theory_hist, NBINS);
    double p_value = 1. - erf(chi_squared / std::sqrt(2 * NBINS - 2));
    return p_value;
}