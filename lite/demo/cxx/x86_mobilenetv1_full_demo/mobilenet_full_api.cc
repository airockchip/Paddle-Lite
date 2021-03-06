// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <vector>
#include "paddle_api.h"  // NOLINT
/////////////////////////////////////////////////////////////////////////
// If this demo is linked to static library:libpaddle_api_full_bundled.a
// , you should include `paddle_use_ops.h` and `paddle_use_kernels.h` to
// avoid linking errors such as `unsupport ops or kernels`.
/////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include "paddle_use_kernels.h"  // NOLINT
#include "paddle_use_ops.h"      // NOLINT
#endif

using namespace paddle::lite_api;  // NOLINT

int64_t ShapeProduction(const shape_t& shape) {
  int64_t res = 1;
  for (auto i : shape) res *= i;
  return res;
}

// Enable `DEMO_WITH_OPENCL` macro below, if user need use gpu(opencl)
// #define DEMO_WITH_OPENCL
void RunModel(std::string model_dir) {
  // 1. Create CxxConfig
  CxxConfig config;
  config.set_model_dir(model_dir);
#ifdef DEMO_WITH_OPENCL
  config.set_valid_places(
      {Place{TARGET(kOpenCL), PRECISION(kFP16), DATALAYOUT(kImageDefault)},
       Place{TARGET(kOpenCL), PRECISION(kFloat), DATALAYOUT(kNCHW)},
       Place{TARGET(kOpenCL), PRECISION(kAny), DATALAYOUT(kImageDefault)},
       Place{TARGET(kOpenCL), PRECISION(kAny), DATALAYOUT(kNCHW)},
       Place{TARGET(kX86), PRECISION(kFloat)},
       Place{TARGET(kHost), PRECISION(kFloat)}});
#else
  config.set_valid_places({Place{TARGET(kX86), PRECISION(kFloat)},
                           Place{TARGET(kHost), PRECISION(kFloat)}});
#endif
  // 2. Create PaddlePredictor by CxxConfig
  std::shared_ptr<PaddlePredictor> predictor =
      CreatePaddlePredictor<CxxConfig>(config);

  // 3. Prepare input data
  std::unique_ptr<Tensor> input_tensor(std::move(predictor->GetInput(0)));
  input_tensor->Resize({1, 3, 224, 224});
  auto* data = input_tensor->mutable_data<float>();
  for (int i = 0; i < ShapeProduction(input_tensor->shape()); ++i) {
    data[i] = 1;
  }

  // 4. Run predictor
  predictor->Run();

  // 5. Get output
  std::unique_ptr<const Tensor> output_tensor(
      std::move(predictor->GetOutput(0)));
  std::cout << "Output shape " << output_tensor->shape()[1] << std::endl;
  for (int i = 0; i < ShapeProduction(output_tensor->shape()); i += 100) {
    std::cout << "Output[" << i << "]: " << output_tensor->data<float>()[i]
              << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "[ERROR] usage: ./" << argv[0] << " naive_buffer_model_dir\n";
    exit(1);
  }
  std::string model_dir = argv[1];
  RunModel(model_dir);
  return 0;
}
