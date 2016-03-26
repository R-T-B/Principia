﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace principia {
namespace ksp_plugin_adapter {

class FlightPlanner : WindowRenderer {
  public FlightPlanner(PrincipiaPluginAdapter adapter,
                       IntPtr plugin) : base(adapter) {
    adapter_ = adapter;
    plugin_ = plugin;
    window_rectangle_.x = UnityEngine.Screen.width / 2;
    window_rectangle_.y = UnityEngine.Screen.height / 3;
    final_time_ = new DifferentialSlider(
                label            : "Plan length",
                unit             : null,
                log10_lower_rate : Log10TimeLowerRate,
                log10_upper_rate : Log10TimeUpperRate,
                min_value        : 10,
                max_value        : double.PositiveInfinity,
                formatter        : value =>
                    FormatPositiveTimeSpan(
                        TimeSpan.FromSeconds(
                            value - plugin_.FlightPlanGetInitialTime(
                                        vessel_.id.ToString()))));

  }

  protected override void RenderWindow() {
    var old_skin = UnityEngine.GUI.skin;
    UnityEngine.GUI.skin = null;
    if (show_planner_) {
      window_rectangle_ = UnityEngine.GUILayout.Window(
                              id         : this.GetHashCode(),
                              screenRect : window_rectangle_,
                              func       : RenderPlanner,
                              text       : "Flight plan",
                              options    : UnityEngine.GUILayout.MinWidth(500));
    }
    UnityEngine.GUI.skin = old_skin;
  }

  public void RenderButton() {
    var old_skin = UnityEngine.GUI.skin;
    UnityEngine.GUI.skin = null;
    if (UnityEngine.GUILayout.Button("Flight plan...")) {
      show_planner_ = !show_planner_;
    }
    UnityEngine.GUI.skin = old_skin;
  }

  private void RenderPlanner(int window_id) {
    var old_skin = UnityEngine.GUI.skin;
    UnityEngine.GUI.skin = null;
    UnityEngine.GUILayout.BeginVertical();

    if (vessel_ == null || vessel_ != FlightGlobals.ActiveVessel ||
        !plugin_.HasVessel(vessel_.id.ToString())) {
      Reset();
    }

    if (vessel_ != null) {
      string vessel_guid = vessel_.id.ToString();
      if (burn_editors_ == null) {
        if (plugin_.HasVessel(vessel_guid)) {
          if (plugin_.FlightPlanExists(vessel_guid)) {
            burn_editors_ = new List<BurnEditor>();
            for (int i = 0;
                 i < plugin_.FlightPlanNumberOfManoeuvres(vessel_guid);
                 ++i) {
              // Dummy initial time, we call |Reset| immediately afterwards.
              final_time_.value = plugin_.FlightPlanGetFinalTime(vessel_guid);
              burn_editors_.Add(
                  new BurnEditor(adapter_, plugin_, vessel_, initial_time : 0));
              burn_editors_.Last().Reset(
                  plugin_.FlightPlanGetManoeuvre(vessel_guid, i));
            }
          } else {
            if (UnityEngine.GUILayout.Button("Create flight plan")) {
              plugin_.FlightPlanCreate(vessel_guid,
                                       plugin_.CurrentTime() + 1000,
                                       vessel_.GetTotalMass());
              final_time_.value = plugin_.FlightPlanGetFinalTime(vessel_guid);
              Shrink();
            }
          }
        }
      } else {
        if (final_time_.Render(enabled: true)) {
          plugin_.FlightPlanSetFinalTime(vessel_guid, final_time_.value);
          final_time_.value = plugin_.FlightPlanGetFinalTime(vessel_guid);
        }

        AdaptiveStepParameters parameters =
            plugin_.FlightPlanGetAdaptiveStepParameters(vessel_guid);
        UnityEngine.GUILayout.BeginHorizontal();
        UnityEngine.GUILayout.Label("Maximal step count",
                                    UnityEngine.GUILayout.Width(150));
        if (parameters.max_steps <= 100) {
          UnityEngine.GUILayout.Button("min");
        } else if (UnityEngine.GUILayout.Button("-")) {
          parameters.max_steps /= 10;
          plugin_.FlightPlanSetAdaptiveStepParameters(vessel_guid, parameters);
        }
        UnityEngine.GUILayout.TextArea(parameters.max_steps.ToString(),
                                       UnityEngine.GUILayout.Width(150));
        if (parameters.max_steps >= Int64.MaxValue / 10) {
          UnityEngine.GUILayout.Button("max");
        } else if (UnityEngine.GUILayout.Button("+")) {
          parameters.max_steps *= 10;
          plugin_.FlightPlanSetAdaptiveStepParameters(vessel_guid, parameters);
        }
        UnityEngine.GUILayout.EndHorizontal();
        UnityEngine.GUILayout.BeginHorizontal();
        UnityEngine.GUILayout.Label("Tolerance",
                                    UnityEngine.GUILayout.Width(150));
        if (parameters.length_integration_tolerance <= 1e-6) {
          UnityEngine.GUILayout.Button("min");
        } else if (UnityEngine.GUILayout.Button("-")) {
          parameters.length_integration_tolerance /= 2;
          parameters.speed_integration_tolerance /= 2;
          plugin_.FlightPlanSetAdaptiveStepParameters(vessel_guid, parameters);
        }
        UnityEngine.GUILayout.TextArea(
            parameters.length_integration_tolerance.ToString("0.0e0") + " m",
            UnityEngine.GUILayout.Width(150));
        if (parameters.length_integration_tolerance >= 1e6) {
          UnityEngine.GUILayout.Button("max");
        } else if (UnityEngine.GUILayout.Button("+")) {
          parameters.length_integration_tolerance *= 2;
          parameters.speed_integration_tolerance *= 2;
          plugin_.FlightPlanSetAdaptiveStepParameters(vessel_guid, parameters);
        }
        UnityEngine.GUILayout.EndHorizontal();

        if (UnityEngine.GUILayout.Button("Delete flight plan")) {
          plugin_.FlightPlanDelete(vessel_guid);
          Reset();
        } else {
          if (burn_editors_.Count > 0) {
            RenderUpcomingEvents();
          }
          for (int i = 0; i < burn_editors_.Count - 1; ++i) {
            UnityEngine.GUILayout.TextArea("Manœuvre #" + (i + 1) + ":");
            burn_editors_[i].Render(enabled : false);
          }
          if (burn_editors_.Count > 0) {
            BurnEditor last_burn = burn_editors_.Last();
            UnityEngine.GUILayout.TextArea("Editing manœuvre #" +
                                           (burn_editors_.Count) + ":");
            if (last_burn.Render(enabled : true)) {
              plugin_.FlightPlanReplaceLast(vessel_guid, last_burn.Burn());
              last_burn.Reset(
                  plugin_.FlightPlanGetManoeuvre(vessel_guid,
                                                 burn_editors_.Count - 1));
            }
            if (UnityEngine.GUILayout.Button(
                    "Delete last manœuvre",
                    UnityEngine.GUILayout.ExpandWidth(true))) {
              plugin_.FlightPlanRemoveLast(vessel_guid);
              burn_editors_.Last().Close();
              burn_editors_.RemoveAt(burn_editors_.Count - 1);
              Shrink();
            }
          }
          if (UnityEngine.GUILayout.Button(
                  "Add manœuvre",
                  UnityEngine.GUILayout.ExpandWidth(true))) {
            double initial_time;
            if (burn_editors_.Count == 0) {
              initial_time = plugin_.CurrentTime() + 60;
            } else {
              initial_time =
                  plugin_.FlightPlanGetManoeuvre(
                      vessel_guid,
                      burn_editors_.Count - 1).final_time + 60;
            }
            var editor =
                new BurnEditor(adapter_, plugin_, vessel_, initial_time);
            Burn candidate_burn = editor.Burn();
            bool inserted = plugin_.FlightPlanAppend(vessel_guid,
                                                     candidate_burn);
            if (inserted) {
              editor.Reset(plugin_.FlightPlanGetManoeuvre(vessel_guid,
                                                          burn_editors_.Count));
              burn_editors_.Add(editor);
            }
            Shrink();
          }
        }
      }
    }
    UnityEngine.GUILayout.EndVertical();

    UnityEngine.GUI.DragWindow(
        position : new UnityEngine.Rect(left : 0f, top : 0f, width : 10000f,
                                        height : 10000f));

    UnityEngine.GUI.skin = old_skin;
  }

  private void RenderUpcomingEvents() {
    string vessel_guid = vessel_.id.ToString();
    double current_time = plugin_.CurrentTime();
    bool should_clear_guidance = true;
    for (int i = 0; i < burn_editors_.Count; ++i) {
      NavigationManoeuvre manoeuvre =
          plugin_.FlightPlanGetManoeuvre(vessel_guid, i);
      if (manoeuvre.final_time > current_time) {
        if (manoeuvre.burn.initial_time > current_time) {
          UnityEngine.GUILayout.TextArea("Upcoming manœuvre: #" + (i + 1));
          UnityEngine.GUILayout.Label(
              "Ignition " + FormatTimeSpan(TimeSpan.FromSeconds(
                                current_time - manoeuvre.burn.initial_time)));
        } else {
          UnityEngine.GUILayout.TextArea("Ongoing manœuvre: #" + (i + 1));
          UnityEngine.GUILayout.Label(
              "Cutoff " + FormatTimeSpan(TimeSpan.FromSeconds(
                              current_time - manoeuvre.final_time)));
        }
        show_guidance_ =
            UnityEngine.GUILayout.Toggle(show_guidance_, "Show on navball");
        if (show_guidance_ &&
            !double.IsNaN(manoeuvre.inertial_direction.x +
                          manoeuvre.inertial_direction.y +
                          manoeuvre.inertial_direction.z)) {
          if (guidance_node_ == null) {
            guidance_node_ = vessel_.patchedConicSolver.AddManeuverNode(
                manoeuvre.burn.initial_time);
          }
          Vector3d stock_velocity_at_node_time =
              vessel_.orbit.getOrbitalVelocityAtUT(
                                manoeuvre.burn.initial_time).xzy;
          Vector3d stock_displacement_from_parent_at_node_time =
              vessel_.orbit.getRelativePositionAtUT(
                                manoeuvre.burn.initial_time).xzy;
          UnityEngine.Quaternion stock_frenet_frame_to_world =
              UnityEngine.Quaternion.LookRotation(
                  stock_velocity_at_node_time,
                  Vector3d.Cross(stock_velocity_at_node_time,
                                 stock_displacement_from_parent_at_node_time));
          guidance_node_.OnGizmoUpdated(
              ((Vector3d)manoeuvre.burn.delta_v).magnitude *
                  (Vector3d)(stock_frenet_frame_to_world.Inverse() *
                             (Vector3d)manoeuvre.inertial_direction),
              manoeuvre.burn.initial_time);
          should_clear_guidance = false;
        }
        break;
      }
    }
    if (should_clear_guidance && guidance_node_ != null) {
      vessel_.patchedConicSolver.RemoveManeuverNode(guidance_node_);
      guidance_node_ = null;
    }
  }

  private void Reset() {
    if (burn_editors_ != null) {
      foreach (BurnEditor editor in burn_editors_) {
        editor.Close();
      }
      Shrink();
    }
    burn_editors_ = null;
    vessel_ = FlightGlobals.ActiveVessel;
  }

  private void Shrink() {
    window_rectangle_.height = 0.0f;
    window_rectangle_.width = 0.0f;
  }

  internal static string FormatPositiveTimeSpan (TimeSpan span) {
    return span.Days.ToString("000;000") + " d " +
           span.Hours.ToString("00;00") + " h " +
           span.Minutes.ToString("00;00") + " min " +
           (span.Seconds + span.Milliseconds / 1000m).ToString("00.0;00.0") +
           " s";
  }

  internal static string FormatTimeSpan (TimeSpan span) {
    return span.Ticks.ToString("+;-") + FormatPositiveTimeSpan(span);
  }

  // Not owned.
  private readonly IntPtr plugin_;
  private readonly PrincipiaPluginAdapter adapter_;
  private Vessel vessel_;
  private List<BurnEditor> burn_editors_;

  private DifferentialSlider final_time_;

  private bool show_planner_ = false;
  private bool show_guidance_ = false;
  private ManeuverNode guidance_node_;
  private UnityEngine.Rect window_rectangle_;
  
  private const double Log10TimeLowerRate = 0.0;
  private const double Log10TimeUpperRate = 7.0;
}

}  // namespace ksp_plugin_adapter
}  // namespace principia
