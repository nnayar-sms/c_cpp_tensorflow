load("//tensorflow:strict.default.bzl", "py_strict_library")

package(
    # copybara:uncomment default_applicable_licenses = ["//tensorflow:license"],
    licenses = ["notice"],
)

config_setting(
    name = "tflite_convert_with_select_tf_ops",
    define_values = {"tflite_convert_with_select_tf_ops": "true"},
    visibility = ["//visibility:private"],
)

filegroup(
    name = "toco_python_api_hdrs",
    srcs = [
        "toco_python_api.h",
    ],
    visibility = [
        "//tensorflow/python:__subpackages__",
    ],
)

cc_library(
    name = "toco_python_api",
    srcs = ["toco_python_api.cc"],
    hdrs = ["toco_python_api.h"],
    features = ["-parse_headers"],
    visibility = [
        "//tensorflow/python:__subpackages__",
    ],
    deps = [
        "//tensorflow/c:kernels",
        "//tensorflow/c:tf_status_headers",
        "//tensorflow/core:framework",
        "//tensorflow/core:lib",
        "//tensorflow/core:protos_all_cc",
        "//tensorflow/lite:model_builder",
        "//tensorflow/lite/core/api",
        "//tensorflow/lite/core/c:common",
        "//tensorflow/lite/python/interpreter_wrapper:python_error_reporter",
        "//tensorflow/lite/python/interpreter_wrapper:python_utils",
        "//tensorflow/lite/schema:schema_fbs",
        "//tensorflow/lite/toco:model",
        "//tensorflow/lite/toco:model_flags_proto_cc",
        "//tensorflow/lite/toco:toco_convert",
        "//tensorflow/lite/toco:toco_flags_proto_cc",
        "//tensorflow/lite/toco:toco_graphviz_dump_options",
        "//tensorflow/lite/toco:toco_port",
        "//tensorflow/lite/toco:toco_tooling",
        "//tensorflow/lite/toco:tooling_util",
        "//tensorflow/lite/toco:types_proto_cc",
        "//tensorflow/lite/toco/logging:conversion_log_util",
        "//tensorflow/lite/toco/logging:toco_conversion_log_proto_cc",
        "//third_party/python_runtime:headers",  # build_cleaner: keep; DNR: b/35864863
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log:absl_log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:string_view",
        "@com_google_protobuf//:protobuf_headers",
        "@flatbuffers//:runtime_cc",
        "@local_tsl//tsl/platform:status",
    ] + select({
        # This is required when running `tflite_convert` from `bazel`.
        # It requires to link with TensorFlow Ops to get the op definitions.
        ":tflite_convert_with_select_tf_ops": [
            "//tensorflow/core:ops",
        ],
        "//conditions:default": [],
    }),
    alwayslink = True,
)

# Compatibility stub. Remove when internal customers moved.
py_strict_library(
    name = "tensorflow_wrap_toco",
    srcs = ["tensorflow_wrap_toco.py"],
    visibility = ["//learning/expander/pod/deep_pod/utils:__subpackages__"],
    deps = [
        "//tensorflow/python:_pywrap_toco_api",
    ],
)
