﻿
#include "ksp_plugin/vessel.hpp"

#include <limits>
#include <set>
#include <vector>

#include "absl/status/status.h"
#include "astronomy/epoch.hpp"
#include "base/not_null.hpp"
#include "geometry/barycentre_calculator.hpp"
#include "geometry/named_quantities.hpp"
#include "geometry/r3x3_matrix.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ksp_plugin/celestial.hpp"
#include "ksp_plugin/frames.hpp"
#include "ksp_plugin/integrators.hpp"
#include "physics/degrees_of_freedom.hpp"
#include "physics/discrete_trajectory.hpp"
#include "physics/massive_body.hpp"
#include "physics/rigid_motion.hpp"
#include "physics/rotating_body.hpp"
#include "physics/mock_ephemeris.hpp"
#include "quantities/named_quantities.hpp"
#include "quantities/quantities.hpp"
#include "quantities/si.hpp"
#include "testing_utilities/almost_equals.hpp"
#include "testing_utilities/componentwise.hpp"
#include "testing_utilities/discrete_trajectory_factories.hpp"
#include "testing_utilities/matchers.hpp"

namespace principia {
namespace ksp_plugin {

using base::not_null;
using base::make_not_null_unique;
using geometry::Barycentre;
using geometry::Displacement;
using geometry::InertiaTensor;
using geometry::Instant;
using geometry::Position;
using geometry::R3x3Matrix;
using geometry::Velocity;
using physics::DegreesOfFreedom;
using physics::DiscreteTrajectory;
using physics::MassiveBody;
using physics::MockEphemeris;
using physics::RigidMotion;
using physics::RotatingBody;
using quantities::Mass;
using quantities::MomentOfInertia;
using quantities::si::Degree;
using quantities::si::Kilogram;
using quantities::si::Metre;
using quantities::si::Radian;
using quantities::si::Second;
using testing_utilities::AlmostEquals;
using testing_utilities::Componentwise;
using testing_utilities::EqualsProto;
using testing_utilities::AppendTrajectoryTimeline;
using testing_utilities::NewLinearTrajectoryTimeline;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::MockFunction;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;

class VesselTest : public testing::Test {
 protected:
  VesselTest()
      : body_(MassiveBody::Parameters(1 * Kilogram),
              RotatingBody<Barycentric>::Parameters(
                  /*mean_radius=*/1 * Metre,
                  /*reference_angle=*/0 * Degree,
                  /*reference_instant=*/t0_,
                  /*angular_frequency=*/1 * Radian / Second,
                  /*right_ascension_of_pole=*/0 * Degree,
                  /*declination_of_pole=*/90 * Degree)),
        celestial_(&body_),
        inertia_tensor1_(MakeWaterSphereInertiaTensor(mass1_)),
        inertia_tensor2_(MakeWaterSphereInertiaTensor(mass2_)),
        vessel_("123",
                "vessel",
                &celestial_,
                &ephemeris_,
                DefaultPredictionParameters()) {
    auto p1 = make_not_null_unique<Part>(
        part_id1_,
        "p1",
        mass1_,
        EccentricPart::origin,
        inertia_tensor1_,
        RigidMotion<EccentricPart, Barycentric>::MakeNonRotatingMotion(p1_dof_),
        /*deletion_callback=*/nullptr);
    auto p2 = make_not_null_unique<Part>(
        part_id2_,
        "p2",
        mass2_,
        EccentricPart::origin,
        inertia_tensor2_,
        RigidMotion<EccentricPart, Barycentric>::MakeNonRotatingMotion(p2_dof_),
        /*deletion_callback=*/nullptr);
    p1_ = p1.get();
    p2_ = p2.get();
    vessel_.AddPart(std::move(p1));
    vessel_.AddPart(std::move(p2));
  }

  MockEphemeris<Barycentric> ephemeris_;
  RotatingBody<Barycentric> const body_;
  Celestial const celestial_;
  PartId const part_id1_ = 111;
  PartId const part_id2_ = 222;
  Mass const mass1_ = 1 * Kilogram;
  Mass const mass2_ = 2 * Kilogram;
  InertiaTensor<RigidPart> inertia_tensor1_;
  InertiaTensor<RigidPart> inertia_tensor2_;

