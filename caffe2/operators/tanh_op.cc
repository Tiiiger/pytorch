#include <cmath>

#include "caffe2/operators/elementwise_op.h"
#include "caffe2/utils/math.h"

namespace caffe2 {

struct TanhCPUFunctor {
  template <typename T>
  inline void
  operator()(const int n, const T* x, T* y, CPUContext* /*device_context*/) {
#ifdef CAFFE2_USE_ACCELERATE
    vvtanhf(y, x, &n);
#else
    ConstEigenVectorArrayMap<T> x_arr(x, n);
    EigenVectorMap<T>(y, n) = 1 - 2 * ((x_arr * 2).exp() + 1).inverse();
#endif
  }
};

struct TanhGradientCPUFunctor {
  template <typename T>
  inline void Run(
      const int n,
      const T* y,
      const T* dy,
      T* dx,
      CPUContext* /*device_context*/) {
    ConstEigenVectorArrayMap<T> dy_arr(dy, n);
    ConstEigenVectorArrayMap<T> y_arr(y, n);
    EigenVectorMap<T>(dx, n) = dy_arr * (1 - y_arr * y_arr);
  }
};

REGISTER_CPU_OPERATOR(
    Tanh, UnaryElementwiseOp<TensorTypes<float>, CPUContext, TanhCPUFunctor>);
REGISTER_CPU_OPERATOR(
    TanhGradient,
    BinaryElementwiseOp<
        TensorTypes<float>,
        CPUContext,
        WithoutBroadcast<TanhGradientCPUFunctor>>);

OPERATOR_SCHEMA(Tanh)
  .NumInputs(1)
  .NumOutputs(1)
  .AllowInplace({{0, 0}})
  .IdenticalTypeAndShape()
  .SetDoc(R"DOC(
Calculates the hyperbolic tangent of the given input tensor element-wise. This
operation can be done in an in-place fashion too, by providing the same input
and output blobs.

Github Links:

- https://github.com/pytorch/pytorch/blob/master/caffe2/operators/tanh_op.cc


<details>

<summary> <b>Example</b> </summary>

**Code**

```

workspace.ResetWorkspace()

op = core.CreateOperator(
    "Tanh",
    ["X"],
    ["X"],
)

workspace.FeedBlob("X", np.random.randn(3, 3).astype(np.float32))
print("X:\n", workspace.FetchBlob("X"), "\n")

workspace.RunOperatorOnce(op)
print("X:\n", workspace.FetchBlob("X"))

```

**Result**

```

X:
 [[ 2.032603   -2.3556721  -0.14955314]
 [ 0.39309832 -1.1020128  -0.92951244]
 [-0.62815386  0.21342885  1.4002231 ]]

X:
 [[ 0.9662601  -0.982175   -0.14844811]
 [ 0.3740282  -0.8012209  -0.73036647]
 [-0.55677974  0.21024609  0.8853999 ]]

```

</details>

)DOC")
  .Input(0, "input", "1-D input tensor")
  .Output(0, "output", "The hyperbolic tangent values of the input tensor, computed element-wise")
  .InheritOnnxSchema("Tanh");

OPERATOR_SCHEMA(TanhGradient).NumInputs(2).NumOutputs(1).AllowInplace({{1, 0}});

class GetTanhGradient : public GradientMakerBase {
  using GradientMakerBase::GradientMakerBase;
  vector<OperatorDef> GetGradientDefs() override {
    return SingleGradientDef(
        "TanhGradient", "",
        std::vector<string>{O(0), GO(0)},
        std::vector<string>{GI(0)});
  }
};
REGISTER_GRADIENT(Tanh, GetTanhGradient);
}  // namespace caffe2
