﻿
#pragma once

#include "numerics/frequency_analysis.hpp"

#include <functional>

#include "numerics/root_finders.hpp"
#include "quantities/si.hpp"

namespace principia {
namespace numerics {
namespace frequency_analysis {
namespace internal_frequency_analysis {

using quantities::Square;
namespace si = quantities::si;

// A helper struct for generating the Poisson series tⁿ sin ω t and tⁿ cos ω t.
template<typename Series, int n>
struct SeriesGenerator {
  // The series 1, irrespective of n.
  static Series One(Instant const& origin);
  // The series tⁿ sin ω t.
  static Series Sin(AngularFrequency const& ω, Instant const& origin);
  // The series tⁿ cos ω t.
  static Series Cos(AngularFrequency const& ω, Instant const& origin);

private:
  // The polynomial tⁿ.
  static typename Series::Polynomial PolynomialUnit(Instant const& origin);
  template<typename S, int>
  friend struct SeriesGenerator;
};

// A helper struct for generating the Кудрявцев basis, i.e., functions of the
// form tⁿ sin ω t and tⁿ cos ω t properly ordered.
template<typename Series,
         typename = std::make_index_sequence<Series::degree>>
struct BasisGenerator;

template<typename Series, std::size_t... indices>
struct BasisGenerator<Series, std::index_sequence<indices...>> {
  static std::array<Series, 2 * Series::degree + 1> Basis(
      AngularFrequency const& ω,
      Instant const& origin);
};


template<typename Series, int n>
Series SeriesGenerator<Series, n>::One(Instant const& origin) {
  return Series(SeriesGenerator<Series, 0>::PolynomialUnit(origin), {{}});
}

template<typename Series, int n>
Series SeriesGenerator<Series, n>::Sin(AngularFrequency const& ω,
                                       Instant const& origin) {
  typename Series::Polynomial::Coefficients const zeros;
  typename Series::Polynomial const zero{zeros, origin};
  return Series(zero,
                {{ω,
                  {/*sin=*/PolynomialUnit(origin),
                   /*cos=*/zero}}});
}

template<typename Series, int n>
Series SeriesGenerator<Series, n>::Cos(AngularFrequency const& ω,
                                       Instant const& origin) {
  typename Series::Polynomial::Coefficients const zeros;
  typename Series::Polynomial const zero{zeros, origin};
  return Series(zero,
                {{ω,
                  {/*sin=*/zero,
                   /*cos=*/PolynomialUnit(origin)}}});
}

template<typename Series, int n>
typename Series::Polynomial SeriesGenerator<Series, n>::PolynomialUnit(
    Instant const& origin) {
  typename Series::Polynomial::Coefficients coefficients;
  std::get<n>(coefficients) = si::Unit<
      std::tuple_element_t<n, typename Series::Polynomial::Coefficients>>;
  return Series::Polynomial(coefficients, origin);
}

template<typename Series, std::size_t... indices>
std::array<Series, 2 * Series::degree + 1>
BasisGenerator<Series, std::index_sequence<indices...>>::Basis(
    AngularFrequency const& ω,
    Instant const& origin) {
  // This has the elements {1, t Sin(ωt), t² Sin(ωt), ... t Cos(ωt)...}
  // which is not the order we want (we want lower-degree polynomials first).
  std::array<Series, 2 * Series::degree + 1> all_series = {
      SeriesGenerator<Series, 0>::One(origin),
      SeriesGenerator<Series, indices>::Sin(ω, origin)...,
      SeriesGenerator<Series, indices>::Cos(ω, origin)...};

  // Order all_series by repeatedly swapping its elements.
  if (Series::degree >= 2) {
    // The index of this array is the current index of a series in all_series.
    // The value is the index of the final resting place of that series in
    // all_series.  The elements at indices 0, 1 and 2 * Series::degree are
    // unused.
    std::array<int, 2 * Series::degree + 1> permutation;
    for (int i = 2; i < 2 * Series::degree; ++i) {
      permutation[i] =
          i <= Series::degree ? 2 * i - 1 : 2 * (i - Series::degree);
    }
    for (int i = 2; i < 2 * Series::degree;) {
      // Swap the series currently at index i to its final resting place.
      // Iterate until the series at index i is at its final resting place
      // (i.e., after we have executed an entire cycle of the permutation).
      // Then move to the next series.
      if (i == permutation[i]) {
        ++i;
      } else {
        int const j = permutation[i];
        std::swap(all_series[i], all_series[j]);
        std::swap(permutation[i], permutation[j]);
      }
    }
  }
  return all_series;
}


template<typename Function,
         typename RValue, int rdegree_, int wdegree_,
         template<typename, typename, int> class Evaluator>
AngularFrequency PreciseMode(
    Interval<AngularFrequency> const& fft_mode,
    Function const& function,
    PoissonSeries<double, wdegree_, Evaluator> const& weight,
    DotProduct<Function, RValue, rdegree_, wdegree_, Evaluator> const& dot) {
  using DotResult =
      Primitive<Product<std::invoke_result_t<Function, Instant>, RValue>, Time>;
  using Degree0 = PoissonSeries<double, 0, Evaluator>;

  auto amplitude = [&dot, &function, &weight](AngularFrequency const& ω) {
    Instant const& t0 = weight.origin();
    Degree0 const sin(typename Degree0::Polynomial({0}, t0),
                      {{ω,
                        {/*sin=*/typename Degree0::Polynomial({1}, t0),
                         /*cos=*/typename Degree0::Polynomial({0}, t0)}}});
    Degree0 const cos(typename Degree0::Polynomial({0}, t0),
                      {{ω,
                        {/*sin=*/typename Degree0::Polynomial({0}, t0),
                         /*cos=*/typename Degree0::Polynomial({1}, t0)}}});
    auto const sin_amplitude = dot(function, sin, weight);
    auto const cos_amplitude = dot(function, cos, weight);
    return sin_amplitude * sin_amplitude + cos_amplitude * cos_amplitude;
  };

  return GoldenSectionSearch(amplitude,
                             fft_mode.min,
                             fft_mode.max,
                             std::greater<Square<DotResult>>());
}

template<typename Function,
         typename RValue, int rdegree_, int wdegree_,
         template<typename, typename, int> class Evaluator>
PoissonSeries<std::invoke_result_t<Function, Instant>, rdegree_, Evaluator>
Projection(
    AngularFrequency const& ω,
    Function const& function,
    PoissonSeries<double, wdegree_, Evaluator> const& weight,
    DotProduct<Function, RValue, rdegree_, wdegree_, Evaluator> const& dot) {
  using Value = std::invoke_result_t<Function, Instant>;
  using Series = PoissonSeries<Value, degree_, Evaluator>;
  Instant const& t0 = weight.origin();

  std::vector<Series> basis;
  typename Series::Polynomial::Coefficients const zeros;
  basis.emplace_back(typename Series::Polynomial({1}, t0), {{}});
  typename Series::Polynomial const zero({0}, t0);
  for (int d = 1; d <= degree_; ++d) {
    basis.emplace_back(zero,
                      {{ω,
                        {/*sin=*/typename Series::Polynomial({1}, t0),
                         /*cos=*/typename Series::Polynomial({0}, t0)}}});
    basis.emplace_back(zero,
                      {{ω,
                        {/*sin=*/typename Series::Polynomial({1}, t0),
                         /*cos=*/typename Series::Polynomial({0}, t0)}}});
  }
}

}  // namespace internal_frequency_analysis
}  // namespace frequency_analysis
}  // namespace numerics
}  // namespace principia