  DegreesOfFreedom<Barycentric> const p1_dof_ = DegreesOfFreedom<Barycentric>(
      Barycentric::origin +
          Displacement<Barycentric>({1 * Metre, 2 * Metre, 3 * Metre}),
      Velocity<Barycentric>(
          {10 * Metre / Second, 20 * Metre / Second, 30 * Metre / Second}));
  DegreesOfFreedom<Barycentric> const p2_dof_ = DegreesOfFreedom<Barycentric>(
      Barycentric::origin +
          Displacement<Barycentric>({6 * Metre, 5 * Metre, 4 * Metre}),
      Velocity<Barycentric>(
          {60 * Metre / Second, 50 * Metre / Second, 40 * Metre / Second}));

  Instant const t0_;
  Part* p1_;
  Part* p2_;
  Vessel vessel_;
};

TEST_F(VesselTest, Parent) {
  Celestial other_celestial(&body_);
  EXPECT_EQ(&celestial_, vessel_.parent());
  vessel_.set_parent(&other_celestial);
  EXPECT_EQ(&other_celestial, vessel_.parent());
}

TEST_F(VesselTest, KeepAndFreeParts) {
  std::set<PartId> remaining_part_ids;
  vessel_.ForAllParts([&remaining_part_ids](Part const& part) {
    remaining_part_ids.insert(part.part_id());
  });
  EXPECT_THAT(remaining_part_ids, ElementsAre(part_id1_, part_id2_));
  EXPECT_EQ(part_id1_, vessel_.part(part_id1_)->part_id());
  EXPECT_EQ(part_id2_, vessel_.part(part_id2_)->part_id());
  remaining_part_ids.clear();

  vessel_.KeepPart(part_id2_);
  vessel_.FreeParts();
  vessel_.ForAllParts([&remaining_part_ids](Part const& part) {
    remaining_part_ids.insert(part.part_id());
  });
  EXPECT_THAT(remaining_part_ids, ElementsAre(part_id2_));
  EXPECT_EQ(part_id2_, vessel_.part(part_id2_)->part_id());
}

TEST_F(VesselTest, PrepareHistory) {
  EXPECT_CALL(ephemeris_, t_max())
      .WillRepeatedly(Return(t0_ + 2 * Second));
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, astronomy::InfiniteFuture, _, _))
      .Times(AnyNumber());
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, t0_ + 2 * Second, _, _))
      .Times(AnyNumber());
  vessel_.PrepareHistory(t0_ + 1 * Second,
                         DefaultDownsamplingParameters());

  auto const expected_dof = Barycentre<DegreesOfFreedom<Barycentric>, Mass>(
      {p1_dof_, p2_dof_}, {mass1_, mass2_});

  EXPECT_EQ(1, vessel_.psychohistory()->size());
  EXPECT_EQ(t0_ + 1 * Second,
            vessel_.psychohistory()->back().time);
  EXPECT_THAT(
      vessel_.psychohistory()->back().degrees_of_freedom,
      Componentwise(AlmostEquals(expected_dof.position(), 0),
                    AlmostEquals(expected_dof.velocity(), 8)));
}

TEST_F(VesselTest, AdvanceTime) {
  EXPECT_CALL(ephemeris_, t_max())
      .WillRepeatedly(Return(t0_ + 2 * Second));
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, astronomy::InfiniteFuture, _, _))
      .Times(AnyNumber());
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, t0_ + 2 * Second, _, _))
      .Times(AnyNumber());
  vessel_.PrepareHistory(t0_,
                         DefaultDownsamplingParameters());

  AppendTrajectoryTimeline<Barycentric>(
      NewLinearTrajectoryTimeline<Barycentric>(p1_dof_,
                                               /*Δt=*/0.5 * Second,
                                               /*t0=*/t0_,
                                               /*t1=*/t0_ + 0.5 * Second,
                                               /*t2=*/t0_ + 1.5 * Second),
      [this](Instant const& time,
             DegreesOfFreedom<Barycentric> const& degrees_of_freedom) {
        p1_->AppendToHistory(time, degrees_of_freedom);
      });
  AppendTrajectoryTimeline<Barycentric>(
      NewLinearTrajectoryTimeline<Barycentric>(p2_dof_,
                                               /*Δt=*/0.5 * Second,
                                               /*t0=*/t0_,
                                               /*t1=*/t0_ + 0.5 * Second,
                                               /*t2=*/t0_ + 1.5 * Second),
      [this](Instant const& time,
             DegreesOfFreedom<Barycentric> const& degrees_of_freedom) {
        p2_->AppendToHistory(time, degrees_of_freedom);
      });

  vessel_.AdvanceTime();

  auto const expected_vessel_psychohistory = NewLinearTrajectoryTimeline(
      Barycentre<DegreesOfFreedom<Barycentric>, Mass>({p1_dof_, p2_dof_},
                                                      {mass1_, mass2_}),
      /*Δt=*/0.5 * Second,
      /*t1=*/t0_,
      /*t2=*/t0_ + 1.1 * Second);

  EXPECT_EQ(3, vessel_.history()->size() + vessel_.psychohistory()->size() - 1);
  auto it1 = vessel_.history()->begin();
  auto it2 = expected_vessel_psychohistory.begin();
  for (;
       it1 != vessel_.psychohistory()->end() &&
       it2 != expected_vessel_psychohistory.end();
       ++it1, ++it2) {
    EXPECT_EQ(it1->time, it2->time);
    EXPECT_THAT(
        it1->degrees_of_freedom,
        Componentwise(AlmostEquals(it2->degrees_of_freedom.position(), 0, 1),
                      AlmostEquals(it2->degrees_of_freedom.velocity(), 0, 8)));
  }
}

