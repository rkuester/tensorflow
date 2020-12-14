/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/kernel_runner.h"
#include "tensorflow/lite/micro/micro_utils.h"
#include "tensorflow/lite/micro/test_helpers.h"
#include "tensorflow/lite/micro/testing/micro_test.h"

namespace {

using namespace tflite::testing;

struct FillOp {
  static constexpr int dims_index = 0;
  static constexpr int value_index = 1;
  static constexpr int output_index = 2;
  const int inputs[3] = {2, dims_index, value_index};
  const int outputs[2] = {1, output_index};

  TfLiteTensor tensors[3];
  tflite::micro::KernelRunner runner;

  template <typename DimsType, typename ValueType, typename OutputType>
  FillOp(int* dims_shape, DimsType* dims_data,
         int* value_shape, ValueType* value_data,
         int* output_shape, OutputType* output_data)
  :
    tensors{
      CreateTensor(dims_data, IntArrayFromInts(dims_shape)),
      CreateTensor(value_data, IntArrayFromInts(value_shape)),
      CreateTensor(output_data, IntArrayFromInts(output_shape))},
    runner{tflite::Register_FILL(),
           tensors,
           sizeof(tensors) / sizeof(TfLiteTensor),
           IntArrayFromInts(inputs),
           IntArrayFromInts(outputs),
           /*builtin_data=*/nullptr,
           micro_test::reporter} {}
};

template <typename DimsType, typename ValueType, typename OutputType>
void TestFill(int* dims_shape, DimsType* dims_data,
              int* value_shape, ValueType* value_data,
              int* output_shape, OutputType* output_data) {
  FillOp op{dims_shape, dims_data,
            value_shape, value_data,
            output_shape, output_data};

  TF_LITE_MICRO_EXPECT_EQ(op.runner.InitAndPrepare(), kTfLiteOk);
  TF_LITE_MICRO_EXPECT_EQ(op.runner.Invoke(), kTfLiteOk);

  // The output shape must match the shape requested via dims.
  const auto output_rank = output_shape[0];
  const auto requested_rank = dims_shape[1]; // yes, 1
  if (output_rank == requested_rank) {
    for (int i = 0; i < requested_rank; ++i) {
      TF_LITE_MICRO_EXPECT_EQ(output_shape[i + 1], dims_data[i]);
    }
  } else {
    TF_LITE_MICRO_FAIL("output shape does not match shape requested via dims");
  }

  // The output type matches the value type.
  TF_LITE_MICRO_EXPECT_EQ(op.tensors[op.output_index].type,
                          op.tensors[op.value_index].type);

  // The output elements contain the fill value.
  const auto elements = tflite::ElementCount(*IntArrayFromInts(output_shape));
  for (int i = 0; i < elements; ++i) {
    TF_LITE_MICRO_EXPECT_EQ(output_data[i], value_data[0]);
  }
}

template <typename DimsType, typename ValueType, typename OutputType>
TfLiteStatus PrepareFill(int* dims_shape, DimsType* dims_data,
                         int* value_shape, ValueType* value_data,
                         int* output_shape, OutputType* output_data) {
  FillOp op{dims_shape, dims_data,
            value_shape, value_data,
            output_shape, output_data};

  return op.runner.InitAndPrepare();
}

}  // namespace anonymous

TF_LITE_MICRO_TESTS_BEGIN

TF_LITE_MICRO_TEST(FillInt8) {
  constexpr int dim1 = 3;
  constexpr int dim2 = 8;

  int dims_shape[] = {1, 2};
  int32_t dims_data[] = {dim1, dim2};

  int value_shape[] = {0};
  int8_t value_data[] = {-42};
 
  int output_shape[] = {2, dim1, dim2};
  int8_t output_data[dim1 * dim2];

  TestFill(dims_shape, dims_data,
           value_shape, value_data,
           output_shape, output_data);
}

TF_LITE_MICRO_TEST(FillFloatInt64Dims) {
  constexpr int dim1 = 2;
  constexpr int dim2 = 2;
  constexpr int dim3 = 2;

  int dims_shape[] = {1, 3};
  int64_t dims_data[] = {dim1, dim2, dim3};

  int value_shape[] = {0};
  float value_data[] = {4.0};
 
  int output_shape[] = {3, dim1, dim2, dim3};
  float output_data[dim1 * dim2 * dim3];

  TestFill(dims_shape, dims_data,
           value_shape, value_data,
           output_shape, output_data);
}

TF_LITE_MICRO_TEST(FillFloatInt32Dims) {
  constexpr int dim1 = 2;
  constexpr int dim2 = 2;
  constexpr int dim3 = 2;

  int dims_shape[] = {1, 3};
  int32_t dims_data[] = {dim1, dim2, dim3};

  int value_shape[] = {0};
  float value_data[] = {4.0};
 
  int output_shape[] = {3, dim1, dim2, dim3};
  float output_data[dim1 * dim2 * dim3];

  TestFill(dims_shape, dims_data,
           value_shape, value_data,
           output_shape, output_data);
}

TF_LITE_MICRO_TEST(FillFloatInt8Dims) {
  constexpr int dim1 = 3;
  constexpr int dim2 = 2;
  constexpr int dim3 = 4;

  int dims_shape[] = {1, 3};
  int8_t dims_data[] = {dim1, dim2, dim3};

  int value_shape[] = {0};
  float value_data[] = {4.0};
 
  int output_shape[] = {3, dim1, dim2, dim3};
  float output_data[dim1 * dim2 * dim3];

  TestFill(dims_shape, dims_data,
           value_shape, value_data,
           output_shape, output_data);
}

TF_LITE_MICRO_TEST(FillScalar) {
  int dims_shape[] = {1, 0};
  int64_t dims_data[] = {0};

  int value_shape[] = {0};
  float value_data[] = {4.0};
 
  int output_shape[] = {0};
  float output_data[] = {0};

  TestFill(dims_shape, dims_data,
           value_shape, value_data,
           output_shape, output_data);
}

TF_LITE_MICRO_TEST(FillMismatchedShape) {
  // The shape of the output tensor shape must match the value given in the
  // dims input tensor, because output tensors are not dynamically adjusted.

  constexpr int dim1 = 3;
  constexpr int dim2 = 4;

  int dims_shape[] = {1, 2};
  int64_t dims_data[] = {dim1, dim2};

  int value_shape[] = {0};
  float value_data[] = {4.0};
 
  int output_shape[] = {2, dim1 + 1, dim2 + 1};
  float output_data[dim1 * dim2];

  TF_LITE_MICRO_EXPECT_EQ(kTfLiteError,
                          PrepareFill(dims_shape, dims_data,
                                      value_shape, value_data,
                                      output_shape, output_data));
}

TF_LITE_MICRO_TEST(FillMismatchedType) {
  // The type of the output tensor must match the type of the value input
  // tensor, because output tensors are not dynamically adjusted.

  using value_type = float;
  using output_type = int8_t;
  constexpr int dim1 = 3;
  constexpr int dim2 = 2;

  int dims_shape[] = {1, 2};
  int64_t dims_data[] = {dim1, dim2};

  int value_shape[] = {0};
  value_type value_data[] = {4.0};
 
  int output_shape[] = {2, dim1, dim2};
  output_type output_data[dim1 * dim2];

  TF_LITE_MICRO_EXPECT_EQ(kTfLiteError,
                          PrepareFill(dims_shape, dims_data,
                                      value_shape, value_data,
                                      output_shape, output_data));
}

TF_LITE_MICRO_TESTS_END
