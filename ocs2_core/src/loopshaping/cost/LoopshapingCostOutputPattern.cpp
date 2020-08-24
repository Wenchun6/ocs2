/******************************************************************************
Copyright (c) 2020, Ruben Grandia. All rights reserved.

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

#include <ocs2_core/loopshaping/cost/LoopshapingCostOutputPattern.h>

namespace ocs2 {

ScalarFunctionQuadraticApproximation LoopshapingCostOutputPattern::costQuadraticApproximation(scalar_t t, const vector_t& x,
                                                                                              const vector_t& u) {
  const scalar_t gamma = loopshapingDefinition_->gamma_;
  const auto& r_filter = loopshapingDefinition_->getInputFilter();
  const vector_t x_system = loopshapingDefinition_->getSystemState(x);
  const vector_t u_system = loopshapingDefinition_->getSystemInput(x, u);
  const vector_t x_filter = loopshapingDefinition_->getFilterState(x);
  const vector_t u_filter = loopshapingDefinition_->getFilteredInput(x, u);
  const auto& L_system = systemCost_->costQuadraticApproximation(t, x_system, u_system);
  const auto& L_filter = systemCost_->costQuadraticApproximation(t, x_system, u_filter);

  ScalarFunctionQuadraticApproximation L;
  L.f = gamma * L_filter.f + (1.0 - gamma) * L_system.f;

  L.dfdx.resize(x.rows());
  L.dfdx.head(x_system.rows()) = gamma * L_filter.dfdx + (1.0 - gamma) * L_system.dfdx;
  L.dfdx.tail(x_filter.rows()) = gamma * r_filter.getC().transpose() * L_filter.dfdu;

  L.dfdxx.resize(x.rows(), x.rows());
  L.dfdxx.topLeftCorner(x_system.rows(), x_system.rows()) = gamma * L_filter.dfdxx + (1.0 - gamma) * L_system.dfdxx;
  L.dfdxx.topRightCorner(x_system.rows(), x_filter.rows()) = gamma * L_filter.dfdux.transpose() * r_filter.getC();
  L.dfdxx.bottomLeftCorner(x_filter.rows(), x_system.rows()) = L.dfdxx.topRightCorner(x_system.rows(), x_filter.rows()).transpose();
  L.dfdxx.bottomRightCorner(x_filter.rows(), x_filter.rows()) = gamma * r_filter.getC().transpose() * L_filter.dfduu * r_filter.getC();

  L.dfdu = gamma * r_filter.getD().transpose() * L_filter.dfdu + (1.0 - gamma) * L_system.dfdu;

  L.dfduu = gamma * r_filter.getD().transpose() * L_filter.dfduu * r_filter.getD() + (1.0 - gamma) * L_system.dfduu;

  L.dfdux.resize(u.rows(), x.rows());
  L.dfdux.leftCols(x_system.rows()) = gamma * r_filter.getD().transpose() * L_filter.dfdux + (1.0 - gamma) * L_system.dfdux;
  L.dfdux.rightCols(x_filter.rows()) = gamma * r_filter.getD().transpose() * L_filter.dfduu * r_filter.getC();

  return L;
}

}  // namespace ocs2