TEST_F(VesselTest, Prediction) {
  EXPECT_CALL(ephemeris_, t_min_locked())
      .WillRepeatedly(Return(t0_));
  EXPECT_CALL(ephemeris_, t_max())
      .WillRepeatedly(Return(t0_ + 2 * Second));

  // The call to fill the prognostication until t_max.
  auto const expected_vessel_prediction = NewLinearTrajectoryTimeline(
      Barycentre<DegreesOfFreedom<Barycentric>, Mass>({p1_dof_, p2_dof_},
                                                      {mass1_, mass2_}),
      /*Δt=*/0.5 * Second,
      /*t1=*/t0_,
      /*t2=*/t0_ + 2 * Second);
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, t0_ + 2 * Second, _, _))
      .WillOnce(DoAll(
          AppendPointsToDiscreteTrajectory(&expected_vessel_prediction),
          Return(absl::OkStatus())))
      .WillRepeatedly(Return(absl::OkStatus()));

  // The call to extend the exphemeris.  Irrelevant since we won't be looking at
  // these points.
  EXPECT_CALL(
      ephemeris_,
      FlowWithAdaptiveStep(_, _, astronomy::InfiniteFuture, _, _))
      .WillRepeatedly(Return(absl::OkStatus()));

  vessel_.PrepareHistory(t0_,
                         DefaultDownsamplingParameters());
  // Polling for the integration to happen.
  do {
    vessel_.RefreshPrediction(t0_ + 1 * Second);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
  } while (vessel_.prediction()->back().time == t0_);

  EXPECT_EQ(3, vessel_.prediction()->size());
  auto it1 = vessel_.prediction()->begin();
  auto it2 = expected_vessel_prediction.begin();
  for (;
       it1 != vessel_.prediction()->end() &&
       it2 != expected_vessel_prediction.end();
       ++it1, ++it2) {
    EXPECT_EQ(it1->time, it2->time);
    EXPECT_THAT(
        it1->degrees_of_freedom,
        Componentwise(AlmostEquals(it2->degrees_of_freedom.position(), 0, 0),
                      AlmostEquals(it2->degrees_of_freedom.velocity(), 0, 8)));
  }
}

TEST_F(VesselTest, PredictBeyondTheInfinite) {
  EXPECT_CALL(ephemeris_, t_min_locked())
      .WillRepeatedly(Return(t0_));
  EXPECT_CALL(ephemeris_, t_max())
      .WillRepeatedly(Return(t0_ + 5 * Second));

  // The call to fill the prognostication until t_max.
  auto const expected_vessel_prediction1 = NewLinearTrajectoryTimeline(
      Barycentre<DegreesOfFreedom<Barycentric>, Mass>({p1_dof_, p2_dof_},
                                                      {mass1_, mass2_}),
      /*Δt=*/0.5 * Second,
      /*t1=*/t0_,
      /*t2=*/t0_ + 5.5 * Second);
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, t0_ + 5 * Second, _, _))
      .WillOnce(DoAll(
          AppendPointsToDiscreteTrajectory(&expected_vessel_prediction1),
          Return(absl::OkStatus())))
      .WillRepeatedly(Return(absl::OkStatus()));

  // The call to extend the exphemeris by many points.
  auto const expected_vessel_prediction2 = NewLinearTrajectoryTimeline(
      Barycentre<DegreesOfFreedom<Barycentric>, Mass>({p1_dof_, p2_dof_},
                                                      {mass1_, mass2_}),
      /*Δt=*/0.5 * Second,
      /*t1=*/t0_ + 5.5 * Second,
      /*t2=*/t0_ + FlightPlan::max_ephemeris_steps_per_frame * Second);
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, astronomy::InfiniteFuture, _, _))
      .WillOnce(DoAll(
          AppendPointsToDiscreteTrajectory(&expected_vessel_prediction2),
          Return(absl::OkStatus())))
      .WillRepeatedly(Return(absl::OkStatus()));

  vessel_.PrepareHistory(t0_,
                         DefaultDownsamplingParameters());
  // Polling for the integration to happen.
  do {
    vessel_.RefreshPrediction();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
  } while (vessel_.prediction()->size() <
           expected_vessel_prediction1.size() +
               expected_vessel_prediction2.size());

  auto it = expected_vessel_prediction1.begin();
  for (auto const& [time, degrees_of_freedom] : *vessel_.prediction()) {
    EXPECT_EQ(time, it->time);
    EXPECT_THAT(
        degrees_of_freedom,
        Componentwise(AlmostEquals(it->degrees_of_freedom.position(), 0, 0),
                      AlmostEquals(it->degrees_of_freedom.velocity(), 0, 8)));
    if (it->time == t0_ + 5 * Second) {
      it = expected_vessel_prediction2.begin();
    } else {
      ++it;
    }
  }
}

