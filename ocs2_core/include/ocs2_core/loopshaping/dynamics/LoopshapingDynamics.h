//
// Created by ruben on 14.09.18.
//

#pragma once

#include <memory>

#include <ocs2_core/Types.h>
#include <ocs2_core/dynamics/SystemDynamicsBase.h>
#include <ocs2_core/loopshaping/LoopshapingDefinition.h>

namespace ocs2 {
class LoopshapingDynamics : public SystemDynamicsBase {
 public:
  ~LoopshapingDynamics() override = default;

  static std::unique_ptr<LoopshapingDynamics> create(const SystemDynamicsBase& systemDynamics,
                                                     std::shared_ptr<LoopshapingDefinition> loopshapingDefinition);

  vector_t computeFlowMap(scalar_t time, const vector_t& state, const vector_t& input) final;
  vector_t computeJumpMap(scalar_t time, const vector_t& state) final;
  vector_t computeGuardSurfaces(scalar_t time, const vector_t& state) final;

  VectorFunctionLinearApproximation jumpMapLinearApproximation(scalar_t t, const vector_t& x) final;
  VectorFunctionLinearApproximation guardSurfacesLinearApproximation(scalar_t t, const vector_t& x, const vector_t& u) final;

  vector_t flowMapDerivativeTime(scalar_t t, const vector_t& x, const vector_t& u) final;
  vector_t jumpMapDerivativeTime(scalar_t t, const vector_t& x, const vector_t& u) final;
  vector_t guardSurfacesDerivativeTime(scalar_t t, const vector_t& x, const vector_t& u) final;

 protected:
  LoopshapingDynamics(const LoopshapingDynamics& obj)
      : systemDynamics_(obj.systemDynamics_->clone()), loopshapingDefinition_(obj.loopshapingDefinition_) {}

  LoopshapingDynamics(const SystemDynamicsBase& systemDynamics, std::shared_ptr<LoopshapingDefinition> loopshapingDefinition)
      : systemDynamics_(systemDynamics.clone()), loopshapingDefinition_(std::move(loopshapingDefinition)){};

  std::unique_ptr<SystemDynamicsBase> systemDynamics_;
  std::shared_ptr<LoopshapingDefinition> loopshapingDefinition_;

 private:
  virtual vector_t filterFlowmap(const vector_t& x_filter, const vector_t& u_filter, const vector_t& u_system) = 0;
};

}  // namespace ocs2
