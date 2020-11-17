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

#include "google/fhir/seqex/bundle_to_seqex_converter.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "google/protobuf/text_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/flags/declare.h"
#include "absl/flags/flag.h"
#include "absl/strings/substitute.h"
#include "google/fhir/r4/primitive_handler.h"
#include "google/fhir/seqex/converter_types.h"
#include "google/fhir/seqex/example_key.h"
#include "google/fhir/seqex/r4.h"
#include "google/fhir/seqex/stu3.h"
#include "google/fhir/seqex/text_tokenizer.h"
#include "google/fhir/test_helper.h"
#include "google/fhir/testutil/fhir_test_env.h"
#include "google/fhir/testutil/proto_matchers.h"
#include "google/fhir/type_macros.h"
#include "tensorflow/core/example/example.pb.h"
#include "tensorflow/core/platform/env.h"

ABSL_DECLARE_FLAG(bool, tokenize_code_text_features);

namespace google {
namespace fhir {
namespace seqex {

namespace {

using google::fhir::proto::VersionConfig;
using ::google::fhir::testutil::EqualsProto;
using ::google::fhir::testutil::EqualsProtoIgnoringReordering;
using ::tensorflow::SequenceExample;

struct Stu3ConverterTestEnv : public testutil::Stu3CoreTestEnv,
                              public seqex_stu3::ConverterTypes {
  using BundleToSeqexConverter = seqex_stu3::UnprofiledBundleToSeqexConverter;

  constexpr static auto& observation_part_of_field() { return "basedOn"; }
  constexpr static auto& encounter_reason_code_field() { return "reason"; }
  constexpr static auto& condition_recorded_date() { return "assertedDate"; }

  constexpr static auto& config_path() {
    return "/com_google_fhir/proto/stu3/version_config.textproto";
  }

  template <typename Resource>
  static void AddContainedResource(const google::protobuf::Message& contained,
                                   Resource* resource) {
    CHECK(SetContainedResource(contained, resource->add_contained()).ok());
  }
};

struct R4ConverterTestEnv : public testutil::R4CoreTestEnv,
                            public seqex_r4::ConverterTypes {
  using BundleToSeqexConverter = seqex_r4::UnprofiledBundleToSeqexConverter;

  constexpr static auto& observation_part_of_field() { return "partOf"; }
  constexpr static auto& encounter_reason_code_field() { return "reasonCode"; }
  constexpr static auto& condition_recorded_date() { return "recordedDate"; }
  constexpr static auto& config_path() {
    return "/com_google_fhir/proto/r4/version_config.textproto";
  }

  template <typename Resource>
  static void AddContainedResource(const google::protobuf::Message& child,
                                   Resource* parent) {
    using ContainedResource = BUNDLE_CONTAINED_RESOURCE(Bundle);
    ContainedResource contained_resource;
    CHECK(SetContainedResource(child, &contained_resource).ok());
    parent->add_contained()->PackFrom(contained_resource);
  }
};

template <typename FhirTestEnv>
class BundleToSeqexConverterTest : public ::testing::Test {
 public:
  void SetUp() override {
    TF_CHECK_OK(::tensorflow::ReadTextProto(
        ::tensorflow::Env::Default(),
        absl::StrCat(getenv("TEST_SRCDIR"), FhirTestEnv::config_path()),
        &fhir_version_config_));
    // Reset command line flags to default values between tests.
    absl::SetFlag(&FLAGS_tokenize_code_text_features, true);
    absl::SetFlag(&FLAGS_trigger_time_redacted_features, "");
    tokenizer_ = TextTokenizer::FromFlags();
  }

  void PerformTest(const std::string& input_key,
                   const typename FhirTestEnv::Bundle& bundle,
                   const std::vector<typename FhirTestEnv::TriggerLabelsPair>&
                       trigger_labels_pair,
                   const std::map<std::string, SequenceExample>& expected) {
    // Until all config options for this object can be passed as args, we need
    // to initialize it after overriing the flags settings.
    typename FhirTestEnv::BundleToSeqexConverter converter(
        this->fhir_version_config_, this->tokenizer_,
        false /* enable_attribution */, false /* generate_sequence_label */);
    std::map<std::string, int> counter_stats;
    ASSERT_TRUE(converter.Begin(input_key, bundle, trigger_labels_pair,
                                &counter_stats));
    for (const auto& iter : expected) {
      EXPECT_EQ(converter.ExampleKey(), iter.first);
      EXPECT_THAT(converter.GetExample(),
                  EqualsProtoIgnoringReordering(iter.second))
          << "\nfor key: " << converter.ExampleKey();
      ASSERT_TRUE(converter.Next());
    }

    ASSERT_TRUE(converter.Done())
        << "key: " << converter.ExampleKey()
        << "\nvalue: " << converter.GetExample().DebugString();
  }

