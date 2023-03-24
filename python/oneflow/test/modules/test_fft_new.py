"""
Copyright 2023 The OneFlow Authors. All rights reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
import unittest

import numpy as np
from collections import OrderedDict

import oneflow as flow
import torch
import oneflow.unittest
from oneflow.test_utils.automated_test_util import *
from oneflow.test_utils.test_util import GenArgList

def tensor_builder(params: dict, dtype=np.complex64):
    input_shape = params["shape"]

    # generate random input
    x = np.random.randn(input_shape) + 1.j * np.random.randn(input_shape)
    x = x.astype(dtype)

    # transfer to gpu memory
    x_flow = flow.from_numpy(x).requires_grad_(True)
    x_torch = torch.from_numpy(x).requires_grad_(True)

    return x_flow, x_torch

def compare_result(test_case, a, b, rtol=1e-5, atol=1e-8):
    test_case.assertTrue(
        np.allclose(a.numpy(), b.numpy(), rtol=rtol, atol=atol),
        f"\na\n{a.numpy()}\n{'-' * 80}\nb:\n{b.numpy()}\n{'*' * 80}\ndiff:\n{a.numpy() - b.numpy()}",
    )

def _test_fft(test_case, params: dict, dtype=np.complex64):
    print(f"========== Start Testing ==========")
    print(f"tensor shape: {params['shape']}")
    print(f"dtype: {dtype}")

    x_flow, x_torch = tensor_builder(params=params, dtype=dtype)
    n = params['n']
    dim = params['dim']
    norm = params['norm']
    print(f"fft n: {n}")
    print(f"fft dim: {dim}")
    print(f"fft norm: {norm}")
    print(f"x_flow.dtype: {x_flow.dtype}")
    print(f"x_torch.dtype: {x_torch.dtype}")
    

    # forward
    y_torch = torch.fft.fft(x_torch, 
                            n=n, 
                            dim=dim, 
                            norm=norm)

    # backward
    y_torch.sum().backward()

    # copy back to cpu memory
    x_torch_grad = x_torch.grad.detach().cpu()
    y_torch = y_torch.detach().cpu()

    # forward
    y_flow = flow._C.fft(x_flow,
                         n=n,
                         dim=dim,
                         norm=norm)

    # backward
    y_flow.sum().backward()

    # copy back to cpu memory
    x_flow_grad = x_flow.grad.detach().cpu()
    y_flow = y_flow.detach().cpu()

    compare_result(test_case, y_flow, y_torch, 1e-5, 1e-2)
    compare_result(test_case, x_flow_grad, x_torch_grad, 1e-5, 1e-2)

    print(f"============== PASSED =============")
    print("\n")


class TestFft(flow.unittest.TestCase):
    def test_gather(test_case):
        arg_dict = OrderedDict()
        # set up test functions
        arg_dict["test_fun"] = [
            _test_fft,
        ]

        # set up profiling functions        
        arg_dict["params"] = []
        lower_n_dims = 1
        upper_n_dims = 5
        for _ in range(10):
            num_dims = np.random.randint(lower_n_dims, upper_n_dims)
            shape = [np.random.randint(1,11) * 8 for _ in range(num_dims)]
            if np.random.randint(0,1) == 1:
                dim = np.random.randint(low=-num_dims, high=num_dims-1)
            else:
                dim = None
            
            norm = np.random.choice(["backward", "forward", "ortho", None])

            if np.random.randint(0,1) == 1 and dim is not None:
                n = np.random.randint(low=1, high=shape[dim])
            else:
                n = None
            

            arg_dict["params"].append(
                {"shape" : shape,
                 "n" : n,
                 "dim" : dim,
                 "norm" : norm})

        arg_dict["dtype"] = [np.complex64, np.complex128]

        for arg in GenArgList(arg_dict):
            arg[0](test_case, *arg[1:])

if __name__ == "__main__":
    unittest.main()