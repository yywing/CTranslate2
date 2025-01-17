# Quantization

Quantization is a technique that can reduce the model size and accelerate its execution with little to no degradation in accuracy. CTranslate2 supports the most common types:

* 8-bit integers (INT8)
* 16-bit integers (INT16)
* 16-bit floating points (FP16)

See the [benchmark results](../README.md#benchmarks) in the main README for a performance and memory usage comparison.

## Enabling quantization

Quantization can be enabled when converting the model or when loading the model.

### When converting the model

Enabling the quantization during conversion is helpful to reduce the model size on disk. The converters expose the option `quantization` that accepts the following values:

* `int8`
* `int8_float16`
* `int16`
* `float16`

For example,

```bash
ct2-opennmt-py-converter --model_path model.pt --quantization int8 --output_dir ct2_model
```

Whatever quantization type is selected here, the runtime ensures the model can be loaded and executed efficiently. This implies the model weights are possibly converted to another type when the model is loaded (see next section).

### When loading the model

Quantization can also be enabled or changed when loading the model. The translator exposes the option `compute_type` that accepts the following values:

* `default`: see description below
* `auto`: selects the fastest computation type
* `int8`
* `int8_float16`
* `int16`
* `float16`
* `float`

Conversions between all types are supported. For example, you can convert a model with `quantization="int8"` and then execute in full precision with `compute_type="float"`.

#### Default loading behavior

By default, the runtime tries to use the type that is saved in the converted model as the computation type. However, if the current platform or backend do not support optimized execution for this computation type (e.g. `int16` is not optimized on GPU), then the library converts the model weights to another optimized type.  The tables below document the fallback types in prebuilt binaries:

**On CPU:**

| CPU vendor | int8 | int8_float16 | int16 | float16 |
| --- | --- | --- | --- | --- |
| Intel | int8 | int8 | int16 | float |
| other | int8 | int8 | int8 | float |

**On GPU:**

| GPU Compute Capability | int8 | int8_float16 | int16 | float16 |
| --- | --- | --- | --- | --- |
| >= 7.0 | int8 | int8_float16 | float16 | float16 |
| 6.1 | int8 | int8 | float | float |
| <= 6.0 | float | float | float | float |

You can get more information about the detected capabilities of your system by setting the environment variable `CT2_VERBOSE=1`.

## Quantized types

### 8-bit integers (`int8`)

**Supported on:**

* NVIDIA GPU with Compute Capability >= 6.1
* x86-64 CPU with the Intel MKL or oneDNN backends

The implementation applies the equation from [Wu et al. 2016](https://arxiv.org/abs/1609.08144) to compute the quantized weights:

![INT8 quantization equation](_static/int8_equation.png)

Note that this corresponds to a symmetric quantization (absolute maximum of the input range instead of separate min/max values). We only quantize the weights of the embedding and linear layers.

### 16-bit integers (`int16`)

**Supported on:**

* Intel CPU with the Intel MKL backend

The implementation follows the work by [Devlin 2017](https://arxiv.org/abs/1705.01991). By default we use one quantization scale per layer. The scale is defined as:

```text
scale = 2^10 / max(abs(W))
```

As suggested by the author, the idea is to use 10 bits for the input so that the multiplication is 20 bits which gives 12 bits left for accumulation. We only quantize the weights of the embedding and linear layers.

### 16-bit floating points (`float16`)

**Supported on:**

* NVIDIA GPU with Compute Capability >= 7.0

In this mode, all model weights are stored in half precision and all layers are run in half precision.

### Mixed 8-bit integers and 16-bit floating points (`int8_float16`)

**Supported on:**

* NVIDIA GPU with Compute Capability >= 7.0

This mode is the same as `int8`, but all non quantized layers are run in FP16 instead of FP32.