 protected:
  VersionConfig fhir_version_config_;
  std::shared_ptr<TextTokenizer> tokenizer_;
  google::protobuf::TextFormat::Parser parser_;
};

using TestEnvs = ::testing::Types<Stu3ConverterTestEnv, R4ConverterTestEnv>;
TYPED_TEST_SUITE(BundleToSeqexConverterTest, TestEnvs);

TYPED_TEST(BundleToSeqexConverterTest, TestMultipleResources) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time {
                                        value_us: 1420102800000000
                                        precision: SECOND
                                        timezone: "America/New_York"
                                      }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            entry {
              resource {
                patient {
                  id { value: "14" }
                  birth_date {
                    value_us: -1323388800000000
                    precision: DAY
                    timezone: "America/New_York"
                  }
                }
              }
            }
            entry {
              resource {
                condition {
                  id { value: "1" }
                  subject { patient_id { value: "14" } }
                  code {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "bar" }
                    }
                  }
                  $1 {
                    value_us: 1417392000000000  # "2014-12-01T00:00:00+00:00"
                  }
                }
              }
            }
            entry {
              resource {
                condition {
                  id { value: "2" }
                  subject { patient_id { value: "14" } }
                  code {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "baz" }
                    }
                  }
                  $1 {
                    value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                  }
                }
              }
            }
            entry {
              resource {
                composition {
                  id { value: "1" }
                  subject { patient_id { value: "14" } }
                  encounter { encounter_id { value: "1" } }
                  section { text { div { value: "test text" } } }
                  date {
                    value_us: 1420102800000000
                    timezone: "UTC"
                    precision: SECOND
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "1" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $2 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "191.4" }
                      display { value: "Malignant neoplasm of occipital lobe" }
                    }
                  }
                  period {
                    start {
                      value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1420102800000000  # "2015-01-01T09:00:00+00:00"
                    }
                  }
                }
              }
            }
          )proto",
          ToSnakeCase(TypeParam::observation_part_of_field()),
          ToSnakeCase(TypeParam::condition_recorded_date()),
          ToSnakeCase(TypeParam::encounter_reason_code_field())),
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            context: {
              feature: {
                key: "Patient.birthDate"
                value { int64_list { value: -1323388800 } }
              }
              feature {
                key: "currentEncounterId"
                value { int64_list { value: 1420099200 } }
              }
              feature {
                key: "patientId"
                value { bytes_list { value: "14" } }
              }
              feature {
                key: "sequenceLength"
                value { int64_list { value: 5 } }
              }
              feature {
                key: "timestamp"
                value { int64_list { value: 1420102800 } }
              }
            }
            feature_lists: {
              feature_list {
                key: "Composition.date"
                value {
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420102800 } }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "Composition.meta.lastUpdated"
                value {
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420102800 } }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "Composition.section.text.div.tokenized"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list { value: "test" value: "text" } }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Condition.meta.lastUpdated"
                value {
                  feature { int64_list { value: 1417392000 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "Condition.code.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
                value {
                  feature { bytes_list { value: "bar" } }
                  feature { bytes_list { value: "baz" } }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Condition.$1"
                value {
                  feature { int64_list { value: 1417392000 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "Encounter.meta.lastUpdated"
                value {
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.class"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature { bytes_list {} }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                }
              }
              feature_list {
                key: "Encounter.period.end"
                value {
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.period.start"
                value {
                  feature { int64_list {} }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "Encounter.$2.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list { value: "191.4" } }
                }
              }
              feature_list {
                key: "Encounter.$2.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.tokenized"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                  feature {
                    bytes_list {
                      value: "malignant"
                      value: "neoplasm"
                      value: "of"
                      value: "occipital"
                      value: "lobe"
                    }
                  }
                }
              }
              feature_list {
                key: "encounterId"
                value {
                  feature { int64_list { value: 1417392000 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "eventId"
                value {
                  feature { int64_list { value: 1417392000 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420102800 } }
                  feature { int64_list { value: 1420102800 } }
                }
              }
            })proto",
          TypeParam::observation_part_of_field(),
          TypeParam::condition_recorded_date(),
          TypeParam::encounter_reason_code_field()),
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-5@1420102800:Encounter/1", seqex}});
}

// Test the case where multiple triggers have the exact same timestamp, but are
// associated with different source encounters.
TYPED_TEST(BundleToSeqexConverterTest, MultipleLabelsSameTimestamp) {
  typename TypeParam::EventTrigger trigger1;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420099200000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger1));
  typename TypeParam::EventTrigger trigger2;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420101000000000 }
                                      source { encounter_id { value: "3" } }
                                    )proto",
                                    &trigger2));
  typename TypeParam::EventTrigger trigger3;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420099200000000 }
                                      source { encounter_id { value: "2" } }
                                    )proto",
                                    &trigger3));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger1, {}}, {trigger2, {}}, {trigger3, {}}});

  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            entry {
              resource {
                patient {
                  id { value: "14" }
                  birth_date {
                    value_us: -1323388800000000
                    precision: DAY
                    timezone: "America/New_York"
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "1" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $0 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "191.4" }
                      display { value: "Malignant neoplasm of occipital lobe" }
                    }
                  }
                  period {
                    start {
                      value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1420102800000000  # "2015-01-01T09:00:00+00:00"
                    }
                  }
                }
              }
            })proto",
          ToSnakeCase(TypeParam::encounter_reason_code_field())),
      &bundle));

  std::string seqex_tmpl = R"(
      context: {
        feature { key: "Patient.birthDate" value { int64_list { value: -1323388800 } } }
        feature {
          key: "currentEncounterId"
          value { int64_list { value: 1420099200 } }
        }
        $0
        feature {
          key: "patientId"
          value { bytes_list { value: "14" } }
        }
        feature {
          key: "sequenceLength"
          value { int64_list { value: 1 } }
        }
        feature {
          key: "timestamp"
          value { int64_list { value: $1 } }
        }
      }
      feature_lists: {
        feature_list {
          key: "Encounter.meta.lastUpdated"
          value { feature { int64_list { value: 1420099200 } } }
        }
        feature_list {
          key: "Encounter.class"
          value { feature { bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" } } }
        }
        feature_list {
          key: "Encounter.period.end"
          value { feature { int64_list { } } }
        }
        feature_list {
          key: "Encounter.period.start"
          value { feature { int64_list { value: 1420099200 } } }
        }
        feature_list {
          key: "Encounter.$2.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
          value { feature { bytes_list { } } }
        }
        feature_list {
          key: "Encounter.$2.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.tokenized"
          value { feature { bytes_list { } } }
        }
        feature_list {
          key: "encounterId"
          value { feature { int64_list { value: 1420099200 } } }
        }
        feature_list {
          key: "eventId"
          value { feature { int64_list { value: 1420099200 } } }
        }
      })";
  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(seqex_tmpl, "", "1420099200",
                       TypeParam::encounter_reason_code_field()),
      &seqex));
  SequenceExample seqex2;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(seqex_tmpl, "", "1420101000",
                       TypeParam::encounter_reason_code_field()),
      &seqex2));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-1@1420099200:Encounter/1", seqex},
                     {"Patient/14:0-1@1420099200:Encounter/2", seqex},
                     {"Patient/14:0-1@1420101000:Encounter/3", seqex2}});
}

