#include <ATen/native/TensorIndexing.h>

#include <c10/util/Exception.h>

namespace at {
namespace indexing {

const EllipsisIndexType Ellipsis = EllipsisIndexType();

namespace impl {
std::ostream& operator<<(std::ostream& stream, const Slice& slice) {
  stream << slice.start() << ":" << slice.stop() << ":" << slice.step();
  return stream;
}
} // namespace impl

std::ostream& operator<<(std::ostream& stream, const TensorIndex& tensor_index) {
  if (tensor_index.is_none()) {
    stream << "None";
  } else if (tensor_index.is_ellipsis()) {
    stream << "...";
  } else if (tensor_index.is_integer()) {
    stream << tensor_index.integer();
  } else if (tensor_index.is_boolean()) {
    stream << std::boolalpha << tensor_index.boolean();
  } else if (tensor_index.is_slice()) {
    stream << tensor_index.slice();
  } else if (tensor_index.is_tensor()) {
    stream << tensor_index.tensor();
  }
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const std::vector<TensorIndex>& tensor_indices) {
  stream << "(";
  for (size_t i = 0; i < tensor_indices.size(); i++) {
    stream << tensor_indices[i];
    if (i < tensor_indices.size() - 1) stream << ", ";
  }
  stream << ")";
  return stream;
}

// This mirrors `THPVariable_setitem` in torch/csrc/autograd/python_variable_indexing.cpp
// for "the assigned value is a Scalar" case
static inline void set_item(Tensor& self, ArrayRef<TensorIndex> indices, Scalar v) {
  Tensor value;

  {
    at::AutoNonVariableTypeMode guard;
    // TODO: This qint special case looks very suspicious...
    if (isQIntType(self.scalar_type())) {
      value = at::indexing::scalarToTensor(v, device(kCPU).dtype(kFloat), at::Device(kCPU));
    } else {
      value = at::indexing::scalarToTensor(v, self.options(), self.device());
    }
  }

  return set_item(self, indices, value);
}

} // namespace indexing

Tensor Tensor::index_tmp(ArrayRef<at::indexing::TensorIndex> indices) const {
  OptionalDeviceGuard device_guard(device_of(*this));
  return at::indexing::get_item(*this, indices);
}
Tensor Tensor::index_tmp(std::initializer_list<at::indexing::TensorIndex> indices) const {
  return index_tmp(ArrayRef<at::indexing::TensorIndex>(indices));
}

Tensor & Tensor::index_put_tmp_(ArrayRef<at::indexing::TensorIndex> indices, Tensor const & rhs) {
  OptionalDeviceGuard device_guard(device_of(*this));
  at::indexing::set_item(*this, indices, rhs);
  return *this;
}
Tensor & Tensor::index_put_tmp_(ArrayRef<at::indexing::TensorIndex> indices, Scalar v) {
  OptionalDeviceGuard device_guard(device_of(*this));
  at::indexing::set_item(*this, indices, v);
  return *this;
}
Tensor & Tensor::index_put_tmp_(std::initializer_list<at::indexing::TensorIndex> indices, Tensor const & rhs) {
  return index_put_tmp_(ArrayRef<at::indexing::TensorIndex>(indices), rhs);
}
Tensor & Tensor::index_put_tmp_(std::initializer_list<at::indexing::TensorIndex> indices, Scalar v) {
  return index_put_tmp_(ArrayRef<at::indexing::TensorIndex>(indices), v);
}

} // namespace at
