package(default_visibility = ["//visibility:public"])

licenses(["notice"])

load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

go_library(
    name = "accessor",
    srcs = ["getter_setter.go"],
    importpath = "google/fhir/jsonformat/internal/accessor/accessor",
    deps = [
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_pkg_errors//:go_default_library",
        "@org_golang_google_protobuf//reflect/protoreflect:go_default_library",
    ],
)

go_test(
    name = "accessor_test",
    size = "small",
    srcs = ["getter_setter_test.go"],
    embed = [":accessor"],
    deps = [
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_google_go_cmp//cmp:go_default_library",
        "@com_google_fhir//proto:version_config_go_proto",
        "@com_google_fhir//proto/stu3:datatypes_go_proto",
        "@com_google_fhir//proto/stu3:resources_go_proto",
        "@org_golang_google_protobuf//testing/protocmp:go_default_library",
    ],
)
