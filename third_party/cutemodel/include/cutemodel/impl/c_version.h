//
// Created by YongGyu Lee on 2021/03/04.
//

#ifndef SEESO_CORE_CORE_GAZE_TRACKER_NEURAL_ENGINE_CUTEMODEL_IMPL_C_VERSION_H_
#define SEESO_CORE_CORE_GAZE_TRACKER_NEURAL_ENGINE_CUTEMODEL_IMPL_C_VERSION_H_

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/c_api.h"

#ifndef USE_XNN_DELEGATE
#define USE_XNN_DELEGATE 1
#endif

#if USE_XNN_DELEGATE
#include "tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h"
#endif

#define USE_GPU_DELEGATE 0

#if USE_GPU_DELEGATE
#include "tensorflow/lite/delegates/gpu/delegate.h"
#endif

namespace cute {

class CuteModel::Impl {
 public:
  Impl() = default;

  void loadBuffer(const void *buffer, std::size_t buffer_size) {
    model.reset(TfLiteModelCreate(buffer, buffer_size));
    options.reset(TfLiteInterpreterOptionsCreate());
  }

  void loadFile(const std::string& path) {
    model.reset(TfLiteModelCreateFromFile(path.c_str()));
    options.reset(TfLiteInterpreterOptionsCreate());
  }

  void setNumThreads(int num) {
#if USE_XNN_DELEGATE
    xnn_options.num_threads = num;
#endif
    TfLiteInterpreterOptionsSetNumThreads(options.get(), num);
  }

  // TODO(?): pass gpu options as parameters
  void setUseGPU() {
    #if USE_GPU_DELEGATE
      gpuDelegateOptionsV2 = TfLiteGpuDelegateOptionsV2Default();
      gpuDelegate.reset(TfLiteGpuDelegateV2Create(&gpuDelegateOptionsV2));
      TfLiteInterpreterOptionsAddDelegate(options.get(), gpuDelegate.get());
    #endif
  }

  void build() {
#if USE_XNN_DELEGATE
    xnn_delegate.reset(TfLiteXNNPackDelegateCreate(&xnn_options));
    TfLiteInterpreterOptionsAddDelegate(options.get(), xnn_delegate.get());
#endif
    interpreter.reset(TfLiteInterpreterCreate(model.get(), options.get()));
    if (interpreter == nullptr) return;
    TfLiteInterpreterAllocateTensors(interpreter.get());
  }

  bool isBuilt() const noexcept {
    return interpreter != nullptr;
  }

  void setInput(int index, const void* data) {
    TfLiteTensorCopyFromBuffer(inputTensor(index), data, TfLiteTensorByteSize(inputTensor(index)));
  }

  void invoke() {
    TfLiteInterpreterInvoke(interpreter.get());
  }

  void copyOutput(int index, void* dst) {
    TfLiteTensorCopyToBuffer(outputTensor(index), dst, TfLiteTensorByteSize(outputTensor(index)));
  }

  auto inputTensorCount() const {
    return TfLiteInterpreterGetInputTensorCount(interpreter.get());
  }

  auto outputTensorCount() const {
    return TfLiteInterpreterGetOutputTensorCount(interpreter.get());
  }

  TfLiteTensor* inputTensor(int index) {
    return TfLiteInterpreterGetInputTensor(interpreter.get(), index);
  }

  const TfLiteTensor* inputTensor(int index) const {
    return TfLiteInterpreterGetInputTensor(interpreter.get(), index);
  }

  const TfLiteTensor* outputTensor(int index) const {
    return TfLiteInterpreterGetOutputTensor(interpreter.get(), index);
  }

  std::vector<int> inputTensorDims(int index) const {
    auto tensor = inputTensor(index);
    std::vector<int> dims(TfLiteTensorNumDims(tensor));
    for (int i = 0; i < TfLiteTensorNumDims(tensor); ++i)
      dims[i] = TfLiteTensorDim(tensor, i);
    return dims;
  }

  std::vector<int> outputTensorDims(int index) const {
    auto tensor = outputTensor(index);
    std::vector<int> dims(TfLiteTensorNumDims(tensor));
    for (int i = 0; i < TfLiteTensorNumDims(tensor); ++i)
      dims[i] = TfLiteTensorDim(tensor, i);
    return dims;
  }

  std::size_t outputBytes(int index) const {
    return TfLiteTensorByteSize(outputTensor(index));
  }


  std::string summarize() const {
    if (interpreter == nullptr)
      return "Interpreter is not built.";

    static const auto getTensorInfo = [](const TfLiteTensor* tensor) -> std::string {
      std::stringstream log;

      log << TfLiteTensorName(tensor) << ' ';
      log << TfLiteTensorByteSize(tensor) << ' ';
      log << TfLiteTypeGetName(TfLiteTensorType(tensor)) << ' ';

      if (TfLiteTensorNumDims(tensor) == 0) {
        log << "None";
      } else {
        log << TfLiteTensorDim(tensor, 0);
        for (int i = 1; i < TfLiteTensorNumDims(tensor); ++i)
          log << 'x' << TfLiteTensorDim(tensor, i);
      }

      return log.str();
    };

    std::stringstream log;

    log << " Input Tensor\n";
    log << " Number / Name / Byte / Type / Size\n";
    for (int i = 0; i < inputTensorCount(); ++i) {
      log <<  "  #" << i << ' ' << getTensorInfo(this->inputTensor(i)) << '\n';
      const auto q = TfLiteTensorQuantizationParams(inputTensor(i));
      log << "\tScale: " << q.scale << ", Zero point: " << q.zero_point << '\n';
      log << "\tQuantization: " << inputTensor(i)->quantization.type << '\n';
    }
    log << '\n';


    log << " Output Tensor\n";
    log << " Number / Name / Byte / Type / Size\n";
    for (int i = 0; i < outputTensorCount(); ++i) {
      log << "  #" << i << ' ' << getTensorInfo(this->outputTensor(i)) << '\n';
      const auto q = TfLiteTensorQuantizationParams(outputTensor(i));
      log << "\tScale: " << q.scale << ", Zero point: " << q.zero_point << '\n';
      log << "\tQuantization: " << outputTensor(i)->quantization.type << '\n';
    }


    return log.str();
  }

 private:
  std::unique_ptr<TfLiteModel, decltype(&TfLiteModelDelete)> model{nullptr, TfLiteModelDelete};
  std::unique_ptr<TfLiteInterpreterOptions, decltype(&TfLiteInterpreterOptionsDelete)> options{nullptr, TfLiteInterpreterOptionsDelete};
  std::unique_ptr<TfLiteInterpreter, decltype(&TfLiteInterpreterDelete)> interpreter{nullptr, &TfLiteInterpreterDelete};

#if USE_XNN_DELEGATE
  TfLiteXNNPackDelegateOptions xnn_options = TfLiteXNNPackDelegateOptionsDefault();
  std::unique_ptr<TfLiteDelegate, decltype(&TfLiteXNNPackDelegateDelete)> xnn_delegate{nullptr, &TfLiteXNNPackDelegateDelete};
#endif

  #if USE_GPU_DELEGATE
    /** Tensorflow Lite GPU delegate members */
    TfLiteGpuDelegateOptionsV2 gpuDelegateOptionsV2{};
    std::unique_ptr<TfLiteDelegate, decltype(&TfLiteGpuDelegateV2Delete)> gpuDelegate{nullptr, TfLiteGpuDelegateV2Delete};
  #endif
};

} // namespace cute

#endif // SEESO_CORE_CORE_GAZE_TRACKER_NEURAL_ENGINE_CUTEMODEL_IMPL_C_VERSION_H_
