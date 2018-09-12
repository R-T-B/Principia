﻿#pragma once

#include "physics/geopotential.hpp"

#include <cmath>

#include "numerics/legendre.hpp"
#include "numerics/polynomial_evaluators.hpp"
#include "quantities/elementary_functions.hpp"
#include "quantities/quantities.hpp"
#include "quantities/si.hpp"

namespace principia {
namespace physics {
namespace internal_geopotential {

using numerics::HornerEvaluator;
using numerics::LegendrePolynomial;
using geometry::InnerProduct;
using quantities::Cos;
using quantities::Inverse;
using quantities::Length;
using quantities::Pow;
using quantities::Sqrt;
using quantities::Square;
using quantities::Sin;
using quantities::SIUnit;

template<typename Frame>
Geopotential<Frame>::Geopotential(not_null<OblateBody<Frame> const*> body)
    : body_(body) {}

template<typename Frame>
Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
Geopotential<Frame>::SphericalHarmonicsAcceleration(
    Instant const& t,
    Displacement<Frame> const& r,
    Square<Length> const& r²,
    Exponentiation<Length, -3> const& one_over_r³) const {
  Exponentiation<Length, -2> const one_over_r² = 1 / r²;
  UnitVector const& axis = body_->polar_axis();
  Vector<Quotient<Acceleration, GravitationalParameter>, Frame> acceleration =
      Degree2ZonalAcceleration(axis, r, one_over_r², one_over_r³);
  if (body_->has_c22() || body_->has_s22()) {
    auto const from_surface_frame =
        body_->template FromSurfaceFrame<SurfaceFrame>(t);
    UnitVector const reference = from_surface_frame(x_);
    UnitVector const bireference = from_surface_frame(y_);
    acceleration +=
        Degree2SectoralAcceleration(
            reference, bireference, r, one_over_r², one_over_r³);
  }
  if (body_->has_j3()) {
    acceleration +=
        Degree3ZonalAcceleration(axis, r, r², one_over_r², one_over_r³);
  }
  return acceleration;
}

template<typename Frame>
template<int degree, int order>
struct Geopotential<Frame>::DegreeNOrderM {
  static Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
  Acceleration(OblateBody<Frame> const& body,
               UnitVector const& x,
               UnitVector const& y,
               UnitVector const& z,
               Displacement<Frame> const& r,
               Square<Length> const rx²_plus_ry²,
               Length const& rx,
               Length const& ry,
               Length const& rz,
               Length const& r_norm,
               Exponentiation<Length, -3> const& one_over_r³,
               Inverse<Length> const& radial_factor,
               Vector<Exponentiation<Length, -2>, Frame> const&
                   radial_factor_derivative,
               double const sin_β,
               double const cos_β,
               double const one_over_cos²_β,
               Angle const& λ) {
    if constexpr (degree == 2 && order == 1) {
      return {};
    } else {

      constexpr int n = degree;
      constexpr int m = order;
      static_assert(0 <= m && m <= n);
      static double const normalization_factor =
          LegendreNormalizationFactor(n, m);

      //TODO(phl):lots of stuff here that could be moved out.
      double const latitudinal_factor =
          Pow<m>(cos_β) * LegendrePolynomialDerivative<n, m>(sin_β);
      double latitudinal_polynomials = 0.0;
      if constexpr (m < n) {
        latitudinal_polynomials +=
            Pow<m + 1>(cos_β) * LegendrePolynomialDerivative<n, m + 1>(sin_β);
      }
      if constexpr (0 < m) {
        latitudinal_polynomials -=
            Pow<m - 1>(cos_β) * m * sin_β *
            LegendrePolynomialDerivative<n, m>(sin_β);
      }
      LOG(ERROR)<<sin_β<<" "<<cos_β<<" "<<λ<<" "<<Cos(λ)<<" "<<Sin(λ);
      Vector<Inverse<Length>, Frame> const latitudinal_factor_derivative =
          latitudinal_polynomials *
          (-sin_β * Cos(λ) * x - sin_β * Sin(λ) * y + cos_β * z) /
          r_norm;

      double const Cnm = body.cos()[n][m];
      double const Snm = body.sin()[n][m];

      Angle const mλ = m * λ;
      double const sin_mλ = Sin(mλ);
      double const cos_mλ = Cos(mλ);
      double const longitudinal_factor = Cnm * cos_mλ + Snm * sin_mλ ;
      Vector<Inverse<Length>, Frame> const longitudinal_factor_derivative =
          m * (Snm * cos_mλ - Cnm * sin_mλ) * (rx * y - ry * x) / rx²_plus_ry²;

      LOG(ERROR)<<radial_factor<<" "<<radial_factor_derivative;
      LOG(ERROR)<<latitudinal_factor<<" "<<latitudinal_factor_derivative;
      LOG(ERROR)<<longitudinal_factor<<" "<<longitudinal_factor_derivative;

      return normalization_factor *
             (radial_factor_derivative * latitudinal_factor *
                  longitudinal_factor +
              radial_factor * latitudinal_factor_derivative *
                  longitudinal_factor +
              radial_factor * latitudinal_factor *
                  longitudinal_factor_derivative);
    }
  }
};

template<typename Frame>
template<int degree, int... orders>
struct Geopotential<Frame>::
DegreeNAllOrders<degree, std::integer_sequence<int, orders...>> {
  static Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
  Acceleration(OblateBody<Frame> const& body,
               UnitVector const& x,
               UnitVector const& y,
               UnitVector const& z,
               Length const& reference_radius,
               Displacement<Frame> const& r,
               Square<Length> const& r²,
               Square<Length> const rx²_plus_ry²,
               Length const& rx,
               Length const& ry,
               Length const& rz,
               Length const& r_norm,
               Exponentiation<Length, -3> const& one_over_r³,
               double const sin_β,
               double const cos_β,
               double const one_over_cos²_β,
               Angle const& λ) {
    if constexpr (degree < 2) {
      return {};
    } else {
      constexpr int n = degree;
      Inverse<Length> const radial_factor =
          Pow<n>(reference_radius / r_norm) / r_norm;
      Vector<Exponentiation<Length, -2>, Frame> const radial_factor_derivative =
          -(n + 1) * r * radial_factor / r²;
      return (DegreeNOrderM<degree, orders>::Acceleration(
                  body,
                  x, y, z,
                  r, rx²_plus_ry², rx, ry, rz, r_norm, one_over_r³,
                  radial_factor, radial_factor_derivative,
                  sin_β, cos_β, one_over_cos²_β,
                  λ) + ...);
    }
  }
};

template<typename Frame>
template<int... degrees>
struct Geopotential<Frame>::AllDegrees<std::integer_sequence<int, degrees...>> {
  static Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
  Acceleration(OblateBody<Frame> const& body,
               Instant const& t,
               Displacement<Frame> const& r,
               Square<Length> const& r²,
               Exponentiation<Length, -3> const& one_over_r³) {
    Length const& reference_radius = body.reference_radius();
    auto const from_surface_frame = body.FromSurfaceFrame<SurfaceFrame>(t);
    UnitVector const x = from_surface_frame(x_);
    UnitVector const y = from_surface_frame(y_);
    UnitVector const& z = body.polar_axis();

    LOG(ERROR)<<r<<" "<<x<<" "<<y<<" "<<z<<" "<<t;

    Length const rx = InnerProduct(r, x);
    Length const ry = InnerProduct(r, y);
    Length const rz = InnerProduct(r, z);

    Length const r_norm = r.Norm();
    Square<Length> const rx²_plus_ry² = r² - rz * rz;
    double const sin_β = rz / r_norm;
    double const cos_β = Sqrt(rx²_plus_ry²) / r_norm;
    double const one_over_cos²_β = r² / rx²_plus_ry²;
    Angle const λ = SIUnit<Angle>() *
                    std::atan2(ry / SIUnit<Length>(), rx / SIUnit<Length>());
    LOG(ERROR)<<rx<<" "<<ry<<" "<<λ;

    return (DegreeNAllOrders<degrees,
                             std::make_integer_sequence<int, degrees + 1>>::
            Acceleration(body,
                         x, y, z,
                         reference_radius,
                         r, r², rx²_plus_ry², rx, ry, rz, r_norm, one_over_r³,
                         sin_β, cos_β, one_over_cos²_β,
                         λ) +
            ...);
  }
};

#define PRINCIPIA_CASE_SPHERICAL_HARMONICS(d)                                  \
  case (d):                                                                    \
    return AllDegrees<std::make_integer_sequence<int, (d + 1)>>::Acceleration( \
        *body_, t, r, r², one_over_r³)

template<typename Frame>
Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
Geopotential<Frame>::FullSphericalHarmonicsAcceleration(
    Instant const& t,
    Displacement<Frame> const& r,
    Square<Length> const& r²,
    Exponentiation<Length, -3> const& one_over_r³) const {
  switch (body_->geopotential_degree()) {
    PRINCIPIA_CASE_SPHERICAL_HARMONICS(2);
    PRINCIPIA_CASE_SPHERICAL_HARMONICS(3);
    PRINCIPIA_CASE_SPHERICAL_HARMONICS(4);
    PRINCIPIA_CASE_SPHERICAL_HARMONICS(5);
    default:
      LOG(FATAL) << "Unexpected degree " << body_->geopotential_degree();
      base::noreturn();
  }
}

#undef PRINCIPIA_CASE_SPHERICAL_HARMONICS

template<typename Frame>
template<int degree, int order>
double Geopotential<Frame>::LegendrePolynomialDerivative(
    double const argument) {
  static auto const pn = LegendrePolynomial<degree, HornerEvaluator>();
  static auto const dmpn = pn.Derivative<order>();
  return dmpn.Evaluate(argument);
}

template<typename Frame>
Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
Geopotential<Frame>::Degree2ZonalAcceleration(
    UnitVector const& axis,
    Displacement<Frame> const& r,
    Exponentiation<Length, -2> const& one_over_r²,
    Exponentiation<Length, -3> const& one_over_r³) const {
  Length const r_axis_projection = InnerProduct(axis, r);
  auto const j2_over_r⁵ = body_->j2_over_μ() * one_over_r³ * one_over_r²;
  Vector<Quotient<Acceleration, GravitationalParameter>, Frame> const
      axis_effect = -3 * j2_over_r⁵ * r_axis_projection * axis;
  Vector<Quotient<Acceleration, GravitationalParameter>, Frame> const
      radial_effect =
          j2_over_r⁵ *
          (-1.5 + 7.5 * r_axis_projection * r_axis_projection * one_over_r²) *
          r;
  return axis_effect + radial_effect;
}

template<typename Frame>
Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
Geopotential<Frame>::Degree2SectoralAcceleration(
    UnitVector const& reference,
    UnitVector const& bireference,
    Displacement<Frame> const& r,
    Exponentiation<Length, -2> const& one_over_r²,
    Exponentiation<Length, -3> const& one_over_r³) const {
  Length const r_reference_projection = InnerProduct(reference, r);
  Length const r_bireference_projection = InnerProduct(bireference, r);
  auto const c22_over_r⁵ = body_->c22_over_μ() * one_over_r³ * one_over_r²;
  auto const s22_over_r⁵ = body_->s22_over_μ() * one_over_r³ * one_over_r²;
  Vector<Quotient<Acceleration, GravitationalParameter>, Frame> const
      c22_effect = 6 * c22_over_r⁵ *
                   (-r_bireference_projection * bireference +
                    r_reference_projection * reference +
                    2.5 *
                        (r_bireference_projection * r_bireference_projection -
                         r_reference_projection * r_reference_projection) *
                        one_over_r² * r);
  Vector<Quotient<Acceleration, GravitationalParameter>, Frame> const
      s22_effect = 6 * s22_over_r⁵ *
                   (r_reference_projection * bireference +
                    r_bireference_projection * reference -
                    5 * r_reference_projection * r_bireference_projection *
                        one_over_r² * r);
  return c22_effect + s22_effect;
}

template<typename Frame>
Vector<Quotient<Acceleration, GravitationalParameter>, Frame>
Geopotential<Frame>::Degree3ZonalAcceleration(
    UnitVector const& axis,
    Displacement<Frame> const& r,
    Square<Length> const& r²,
    Exponentiation<Length, -2> const& one_over_r²,
    Exponentiation<Length, -3> const& one_over_r³) const {
  // TODO(phl): Factor the projections across accelerations?
  Length const r_axis_projection = InnerProduct(axis, r);
  Square<Length> const r_axis_projection² =
      r_axis_projection * r_axis_projection;
  auto const j3_over_r⁷ =
      body_->j3_over_μ() * one_over_r³ * one_over_r²* one_over_r²;
  Vector<Quotient<Acceleration, GravitationalParameter>, Frame> const
      axis_effect = 1.5 * j3_over_r⁷ * (r² - 5 * r_axis_projection²) * axis;
  Vector<Quotient<Acceleration, GravitationalParameter>, Frame> const
      radial_effect = j3_over_r⁷ * r_axis_projection *
                      (-7.5 + 17.5 * r_axis_projection² * one_over_r²) * r;
  return axis_effect + radial_effect;
}

template<typename Frame>
const Vector<double, typename Geopotential<Frame>::SurfaceFrame>
    Geopotential<Frame>::x_({1, 0, 0});
template<typename Frame>
const Vector<double, typename Geopotential<Frame>::SurfaceFrame>
    Geopotential<Frame>::y_({0, 1, 0});

}  // namespace internal_geopotential
}  // namespace physics
}  // namespace principia
