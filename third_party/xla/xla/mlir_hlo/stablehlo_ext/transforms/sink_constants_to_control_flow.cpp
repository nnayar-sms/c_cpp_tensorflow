/* Copyright 2020 The OpenXLA Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "llvm/ADT/DenseMap.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"  // IWYU pragma: keep, passes.h.inc
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Region.h"
#include "mlir/IR/Value.h"
#include "mlir/Pass/Pass.h"  // IWYU pragma: keep, passes.h.inc
#include "mlir/Pass/PassManager.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Transforms/RegionUtils.h"
#include "stablehlo/dialect/StablehloOps.h"

namespace mlir {
namespace stablehlo_ext {

#define GEN_PASS_DEF_SINKCONSTANTSTOCONTROLFLOWPASS
#include "stablehlo_ext/transforms/passes.h.inc"

namespace {

// A pass that sinks constants implicitly captured in control flow regions. This
// is necessary to export to XLA.
//
// TODO(b/203775547): Any value used within the region that is defined outside
// of op's region should be sank to the regions and not just the constants. Ops
// such as If and While whose computations doesn't require fixed signature like
// Sort or Reduce have an option to pass outside values as operands of the op to
// avoid recomputing those within internally. Note that doing so is the only
// option in case of values defined outside that are BlockArguments of any of
// the parent region.
class SinkConstantsToControlFlowPass
    : public impl::SinkConstantsToControlFlowPassBase<
          SinkConstantsToControlFlowPass> {
  void runOnOperation() override {
    getOperation().walk([](Operation* op) {
      // TODO: Consider special handling for WhileOp.
      for (Region& region : op->getRegions()) sinkToRegion(&region);
    });
  }

 private:
  // Performs constant sinking into a region.
  static void sinkToRegion(Region* region) {
    llvm::DenseMap<Value, Operation*> sunkConstant;
    visitUsedValuesDefinedAbove({*region}, [&](OpOperand* use) {
      Value constant = use->get();
      auto* op = constant.getDefiningOp();
      if (!op || !op->hasTrait<mlir::OpTrait::ConstantLike>()) return;
      auto mapEntry = sunkConstant.try_emplace(constant, nullptr);
      if (!mapEntry.second) {
        // This constant has already been cloned into the region, reuse it.
        use->set(mapEntry.first->getSecond()->getResult(0));
        if (op->use_empty()) op->erase();
        return;
      }
      if (constant.hasOneUse()) {
        op->moveBefore(&region->front().front());
        return;
      }
      mapEntry.first->getSecond() = op->clone();
      region->front().getOperations().insert(region->front().begin(),
                                             mapEntry.first->getSecond());
      use->set(mapEntry.first->getSecond()->getResult(0));
    });
  }
};

}  // anonymous namespace

}  // namespace stablehlo_ext
}  // namespace mlir