TYPED_TEST(BundleToSeqexConverterTest, TestClassLabel) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420444800000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              class_value {
                system { value: "http://hl7.org/fhir/v3/ActCode" }
                code { value: "IMP" }
              }
              period {
                start {
                  value_us: 1420444800000000  # "2015-01-05T08:00:00+00:00"
                }
                end {
                  value_us: 1420455600000000  # "2015-01-05T11:00:00+00:00"
                }
              }
            }
          }
        })proto",
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 1 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420444800 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "Encounter.class"
            value {
              feature {
                bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
              }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value { feature { int64_list {} } }
          }
          feature_list {
            key: "Encounter.period.start"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "encounterId"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "eventId"
            value { feature { int64_list { value: 1420444800 } } }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-1@1420444800:Encounter/1", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, TestBooleanLabelTrue) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420444800000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  typename TypeParam::EventLabel label;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        type {
          system { value: "test_boolean_system" }
          code { value: "test_boolean_label" }
        }
        event_time { value_us: 1420444800000000 }
        label { class_value { boolean { value: 1 } } }
      )proto",
      &label));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {label}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              class_value {
                system { value: "http://hl7.org/fhir/v3/ActCode" }
                code { value: "IMP" }
              }
              period {
                start {
                  value_us: 1420444800000000  # "2015-01-05T08:00:00+00:00"
                }
                end {
                  value_us: 1420455600000000  # "2015-01-05T11:00:00+00:00"
                }
              }
            }
          }
        })proto",
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 1 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_boolean_label.class"
            value {}
          }
          feature {
            key: "label.test_boolean_label.timestamp_secs"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_boolean_label.value_boolean"
            value { int64_list { value: 1 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "Encounter.class"
            value {
              feature {
                bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
              }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value { feature { int64_list {} } }
          }
          feature_list {
            key: "Encounter.period.start"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "encounterId"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "eventId"
            value { feature { int64_list { value: 1420444800 } } }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-1@1420444800:Encounter/1", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, TestBooleanLabelFalse) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420444800000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  typename TypeParam::EventLabel label;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        type {
          system { value: "test_boolean_system" }
          code { value: "test_boolean_label" }
        }
        event_time { value_us: 1420444800000000 }
        label { class_value { boolean { value: 0 } } }
      )proto",
      &label));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {label}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              class_value {
                system { value: "http://hl7.org/fhir/v3/ActCode" }
                code { value: "IMP" }
              }
              period {
                start {
                  value_us: 1420444800000000  # "2015-01-05T08:00:00+00:00"
                }
                end {
                  value_us: 1420455600000000  # "2015-01-05T11:00:00+00:00"
                }
              }
            }
          }
        })proto",
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 1 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_boolean_label.class"
            value {}
          }
          feature {
            key: "label.test_boolean_label.timestamp_secs"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_boolean_label.value_boolean"
            value { int64_list { value: 0 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "Encounter.class"
            value {
              feature {
                bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
              }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value { feature { int64_list {} } }
          }
          feature_list {
            key: "Encounter.period.start"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "encounterId"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "eventId"
            value { feature { int64_list { value: 1420444800 } } }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-1@1420444800:Encounter/1", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, TestClassNameWithClassValueBoolean) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420444800000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  typename TypeParam::EventLabel label;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      type {
                                        system { value: "test_system" }
                                        code { value: "code1" }
                                      }
                                      event_time { value_us: 1420444800000000 }
                                      label {
                                        class_name { code { value: "value1" } }
                                        class_value { boolean { value: 0 } }
                                      }
                                      label {
                                        class_name { code { value: "value2" } }
                                        class_value { boolean { value: 1 } }
                                      }
                                    )proto",
                                    &label));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {label}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              class_value {
                system { value: "http://hl7.org/fhir/v3/ActCode" }
                code { value: "IMP" }
              }
              period {
                start {
                  value_us: 1420444800000000  # "2015-01-05T08:00:00+00:00"
                }
                end {
                  value_us: 1420455600000000  # "2015-01-05T11:00:00+00:00"
                }
              }
            }
          }
        })proto",
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 1 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.code1.class"
            value { bytes_list { value: "value1" value: "value2" } }
          }
          feature {
            key: "label.code1.timestamp_secs"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.code1.value_boolean"
            value { int64_list { value: 0 value: 1 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "Encounter.class"
            value {
              feature {
                bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
              }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value { feature { int64_list {} } }
          }
          feature_list {
            key: "Encounter.period.start"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "encounterId"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "eventId"
            value { feature { int64_list { value: 1420444800 } } }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-1@1420444800:Encounter/1", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, TestBooleanLabelMultiple) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420444800000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  typename TypeParam::EventLabel label;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        type {
          system { value: "test_boolean_system" }
          code { value: "test_boolean_label" }
        }
        event_time { value_us: 1420444800000000 }
        label { class_value { boolean { value: 0 } } }
        label { class_value { boolean { value: 1 } } }
      )proto",
      &label));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {label}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              class_value {
                system { value: "http://hl7.org/fhir/v3/ActCode" }
                code { value: "IMP" }
              }
              period {
                start {
                  value_us: 1420444800000000  # "2015-01-05T08:00:00+00:00"
                }
                end {
                  value_us: 1420455600000000  # "2015-01-05T11:00:00+00:00"
                }
              }
            }
          }
        })proto",
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 1 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_boolean_label.class"
            value {}
          }
          feature {
            key: "label.test_boolean_label.timestamp_secs"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_boolean_label.value_boolean"
            value { int64_list { value: 0 value: 1 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "Encounter.class"
            value {
              feature {
                bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
              }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value { feature { int64_list {} } }
          }
          feature_list {
            key: "Encounter.period.start"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "encounterId"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "eventId"
            value { feature { int64_list { value: 1420444800 } } }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-1@1420444800:Encounter/1", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, TestDateTimeLabel) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1420444800000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  typename TypeParam::EventLabel label;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        type {
          system { value: "test_datetime_system" }
          code { value: "test_datetime_label" }
        }
        event_time { value_us: 1420444800000000 }
        label {
          class_value {
            date_time {
              value_us: 1515980100000000  # Monday, January 15, 2018 1:35:00 AM
              timezone: "UTC"
              precision: DAY
            }
          }
        }
      )proto",
      &label));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {label}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              class_value {
                system { value: "http://hl7.org/fhir/v3/ActCode" }
                code { value: "IMP" }
              }
              period {
                start {
                  value_us: 1420444800000000  # "2015-01-05T08:00:00+00:00"
                }
                end {
                  value_us: 1420455600000000  # "2015-01-05T11:00:00+00:00"
                }
              }
            }
          }
        })proto",
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 1 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_datetime_label.class"
            value {}
          }
          feature {
            key: "label.test_datetime_label.timestamp_secs"
            value { int64_list { value: 1420444800 } }
          }
          feature {
            key: "label.test_datetime_label.value_datetime_secs"
            value { int64_list { value: 1515980100 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "Encounter.class"
            value {
              feature {
                bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
              }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value { feature { int64_list {} } }
          }
          feature_list {
            key: "Encounter.period.start"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "encounterId"
            value { feature { int64_list { value: 1420444800 } } }
          }
          feature_list {
            key: "eventId"
            value { feature { int64_list { value: 1420444800 } } }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-1@1420444800:Encounter/1", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, RedactedFeatures) {
  absl::SetFlag(
      &FLAGS_trigger_time_redacted_features,
      absl::Substitute("Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis",
                       TypeParam::encounter_reason_code_field()));

  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time {
                                        value_us: 1420102800000000
                                        precision: SECOND
                                        timezone: "America/New_York"
                                      }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            entry {
              resource {
                patient {
                  id { value: "14" }
                  birth_date {
                    value_us: -1323388800000000
                    precision: DAY
                    timezone: "America/New_York"
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "1" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $0 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "V410.9" }
                      display { value: "Standard issue" }
                    }
                  }
                  period {
                    start {
                      value_us: 1417420800000000  # "2014-12-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1417424400000000  # "2014-12-01T09:00:00+00:00"
                    }
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "2" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $0 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "191.4" }
                      display { value: "Malignant neoplasm of occipital lobe" }
                    }
                  }
                  period {
                    start {
                      value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1420102800000000  # "2015-01-01T09:00:00+00:00"
                    }
                  }
                }
              }
            })proto",
          ToSnakeCase(TypeParam::encounter_reason_code_field())),
      &bundle));

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            context: {
              feature {
                key: "Patient.birthDate"
                value { int64_list { value: -1323388800 } }
              }
              feature {
                key: "currentEncounterId"
                value { int64_list { value: 1420099200 } }
              }
              feature {
                key: "patientId"
                value { bytes_list { value: "14" } }
              }
              feature {
                key: "sequenceLength"
                value { int64_list { value: 4 } }
              }
              feature {
                key: "timestamp"
                value { int64_list { value: 1420102800 } }
              }
            }
            feature_lists: {
              feature_list: {
                key: "Encounter.meta.lastUpdated"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.class"
                value {
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                }
              }
              feature_list {
                key: "Encounter.period.end"
                value {
                  feature { int64_list {} }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.period.start"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list { value: "V410.9" } }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.tokenized"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list { value: "standard" value: "issue" } }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "encounterId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "eventId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420102800 } }
                }
              }
            })proto",
          TypeParam::encounter_reason_code_field()),
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-4@1420102800:Encounter/1", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, JoinMedication) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(this->parser_.ParseFromString(R"proto(
                                              event_time {
                                                value_us: 1420102800000000
                                                precision: SECOND
                                                timezone: "America/New_York"
                                              }
                                            )proto",
                                            &trigger));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              period {
                start {
                  value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                }
                end {
                  value_us: 1420102800000000  # "2015-01-01T09:00:00+00:00"
                }
              }
            }
          }
        }
        entry {
          resource {
            medication {
              id { value: "med" }
              code {
                coding {
                  system { value: "http://hl7.org/fhir/sid/ndc" }
                  code { value: "123" }
                }
              }
            }
          }
        })proto",
      &bundle));

  typename TypeParam::MedicationRequest med_req = PARSE_INVALID_FHIR_PROTO(
      R"proto(
        medication { reference { medication_id { value: "med" } } }
        id { value: "1" }
        subject { patient_id { value: "14" } }
        authored_on { value_us: 1420102700000000 }
      )proto");
  const typename TypeParam::Medication medication = PARSE_VALID_FHIR_PROTO(
      R"proto(
        id { value: "med" }
        code {
          coding {
            system { value: "http://hl7.org/fhir/sid/ndc" }
            code { value: "123" }
          }
        }
      )proto");

  TypeParam::AddContainedResource(medication, &med_req);

  *bundle.add_entry()->mutable_resource()->mutable_medication_request() =
      med_req;

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420099200 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 3 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420102800 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list {} }
              feature { int64_list { value: 1420102800 } }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value {
              feature { int64_list {} }
              feature { int64_list {} }
              feature { int64_list { value: 1420102800 } }
            }
          }
          feature_list {
            key: "Encounter.period.start"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list {} }
              feature { int64_list { value: 1420099200 } }
            }
          }
          feature_list {
            key: "MedicationRequest.meta.lastUpdated"
            value {
              feature { int64_list {} }
              feature { int64_list { value: 1420102700 } }
              feature { int64_list {} }
            }
          }
          feature_list {
            key: "MedicationRequest.contained.medication.code.http-hl7-org-fhir-sid-ndc"
            value {
              feature { bytes_list {} }
              feature { bytes_list { value: "123" } }
              feature { bytes_list {} }
            }
          }
          feature_list {
            key: "MedicationRequest.authoredOn"
            value {
              feature { int64_list {} }
              feature { int64_list { value: 1420102700 } }
              feature { int64_list {} }
            }
          }
          feature_list {
            key: "eventId"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list { value: 1420102700 } }
              feature { int64_list { value: 1420102800 } }
            }
          }
          feature_list {
            key: "encounterId"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list { value: 1420099200 } }
              feature { int64_list { value: 1420099200 } }
            }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-3@1420102800", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, EmptyLabel) {
  typename TypeParam::EventTrigger trigger;
  ASSERT_TRUE(this->parser_.ParseFromString(R"proto(
                                              event_time {
                                                value_us: 1420102800000000
                                                precision: SECOND
                                                timezone: "America/New_York"
                                              }
                                            )proto",
                                            &trigger));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger, {}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        entry {
          resource {
            patient {
              id { value: "14" }
              birth_date {
                value_us: -1323388800000000
                precision: DAY
                timezone: "America/New_York"
              }
            }
          }
        }
        entry {
          resource {
            encounter {
              id { value: "1" }
              subject { patient_id { value: "14" } }
              period {
                start {
                  value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                }
                end {
                  value_us: 1420102800000000  # "2015-01-01T09:00:00+00:00"
                }
              }
            }
          }
        }
        entry {
          resource {
            medication {
              id { value: "med" }
              code {
                coding {
                  system { value: "http://hl7.org/fhir/sid/ndc" }
                  code { value: "123" }
                }
              }
            }
          }
        })proto",
      &bundle));

  typename TypeParam::MedicationRequest med_req = PARSE_INVALID_FHIR_PROTO(
      R"proto(
        medication { reference { medication_id { value: "med" } } }
        id { value: "1" }
        subject { patient_id { value: "14" } }
        authored_on { value_us: 1420100000000000 }
      )proto");
  const typename TypeParam::Medication medication = PARSE_VALID_FHIR_PROTO(
      R"proto(
        id { value: "med" }
        code {
          coding {
            system { value: "http://hl7.org/fhir/sid/ndc" }
            code { value: "123" }
          }
        }
      )proto");
  TypeParam::AddContainedResource(medication, &med_req);
  *bundle.add_entry()->mutable_resource()->mutable_medication_request() =
      med_req;

  SequenceExample seqex;
  ASSERT_TRUE(this->parser_.ParseFromString(
      R"proto(
        context: {
          feature: {
            key: "Patient.birthDate"
            value { int64_list { value: -1323388800 } }
          }
          feature {
            key: "currentEncounterId"
            value { int64_list { value: 1420099200 } }
          }
          feature {
            key: "patientId"
            value { bytes_list { value: "14" } }
          }
          feature {
            key: "sequenceLength"
            value { int64_list { value: 3 } }
          }
          feature {
            key: "timestamp"
            value { int64_list { value: 1420102800 } }
          }
        }
        feature_lists: {
          feature_list {
            key: "Encounter.meta.lastUpdated"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list {} }
              feature { int64_list { value: 1420102800 } }
            }
          }
          feature_list {
            key: "Encounter.period.end"
            value {
              feature { int64_list {} }
              feature { int64_list {} }
              feature { int64_list { value: 1420102800 } }
            }
          }
          feature_list {
            key: "Encounter.period.start"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list {} }
              feature { int64_list { value: 1420099200 } }
            }
          }
          feature_list {
            key: "MedicationRequest.meta.lastUpdated"
            value {
              feature { int64_list {} }
              feature { int64_list { value: 1420100000 } }
              feature { int64_list {} }
            }
          }
          feature_list {
            key: "MedicationRequest.contained.medication.code.http-hl7-org-fhir-sid-ndc"
            value {
              feature { bytes_list {} }
              feature { bytes_list { value: "123" } }
              feature { bytes_list {} }
            }
          }
          feature_list {
            key: "MedicationRequest.authoredOn"
            value {
              feature { int64_list {} }
              feature { int64_list { value: 1420100000 } }
              feature { int64_list {} }
            }
          }
          feature_list {
            key: "eventId"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list { value: 1420100000 } }
              feature { int64_list { value: 1420102800 } }
            }
          }
          feature_list {
            key: "encounterId"
            value {
              feature { int64_list { value: 1420099200 } }
              feature { int64_list { value: 1420099200 } }
              feature { int64_list { value: 1420099200 } }
            }
          }
        })proto",
      &seqex));

  this->PerformTest("Patient/14", bundle, trigger_labels_pair,
                    {{"Patient/14:0-3@1420102800", seqex}});
}

