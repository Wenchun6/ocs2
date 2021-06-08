/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <gtest/gtest.h>

#include "ocs2_sqp/MultipleShootingSolver.h"

#include <ocs2_core/initialization/OperatingPoints.h>

#include <ocs2_oc/test/circular_kinematics.h>

TEST(test_circular_kinematics, solve_projected_EqConstraints) {
  ocs2::CircularKinematicsSystem system;
  ocs2::CircularKinematicsCost cost;
  ocs2::CircularKinematicsConstraints constraint;

  // Solver settings
  ocs2::multiple_shooting::Settings settings;
  settings.dt = 0.01;
  settings.sqpIteration = 20;
  settings.projectStateInputEqualityConstraints = true;
  settings.useFeedbackPolicy = true;
  settings.printSolverStatistics = true;
  settings.printSolverStatus = true;
  settings.printLinesearch = true;

  // Additional problem definitions
  const ocs2::scalar_t startTime = 0.0;
  const ocs2::scalar_t finalTime = 1.0;
  const ocs2::vector_t initState = (ocs2::vector_t(2) << 1.0, 0.0).finished();  // radius 1.0
  const ocs2::scalar_array_t partitioningTimes{0.0};                            // doesn't matter
  ocs2::OperatingPoints operatingPoints(initState, ocs2::vector_t::Zero(2));

  // Solve
  ocs2::MultipleShootingSolver solver(settings, &system, &cost, &operatingPoints, &constraint);
  solver.run(startTime, initState, finalTime, partitioningTimes);

  // Inspect solution
  const auto primalSolution = solver.primalSolution(finalTime);
  for (int i = 0; i < primalSolution.timeTrajectory_.size(); i++) {
    std::cout << "time: " << primalSolution.timeTrajectory_[i] << "\t state: " << primalSolution.stateTrajectory_[i].transpose()
              << "\t input: " << primalSolution.inputTrajectory_[i].transpose() << std::endl;
  }

  // Check initial condition
  ASSERT_TRUE(primalSolution.stateTrajectory_.front().isApprox(initState));
  ASSERT_DOUBLE_EQ(primalSolution.timeTrajectory_.front(), startTime);
  ASSERT_DOUBLE_EQ(primalSolution.timeTrajectory_.back(), finalTime);

  // Check constraint satisfaction.
  const auto performance = solver.getPerformanceIndeces();
  ASSERT_LT(performance.stateEqConstraintISE, 1e-6);
  ASSERT_LT(performance.stateInputEqConstraintISE, 1e-6);

  // Check feedback controller
  for (int i = 0; i < primalSolution.timeTrajectory_.size() - 1; i++) {
    const auto t = primalSolution.timeTrajectory_[i];
    const auto& x = primalSolution.stateTrajectory_[i];
    const auto& u = primalSolution.inputTrajectory_[i];
    // Feed forward part
    ASSERT_TRUE(u.isApprox(primalSolution.controllerPtr_->computeInput(t, x)));
  }
}

TEST(test_circular_kinematics, solve_EqConstraints_inQPSubproblem) {
  ocs2::CircularKinematicsSystem system;
  ocs2::CircularKinematicsCost cost;
  ocs2::CircularKinematicsConstraints constraint;

  // Solver settings
  ocs2::multiple_shooting::Settings settings;
  settings.dt = 0.01;
  settings.sqpIteration = 20;
  settings.projectStateInputEqualityConstraints = false;  // <- false to turn off projection of state-input equalities
  settings.useFeedbackPolicy = true;
  settings.printSolverStatistics = true;
  settings.printSolverStatus = true;
  settings.printLinesearch = true;

  // Additional problem definitions
  const ocs2::scalar_t startTime = 0.0;
  const ocs2::scalar_t finalTime = 1.0;
  const ocs2::vector_t initState = (ocs2::vector_t(2) << 1.0, 0.0).finished();  // radius 1.0
  const ocs2::scalar_array_t partitioningTimes{0.0};                            // doesn't matter
  ocs2::OperatingPoints operatingPoints(initState, ocs2::vector_t::Zero(2));

  // Solve
  ocs2::MultipleShootingSolver solver(settings, &system, &cost, &operatingPoints, &constraint);
  solver.run(startTime, initState, finalTime, partitioningTimes);

  // Inspect solution
  const auto primalSolution = solver.primalSolution(finalTime);
  for (int i = 0; i < primalSolution.timeTrajectory_.size(); i++) {
    std::cout << "time: " << primalSolution.timeTrajectory_[i] << "\t state: " << primalSolution.stateTrajectory_[i].transpose()
              << "\t input: " << primalSolution.inputTrajectory_[i].transpose() << std::endl;
  }

  // Check initial condition
  ASSERT_TRUE(primalSolution.stateTrajectory_.front().isApprox(initState));
  ASSERT_DOUBLE_EQ(primalSolution.timeTrajectory_.front(), startTime);
  ASSERT_DOUBLE_EQ(primalSolution.timeTrajectory_.back(), finalTime);

  // Check constraint satisfaction.
  const auto performance = solver.getPerformanceIndeces();
  ASSERT_LT(performance.stateEqConstraintISE, 1e-6);
  ASSERT_LT(performance.stateInputEqConstraintISE, 1e-6);

  // Check feedback controller
  for (int i = 0; i < primalSolution.timeTrajectory_.size() - 1; i++) {
    const auto t = primalSolution.timeTrajectory_[i];
    const auto& x = primalSolution.stateTrajectory_[i];
    const auto& u = primalSolution.inputTrajectory_[i];
    // Feed forward part
    ASSERT_TRUE(u.isApprox(primalSolution.controllerPtr_->computeInput(t, x)));
  }
}