TEST_F(VesselTest, FlightPlan) {
  EXPECT_CALL(ephemeris_, t_max())
      .WillRepeatedly(Return(t0_ + 2 * Second));
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, astronomy::InfiniteFuture, _, _))
      .Times(AnyNumber());
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, t0_ + 2 * Second, _, _))
      .Times(AnyNumber());
  std::vector<not_null<MassiveBody const*>> const bodies;
  ON_CALL(ephemeris_, bodies()).WillByDefault(ReturnRef(bodies));
  vessel_.PrepareHistory(t0_,
                         DefaultDownsamplingParameters());

  EXPECT_FALSE(vessel_.has_flight_plan());
  EXPECT_CALL(
      ephemeris_,
      FlowWithAdaptiveStep(_, _, t0_ + 3 * Second, _, _))
      .WillOnce(Return(absl::OkStatus()));
  vessel_.CreateFlightPlan(t0_ + 3.0 * Second,
                           10 * Kilogram,
                           DefaultPredictionParameters(),
                           DefaultBurnParameters());
  EXPECT_TRUE(vessel_.has_flight_plan());
  EXPECT_EQ(0, vessel_.flight_plan().number_of_manœuvres());
  EXPECT_EQ(1, vessel_.flight_plan().number_of_segments());
  vessel_.DeleteFlightPlan();
  EXPECT_FALSE(vessel_.has_flight_plan());
}

TEST_F(VesselTest, SerializationSuccess) {
  MockFunction<int(not_null<PileUp const*>)>
      serialization_index_for_pile_up;
  EXPECT_CALL(serialization_index_for_pile_up, Call(_)).Times(0);

  EXPECT_CALL(ephemeris_, t_max())
      .WillRepeatedly(Return(t0_ + 2 * Second));
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, astronomy::InfiniteFuture, _, _))
      .Times(AnyNumber());
  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, t0_ + 2 * Second, _, _))
      .Times(AnyNumber());
  vessel_.PrepareHistory(t0_,
                         DefaultDownsamplingParameters());

  EXPECT_CALL(ephemeris_,
              FlowWithAdaptiveStep(_, _, t0_ + 3 * Second, _, _))
      .WillRepeatedly(Return(absl::OkStatus()));

  std::vector<not_null<MassiveBody const*>> const bodies;
  ON_CALL(ephemeris_, bodies()).WillByDefault(ReturnRef(bodies));

  vessel_.CreateFlightPlan(t0_ + 3.0 * Second,
                           10 * Kilogram,
                           DefaultPredictionParameters(),
                           DefaultBurnParameters());

  serialization::Vessel message;
  vessel_.WriteToMessage(&message,
                         serialization_index_for_pile_up.AsStdFunction());
  EXPECT_TRUE(message.has_history());
  EXPECT_TRUE(message.has_flight_plan());

  EXPECT_CALL(ephemeris_, Prolong(_)).Times(2);
  auto const v = Vessel::ReadFromMessage(
      message, &celestial_, &ephemeris_, /*deletion_callback=*/nullptr);
  EXPECT_TRUE(v->has_flight_plan());

  serialization::Vessel second_message;
  v->WriteToMessage(&second_message,
                    serialization_index_for_pile_up.AsStdFunction());
  EXPECT_THAT(message, EqualsProto(second_message));
}

}  // namespace ksp_plugin
}  // namespace principia
