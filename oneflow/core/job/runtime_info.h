#ifndef ONEFLOW_CORE_JOB_RUNTIME_INFO_H_
#define ONEFLOW_CORE_JOB_RUNTIME_INFO_H_

#include "oneflow/core/common/util.h"
#include "oneflow/core/job/id_manager.h"

namespace oneflow {

class RuntimeInfo final {
 public:
  OF_DISALLOW_COPY_AND_MOVE(RuntimeInfo);
  ~RuntimeInfo() = default;

  OF_SINGLETON(RuntimeInfo);

  int64_t this_machine_id() const { return this_machine_id_; }

  void set_this_machine_name(const std::string& name) {
    this_machine_name_ = name;
    this_machine_id_ = IDMgr::Singleton().MachineID4MachineName(name);
    LOG(INFO) << "this machine name: " << this_machine_name_;
    LOG(INFO) << "this machine id: " << this_machine_id_;
  }

 private:
  RuntimeInfo() = default;

  int64_t this_machine_id_;
  std::string this_machine_name_;

};

} // namespace oneflow

#endif // ONEFLOW_CORE_JOB_RUNTIME_INFO_H_