TYPED_TEST(BundleToSeqexConverterTest, TwoExamples) {
  absl::SetFlag(
      &FLAGS_trigger_time_redacted_features,
      absl::Substitute("Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis",
                       TypeParam::encounter_reason_code_field()));

  typename TypeParam::EventTrigger trigger1;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1417424400000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger1));
  typename TypeParam::EventTrigger trigger2;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time {
                                        value_us: 1420102800000000
                                        precision: SECOND
                                        timezone: "America/New_York"
                                      }
                                      source { encounter_id { value: "2" } }
                                    )proto",
                                    &trigger2));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger1, {}}, {trigger2, {}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            entry {
              resource {
                patient {
                  id { value: "14" }
                  birth_date {
                    value_us: -1323388800000000
                    precision: DAY
                    timezone: "America/New_York"
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "1" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $0 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "V410.9" }
                      display { value: "Standard issue" }
                    }
                  }
                  period {
                    start {
                      value_us: 1417420800000000  # "2014-12-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1417424400000000  # "2014-12-01T09:00:00+00:00"
                    }
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "2" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $0 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "191.4" }
                      display { value: "Malignant neoplasm of occipital lobe" }
                    }
                  }
                  period {
                    start {
                      value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1420102800000000  # "2015-01-01T09:00:00+00:00"
                    }
                  }
                }
              }
            })proto",
          ToSnakeCase(TypeParam::encounter_reason_code_field())),
      &bundle));

  SequenceExample seqex1;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            context: {
              feature {
                key: "Patient.birthDate"
                value { int64_list { value: -1323388800 } }
              }
              feature {
                key: "currentEncounterId"
                value { int64_list { value: 1417420800 } }
              }
              feature {
                key: "patientId"
                value { bytes_list { value: "14" } }
              }
              feature {
                key: "sequenceLength"
                value { int64_list { value: 2 } }
              }
              feature {
                key: "timestamp"
                value { int64_list { value: 1417424400 } }
              }
            }
            feature_lists: {
              feature_list {
                key: "Encounter.meta.lastUpdated"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                }
              }
              feature_list {
                key: "Encounter.class"
                value {
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                }
              }
              feature_list {
                key: "Encounter.period.end"
                value {
                  feature { int64_list {} }
                  feature { int64_list { value: 1417424400 } }
                }
              }
              feature_list {
                key: "Encounter.period.start"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.tokenized"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "encounterId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                }
              }
              feature_list {
                key: "eventId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                }
              }
            })proto",
          TypeParam::encounter_reason_code_field()),
      &seqex1));

  SequenceExample seqex2;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            context: {
              feature {
                key: "Patient.birthDate"
                value { int64_list { value: -1323388800 } }
              }
              feature {
                key: "currentEncounterId"
                value { int64_list { value: 1420099200 } }
              }
              feature {
                key: "patientId"
                value { bytes_list { value: "14" } }
              }
              feature {
                key: "sequenceLength"
                value { int64_list { value: 4 } }
              }
              feature {
                key: "timestamp"
                value { int64_list { value: 1420102800 } }
              }
            }
            feature_lists: {
              feature_list {
                key: "Encounter.meta.lastUpdated"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.class"
                value {
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                }
              }
              feature_list {
                key: "Encounter.period.end"
                value {
                  feature { int64_list {} }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.period.start"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list { value: "V410.9" } }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.tokenized"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list { value: "standard" value: "issue" } }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "encounterId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "eventId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420102800 } }
                }
              }
            })proto",
          TypeParam::encounter_reason_code_field()),
      &seqex2));

  typename TypeParam::BundleToSeqexConverter converter(
      this->fhir_version_config_, this->tokenizer_,
      false /* enable_attribution */, false /*generate_sequence_label */);
  std::map<std::string, int> counter_stats;
  ASSERT_TRUE(converter.Begin("Patient/14", bundle, trigger_labels_pair,
                              &counter_stats));
  ASSERT_FALSE(converter.Done());
  EXPECT_EQ("a8c128978feaab69-Patient/14:0-2@1417424400:Encounter/1",
            converter.ExampleKeyWithPrefix());
  EXPECT_THAT(seqex1, EqualsProto(converter.GetExample()));
  ASSERT_TRUE(converter.Next());
  ASSERT_FALSE(converter.Done());
  EXPECT_EQ("a87dd8b5f6221497-Patient/14:0-4@1420102800:Encounter/2",
            converter.ExampleKeyWithPrefix());
  EXPECT_THAT(seqex2, EqualsProto(converter.GetExample()));
  ASSERT_TRUE(converter.Next());
  ASSERT_TRUE(converter.Done());
}

