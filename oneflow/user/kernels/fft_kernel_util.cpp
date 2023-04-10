/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "oneflow/user/kernels/fft_kernel_util.h"
#include <type_traits>
#include "oneflow/core/common/device_type.pb.h"
#include "oneflow/core/common/preprocessor.h"
#include "pocketfftplan.h"

namespace oneflow {

template<typename T>
struct FftC2CKernelUtil<DeviceType::kCPU,
    T,
    typename std::enable_if<std::is_same<T, std::complex<float>>::value>::type> {
  static void FftC2CForward(ep::Stream* stream, 
                            const std::complex<float>* data_in, std::complex<float>* data_out, std::complex<float>* tmp_buffer, 
                            const Shape& input_shape, const Shape& output_shape, const Shape& tmp_buffer_shape,
                            const Stride& input_stride, const Stride& output_stride, const Stride& tmp_buffer_stride,
                            bool forward,
                            const std::vector<int64_t>& dims, fft_norm_mode normalization) {
    PocketFFtParams<float> params(
        input_shape, output_shape, input_stride, output_stride, dims, forward,
        compute_fct<float>(input_shape, dims, normalization) /*1.f*/, FFT_EXCUTETYPE::C2C);
    PocketFFtConfig<float> config(params);
    config.excute(data_in, data_out);
  }
};

template<typename T>
struct FftC2CKernelUtil<DeviceType::kCPU,
    T,
    typename std::enable_if<std::is_same<T, std::complex<double>>::value>::type> {
  static void FftC2CForward(ep::Stream* stream, const std::complex<double>* data_in, std::complex<double>* data_out, std::complex<double>* tmp_buffer,
                            const Shape& input_shape, const Shape& output_shape, const Shape& tmp_buffer_shape, 
                            const Stride& input_stride, const Stride& output_stride, const Stride& tmp_buffer_stride,
                            bool forward, const std::vector<int64_t>& dims, fft_norm_mode normalization) {
    PocketFFtParams<double> params(
        input_shape, output_shape, input_stride, output_stride, dims, forward,
        compute_fct<double>(input_shape, dims, normalization) /*1.f*/, FFT_EXCUTETYPE::C2C);
    PocketFFtConfig<double> config(params);
    config.excute(data_in, data_out);
  }
};

template<typename IN, typename OUT>
struct FftR2CKernelUtil<DeviceType::kCPU, IN, OUT> {
  static void FftR2CForward(ep::Stream* stream, const IN* data_in, OUT* data_out, OUT* tmp_buffer,
                            const Shape& input_shape, const Shape& output_shape, const Shape& tmp_buffer_shape,
                            const Stride& input_stride, const Stride& output_stride, const Shape& tmp_buffer_stride,
                            bool forward,
                            const std::vector<int64_t>& dims, fft_norm_mode normalization) {
    PocketFFtParams<IN> params(input_shape, output_shape, input_stride, output_stride, dims, forward,
                              compute_fct<IN>(input_shape, dims, normalization) /*1.f*/,
                              FFT_EXCUTETYPE::R2C);
    PocketFFtConfig<IN> config(params);
    config.excute(data_in, data_out);
  }
};

template<typename IN, typename OUT>
struct FftC2RKernelUtil<DeviceType::kCPU, IN, OUT> {
  static void FftC2RForward(ep::Stream* stream, const IN* data_in, OUT* data_out, IN* tmp_buffer,
                            const Shape& input_shape, const Shape& output_shape, const Shape& tmp_buffer_shape,
                            const Stride& input_stride, const Stride& output_stride, const Shape& tmp_buffer_stride,
                            int64_t last_dim_size, const std::vector<int64_t>& dims,
                            fft_norm_mode normalization) {
    PocketFFtParams<OUT> params(
        input_shape, output_shape, input_stride, output_stride, dims, /*is_forward=*/false,
        compute_fct<OUT>(output_shape, dims, normalization) /*1.f*/, FFT_EXCUTETYPE::C2R);
    PocketFFtConfig<OUT> config(params);
    config.excute(data_in, data_out);
  }
};

template<typename IN, typename OUT>
struct FftStftKernelUtil<DeviceType::kCPU, IN, OUT> {
  static void FftStftForward(ep::Stream* stream, const IN* data_in, OUT* data_out,
                             const Shape& input_shape, const Shape& output_shape,
                             const Stride& input_stride, const Stride& output_stride, bool forward,
                             const std::vector<int64_t>& axes, fft_norm_mode normalization,
                             int64_t len, int64_t dims, int64_t batch) {
    PocketFFtParams<IN> params(input_shape, output_shape, input_stride, output_stride, axes, forward,
                              compute_fct<IN>(len, normalization) /*1.f*/, FFT_EXCUTETYPE::R2C);
    PocketFFtConfig<IN> config(params);
    int64_t in_offset = len;
    int64_t out_offset = len / 2 + 1;
    for (int j = 0; j < dims; j++) {
      for (int i = 0; i < batch; i++) {
        const IN* in = data_in + j * batch * in_offset + i * in_offset;
        OUT* out = data_out + j * batch * out_offset + i * out_offset;
        config.excute(in, out);
      }
    }
  }
};

template struct FftC2CKernelUtil<DeviceType::kCPU, std::complex<float>>;
template struct FftC2CKernelUtil<DeviceType::kCPU, std::complex<double>>;

template struct FftR2CKernelUtil<DeviceType::kCPU, float, std::complex<float>>;
template struct FftR2CKernelUtil<DeviceType::kCPU, double, std::complex<double>>;

template struct FftC2RKernelUtil<DeviceType::kCPU, std::complex<float>, float>;
template struct FftC2RKernelUtil<DeviceType::kCPU, std::complex<double>, double>;

template struct FftStftKernelUtil<DeviceType::kCPU, float, std::complex<float>>;
template struct FftStftKernelUtil<DeviceType::kCPU, double, std::complex<double>>;
}  // namespace oneflow