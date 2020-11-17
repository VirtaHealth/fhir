// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/fhir/resource_validation.h"

#include "google/protobuf/any.pb.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/descriptor.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "google/fhir/annotations.h"
#include "google/fhir/fhir_types.h"
#include "google/fhir/primitive_handler.h"
#include "google/fhir/proto_util.h"
#include "google/fhir/status/status.h"
#include "google/fhir/status/statusor.h"
#include "google/fhir/util.h"
#include "proto/annotations.pb.h"

namespace google {
namespace fhir {

using ::absl::FailedPreconditionError;
using ::google::fhir::proto::validation_requirement;
using ::google::protobuf::Descriptor;
using ::google::protobuf::FieldDescriptor;
using ::google::protobuf::Message;
using ::google::protobuf::Reflection;

namespace {

Status CheckField(const Message& message, const FieldDescriptor* field,
                  const std::string& field_name,
                  const PrimitiveHandler* primitive_handler);

Status ValidateFhirConstraints(const Message& message,
                               const std::string& base_name,
                               const PrimitiveHandler* primitive_handler) {
  if (IsPrimitive(message.GetDescriptor())) {
    return primitive_handler->ValidatePrimitive(message).ok()
               ? absl::OkStatus()
               : FailedPreconditionError(
                     absl::StrCat("invalid-primitive-", base_name));
  }

  if (IsMessageType<::google::protobuf::Any>(message)) {
    // We do not validate "Any" contained resources.
    // TODO: Potentially unpack the correct type and validate?
    return absl::OkStatus();
  }

  const Descriptor* descriptor = message.GetDescriptor();
  const Reflection* reflection = message.GetReflection();

  for (int i = 0; i < descriptor->field_count(); i++) {
    const FieldDescriptor* field = descriptor->field(i);
    const std::string& field_name =
        absl::StrCat(base_name, ".", field->json_name());
    FHIR_RETURN_IF_ERROR(
        CheckField(message, field, field_name, primitive_handler));
  }
  // Also verify that oneof fields are set.
  // Note that optional choice-types should have the containing message unset -
  // if the containing message is set, it should have a value set as well.
  for (int i = 0; i < descriptor->oneof_decl_count(); i++) {
    const ::google::protobuf::OneofDescriptor* oneof = descriptor->oneof_decl(i);
    if (!reflection->HasOneof(message, oneof) &&
        !oneof->options().GetExtension(
            ::google::fhir::proto::fhir_oneof_is_optional)) {
      FHIR_RETURN_IF_ERROR(::absl::FailedPreconditionError(
          absl::StrCat("empty-oneof-", oneof->full_name())));
    }
  }
  return absl::OkStatus();
}

// Check if a required field is missing.
Status CheckField(const Message& message, const FieldDescriptor* field,
                  const std::string& field_name,
                  const PrimitiveHandler* primitive_handler) {
  if (field->options().HasExtension(validation_requirement) &&
      field->options().GetExtension(validation_requirement) ==
          ::google::fhir::proto::REQUIRED_BY_FHIR) {
    if (!FieldHasValue(message, field)) {
      return FailedPreconditionError(absl::StrCat("missing-", field_name));
    }
  }

  if (IsReference(field->message_type())) {
    auto status = primitive_handler->ValidateReferenceField(message, field);
    return status.ok() ? status
                       : FailedPreconditionError(absl::StrCat(
                             status.message(), "-at-", field_name));
  }

  if (field->cpp_type() == ::google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
    for (int i = 0; i < PotentiallyRepeatedFieldSize(message, field); i++) {
      const auto& submessage = GetPotentiallyRepeatedMessage(message, field, i);
      FHIR_RETURN_IF_ERROR(
          ValidateFhirConstraints(submessage, field_name, primitive_handler));
    }
  }

  return absl::OkStatus();
}

}  // namespace

// TODO: Invert the default here for FHIRPath handling, and have
// ValidateWithoutFhirPath instead of ValidateWithFhirPath

Status ValidateResourceWithFhirPath(
    const Message& resource, const PrimitiveHandler* primitive_handler,
    fhir_path::FhirPathValidator* message_validator) {
  FHIR_RETURN_IF_ERROR(ValidateFhirConstraints(
      resource, resource.GetDescriptor()->name(), primitive_handler));
  return message_validator->Validate(resource).LegacyValidationResult();
}

Status ValidateResource(const Message& resource,
                        const PrimitiveHandler* primitive_handler) {
  return ValidateFhirConstraints(resource, resource.GetDescriptor()->name(),
                                 primitive_handler);
}

}  // namespace fhir
}  // namespace google
