#ifndef ONEFLOW_USER_KERNELS_FFT_KERNEL_UTIL_H_
#define ONEFLOW_USER_KERNELS_FFT_KERNEL_UTIL_H_

#include <cstdint>
#include <vector>
#include "oneflow/core/common/data_type.pb.h"
#include "oneflow/core/common/maybe.h"
#include "oneflow/core/common/shape.h"
#include "oneflow/core/common/throw.h"
#include "oneflow/core/common/util.h"
#include "oneflow/core/framework/framework.h"
#include "oneflow/core/framework/op_kernel.h"
#include "oneflow/core/ep/include/stream.h"
#include "oneflow/core/operator/operator_util.h"
#include "oneflow/core/common/shape_vec.h"
#include "oneflow/core/kernel/kernel_util.h"

namespace oneflow{

enum class fft_norm_mode {
    none = 0,   // No normalization
    by_root_n,  // Divide by sqrt(signal_size)
    by_n,       // Divide by signal_size
};

// Convert NumPy compatible normalization mode string to enum values
// In Numpy, "forward" translates to `by_n` for a forward transform and `none` for backward.
fft_norm_mode norm_from_string(Optional<std::string> norm_op, bool forward){
    if (!norm_op.has_value() || norm_op.value() == "backward"){
        return forward ? fft_norm_mode::none : fft_norm_mode::by_n;
    }
    else if (norm_op.value() == "forward"){
        return forward ? fft_norm_mode::by_n : fft_norm_mode::none;
    }
    else if (norm_op.value() == "ortho"){
        return fft_norm_mode::by_root_n;
    }

    CHECK_OR_THROW(false) << "Invalid normalization mode: \"" << norm_op.value() << "\"";
}

template<typename T>
T compute_fct(int64_t size, fft_norm_mode normalization) {
  constexpr auto one = static_cast<T>(1);
  switch (normalization) {
    case fft_norm_mode::none: return one;
    case fft_norm_mode::by_n: return one / static_cast<T>(size);
    case fft_norm_mode::by_root_n: return one / std::sqrt(static_cast<T>(size));
  }
  return static_cast<T>(0);
}

template<typename T>
T compute_fct(const Shape& in_shape, std::vector<int64_t> dims, fft_norm_mode normalization){
  if (normalization == fft_norm_mode::none) {
    return static_cast<T>(1);
  }
  int64_t n = 1;
  for(int64_t idx : dims) {
    n *= in_shape.At(idx);
  }
  return compute_fct<T>(n, normalization);
}


template<DeviceType device_type, typename IN, typename OUT, typename fct_type>
struct FftC2CKernelUtil{
    static void FftC2CForward(ep::Stream* stream, IN* data_in, OUT* data_out, const Shape& input_shape_view, 
                              const Shape& output_shape, bool forward, const std::vector<int64_t>& dims,
                              fft_norm_mode normalization);
};




}   // oneflow
#endif // ONEFLOW_USER_KERNEL_UTIL_H_