TYPED_TEST(BundleToSeqexConverterTest, TwoExamples_EnableAttribution) {
  absl::SetFlag(
      &FLAGS_trigger_time_redacted_features,
      absl::Substitute("Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis",
                       TypeParam::encounter_reason_code_field()));

  typename TypeParam::EventTrigger trigger1;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time { value_us: 1417424400000000 }
                                      source { encounter_id { value: "1" } }
                                    )proto",
                                    &trigger1));
  typename TypeParam::EventTrigger trigger2;
  ASSERT_TRUE(
      this->parser_.ParseFromString(R"proto(
                                      event_time {
                                        value_us: 1420102800000000
                                        precision: SECOND
                                        timezone: "America/New_York"
                                      }
                                      source { encounter_id { value: "2" } }
                                    )proto",
                                    &trigger2));
  std::vector<typename TypeParam::TriggerLabelsPair> trigger_labels_pair(
      {{trigger1, {}}, {trigger2, {}}});
  typename TypeParam::Bundle bundle;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            entry {
              resource {
                patient {
                  id { value: "14" }
                  birth_date {
                    value_us: -1323388800000000
                    precision: DAY
                    timezone: "America/New_York"
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "1" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $0 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "V410.9" }
                      display { value: "Standard issue" }
                    }
                  }
                  period {
                    start {
                      value_us: 1417420800000000  # "2014-12-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1417424400000000  # "2014-12-01T09:00:00+00:00"
                    }
                  }
                }
              }
            }
            entry {
              resource {
                encounter {
                  id { value: "2" }
                  subject { patient_id { value: "14" } }
                  class_value {
                    system { value: "http://hl7.org/fhir/v3/ActCode" }
                    code { value: "IMP" }
                  }
                  $0 {
                    coding {
                      system {
                        value: "http://hl7.org/fhir/sid/icd-9-cm/diagnosis"
                      }
                      code { value: "191.4" }
                      display { value: "Malignant neoplasm of occipital lobe" }
                    }
                  }
                  period {
                    start {
                      value_us: 1420099200000000  # "2015-01-01T08:00:00+00:00"
                    }
                    end {
                      value_us: 1420102800000000  # "2015-01-01T09:00:00+00:00"
                    }
                  }
                }
              }
            })proto",
          ToSnakeCase(TypeParam::encounter_reason_code_field())),
      &bundle));

  SequenceExample seqex1;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            context: {
              feature {
                key: "Patient.birthDate"
                value { int64_list { value: -1323388800 } }
              }
              feature {
                key: "currentEncounterId"
                value { int64_list { value: 1417420800 } }
              }
              feature {
                key: "patientId"
                value { bytes_list { value: "14" } }
              }
              feature {
                key: "sequenceLength"
                value { int64_list { value: 2 } }
              }
              feature {
                key: "timestamp"
                value { int64_list { value: 1417424400 } }
              }
            }
            feature_lists: {
              feature_list {
                key: "Encounter.meta.lastUpdated"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                }
              }
              feature_list {
                key: "Encounter.class"
                value {
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                }
              }
              feature_list {
                key: "Encounter.period.end"
                value {
                  feature { int64_list {} }
                  feature { int64_list { value: 1417424400 } }
                }
              }
              feature_list {
                key: "Encounter.period.start"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.tokenized"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.token_start"
                value {
                  feature { int64_list {} }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.token_end"
                value {
                  feature { int64_list {} }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "encounterId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                }
              }
              feature_list {
                key: "eventId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                }
              }
              feature_list {
                key: "resourceId"
                value {
                  feature { bytes_list { value: "Encounter/1" } }
                  feature { bytes_list { value: "Encounter/1" } }
                }
              }
            })proto",
          TypeParam::encounter_reason_code_field()),
      &seqex1));

  SequenceExample seqex2;
  ASSERT_TRUE(this->parser_.ParseFromString(
      absl::Substitute(
          R"proto(
            context: {
              feature {
                key: "Patient.birthDate"
                value { int64_list { value: -1323388800 } }
              }
              feature {
                key: "currentEncounterId"
                value { int64_list { value: 1420099200 } }
              }
              feature {
                key: "patientId"
                value { bytes_list { value: "14" } }
              }
              feature {
                key: "sequenceLength"
                value { int64_list { value: 4 } }
              }
              feature {
                key: "timestamp"
                value { int64_list { value: 1420102800 } }
              }
            }
            feature_lists: {
              feature_list {
                key: "Encounter.meta.lastUpdated"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.class"
                value {
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                  feature {
                    bytes_list { value: "http-hl7-org-fhir-v3-ActCode:IMP" }
                  }
                }
              }
              feature_list {
                key: "Encounter.period.end"
                value {
                  feature { int64_list {} }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list {} }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "Encounter.period.start"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list { value: "V410.9" } }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.tokenized"
                value {
                  feature { bytes_list {} }
                  feature { bytes_list { value: "standard" value: "issue" } }
                  feature { bytes_list {} }
                  feature { bytes_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.token_start"
                value {
                  feature { int64_list {} }
                  feature { int64_list { value: 0 value: 9 } }
                  feature { int64_list {} }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "Encounter.$0.http-hl7-org-fhir-sid-icd-9-cm-diagnosis.display.token_end"
                value {
                  feature { int64_list {} }
                  feature { int64_list { value: 8 value: 14 } }
                  feature { int64_list {} }
                  feature { int64_list {} }
                }
              }
              feature_list {
                key: "encounterId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420099200 } }
                }
              }
              feature_list {
                key: "eventId"
                value {
                  feature { int64_list { value: 1417420800 } }
                  feature { int64_list { value: 1417424400 } }
                  feature { int64_list { value: 1420099200 } }
                  feature { int64_list { value: 1420102800 } }
                }
              }
              feature_list {
                key: "resourceId"
                value {
                  feature { bytes_list { value: "Encounter/1" } }
                  feature { bytes_list { value: "Encounter/1" } }
                  feature { bytes_list { value: "Encounter/2" } }
                  feature { bytes_list { value: "Encounter/2" } }
                }
              }
            })proto",
          TypeParam::encounter_reason_code_field()),
      &seqex2));

  typename TypeParam::BundleToSeqexConverter converter(
      this->fhir_version_config_, this->tokenizer_,
      true /* enable_attribution */, false /* generate_sequence_label */);
  std::map<std::string, int> counter_stats;
  ASSERT_TRUE(converter.Begin("Patient/14", bundle, trigger_labels_pair,
                              &counter_stats));
  ASSERT_FALSE(converter.Done());
  EXPECT_EQ("a8c128978feaab69-Patient/14:0-2@1417424400:Encounter/1",
            converter.ExampleKeyWithPrefix());
  EXPECT_THAT(seqex1, EqualsProto(converter.GetExample()));
  ASSERT_TRUE(converter.Next());
  ASSERT_FALSE(converter.Done());
  EXPECT_EQ("a87dd8b5f6221497-Patient/14:0-4@1420102800:Encounter/2",
            converter.ExampleKeyWithPrefix());
  EXPECT_THAT(seqex2, EqualsProto(converter.GetExample()));
  ASSERT_TRUE(converter.Next());
  ASSERT_TRUE(converter.Done());
}

}  // namespace

}  // namespace seqex
}  // namespace fhir
}  // namespace google
