//
// Created by YongGyu Lee on 2020-03-26.
//

#include "cute_model.h"

#include "impl/c_version.h"

namespace cute {

CuteModel::CuteModel() : pImpl(nullptr) {
  pImpl = new Impl();
}

CuteModel::~CuteModel() {
  delete pImpl;
}

CuteModel& CuteModel::loadBuffer(const void *buffer, std::size_t buffer_size) & {
  pImpl->loadBuffer(buffer, buffer_size);
  return *this;
}

CuteModel& CuteModel::loadFile(const std::string& path) & {
  pImpl->loadFile(path);
  return *this;
}

CuteModel &CuteModel::setNumThreads(int num) & {
  pImpl->setNumThreads(num);
  return *this;
}

CuteModel& CuteModel::setUseGPU(bool use) & {
  if (use)
    pImpl->setUseGPU();
  return *this;
}

void CuteModel::build() {
  return pImpl->build();
}

bool CuteModel::isBuilt() const {
  return pImpl->isBuilt();
}

void CuteModel::setInputInner(int index, const void *data) {
  return pImpl->setInput(index, data);
}

void CuteModel::invoke() {
  input_index = 0;
  return pImpl->invoke();
}

void CuteModel::copyOutput(int index, void *dst) const {
  return pImpl->copyOutput(index, dst);
}

TfLiteTensor* CuteModel::inputTensor(int index) {
  return pImpl->inputTensor(index);
}

const TfLiteTensor* CuteModel::inputTensor(int index) const {
  return pImpl->inputTensor(index);
}

const TfLiteTensor* CuteModel::outputTensor(int index) const {
  return pImpl->outputTensor(index);
}

std::vector<int> CuteModel::inputTensorDims(int index) const {
  return pImpl->inputTensorDims(index);
}

std::vector<int> CuteModel::outputTensorDims(int index) const {
  return pImpl->outputTensorDims(index);
}

std::size_t CuteModel::outputBytes(int index) const {
  return pImpl->outputBytes(index);
}

std::string CuteModel::summarize() const {
  return pImpl->summarize();
}
size_t CuteModel::inputTensorCount() const {
  return pImpl->inputTensorCount();
}
size_t CuteModel::outputTensorCount() const {
  return pImpl->outputTensorCount();
}

} // namespace cute
