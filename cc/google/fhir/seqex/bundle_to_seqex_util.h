/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GOOGLE_FHIR_SEQEX_BUNDLE_TO_SEQEX_UTIL_H_
#define GOOGLE_FHIR_SEQEX_BUNDLE_TO_SEQEX_UTIL_H_

#include <set>
#include <string>
#include <vector>

#include "google/protobuf/reflection.h"
#include "google/fhir/extensions.h"
#include "google/fhir/status/statusor.h"
#include "google/fhir/util.h"

namespace google {
namespace fhir {
namespace seqex {

namespace internal {

template <typename ConverterTypes>
StatusOr<std::vector<typename ConverterTypes::EventLabel>>
ExtractLabelsFromExtensions(
    const std::set<std::string>& label_names,
    google::protobuf::RepeatedFieldRef<
        typename ConverterTypes::PrimitiveHandler::Extension>
        extensions) {
  std::vector<typename ConverterTypes::EventLabel> labels;
  FHIR_RETURN_IF_ERROR(google::fhir::extensions_lib::GetRepeatedFromExtension(
      extensions, &labels));
  std::vector<typename ConverterTypes::EventLabel> target_labels;
  for (const auto& label : labels) {
    if (label_names.count(label.type().code().value()) > 0) {
      target_labels.push_back(label);
    }
  }
  return target_labels;
}

template <typename ConverterTypes>
void GetTriggerLabelsPairFromExtensions(
    const ::google::protobuf::RepeatedFieldRef<
        typename ConverterTypes::PrimitiveHandler::Extension>
        extensions,
    const std::set<std::string>& label_names,
    const std::string& trigger_event_name,
    std::vector<typename ConverterTypes::TriggerLabelsPair>*
        trigger_labels_pair,
    int* num_triggers_filtered) {
  std::vector<typename ConverterTypes::EventTrigger> triggers;
  FHIR_CHECK_OK(google::fhir::extensions_lib::GetRepeatedFromExtension(
      extensions, &triggers));
  // Note that this only joins triggers and labels within the same resource.
  auto labels_result =
      ExtractLabelsFromExtensions<ConverterTypes>(label_names, extensions);
  std::vector<typename ConverterTypes::EventLabel> labels =
      labels_result.ValueOrDie();
  for (const auto& trigger : triggers) {
    if (trigger.type().code().value() != trigger_event_name) {
      continue;
    }
    absl::Time trigger_time =
        google::fhir::GetTimeFromTimelikeElement(trigger.event_time());
    bool should_keep = true;
    for (const typename ConverterTypes::EventLabel& label : labels) {
      if (label.has_event_time() && google::fhir::GetTimeFromTimelikeElement(
                                        label.event_time()) < trigger_time) {
        // If the label happens before the trigger, the trigger should be
        // thrown out. Note that there is no easy way to only throw out one
        // type of label because a missing label would be treated as a
        // negative example, not an example which should be skipped.
        should_keep = false;
        break;
      }
    }
    if (should_keep) {
      trigger_labels_pair->push_back(std::make_pair(trigger, labels));
    } else {
      ++(*num_triggers_filtered);
    }
  }
}

}  // namespace internal


// Group label events by event time, create a trigger proto for each group, and
// format as a TriggerLabelsPair. The output is guaranteed to be sorted.
template <typename ConverterTypes>
void GetTriggerLabelsPairFromInputLabels(
    const std::vector<typename ConverterTypes::EventLabel>& input_labels,
    std::vector<typename ConverterTypes::TriggerLabelsPair>*
        trigger_labels_pair) {
  if (input_labels.empty()) {
    return;
  }
  std::map<absl::Time, uint> trigger_index_for_time;
  for (const auto& label : input_labels) {
    typename ConverterTypes::EventLabel trigger_label_template;
    CHECK(label.has_patient() && label.patient().has_patient_id() &&
          !label.patient().patient_id().value().empty())
        << label.DebugString();
    if (!trigger_label_template.has_patient()) {
      *trigger_label_template.mutable_patient() = label.patient();
    } else {
      CHECK_EQ(trigger_label_template.patient().patient_id().value(),
               label.patient().patient_id().value())
          << label.DebugString();
    }
    CHECK(label.has_type()) << label.DebugString();
    absl::Time time = GetTimeFromTimelikeElement(label.event_time());
    if (trigger_index_for_time.find(time) == trigger_index_for_time.end()) {
      trigger_labels_pair->emplace_back();
      *trigger_labels_pair->back().first.mutable_event_time() =
          label.event_time();
      if (label.has_source()) {
        *trigger_labels_pair->back().first.mutable_source() = label.source();
      }
      trigger_index_for_time[time] = trigger_labels_pair->size() - 1;
    }
    typename ConverterTypes::EventLabel trigger_labels(trigger_label_template);
    (*trigger_labels_pair)[trigger_index_for_time[time]].second.push_back(
        label);
  }
}

// Extract triggers and labels from the provided bundle, and format as
// TriggerLabelsPair. The output is guaranteed to be sorted.
template <typename ConverterTypes, typename BundleLike>
void GetTriggerLabelsPair(
    const BundleLike& bundle, const std::set<std::string>& label_names,
    const std::string& trigger_event_name,
    std::vector<typename ConverterTypes::TriggerLabelsPair>*
        trigger_labels_pair,
    int* num_triggers_filtered) {
  for (const auto& entry : bundle.entry()) {
    auto result = GetResourceExtensionsFromBundleEntry(entry);
    if (!result.ok()) {
      continue;
    }

    internal::GetTriggerLabelsPairFromExtensions<ConverterTypes>(
        result.ValueOrDie(), label_names, trigger_event_name,
        trigger_labels_pair, num_triggers_filtered);
  }
}

template <typename ConverterTypes, typename BundleLike>
std::vector<typename ConverterTypes::EventLabel> ExtractLabelsFromBundle(
    const BundleLike& bundle, const std::set<std::string>& label_names) {
  std::vector<typename ConverterTypes::EventLabel> labels;
  for (const auto& entry : bundle.entry()) {
    auto result = GetResourceExtensionsFromBundleEntry(entry);
    if (!result.ok()) {
      continue;
    }
    auto extensions = result.ValueOrDie();
    auto labels_result = internal::ExtractLabelsFromExtensions<ConverterTypes>(
        label_names, extensions);
    if (!labels_result.ok()) {
      continue;
    }
    std::vector<typename ConverterTypes::EventLabel>& new_labels =
        labels_result.ValueOrDie();
    labels.insert(labels.end(), new_labels.begin(), new_labels.end());
  }
  return labels;
}

}  // namespace seqex
}  // namespace fhir
}  // namespace google

#endif  // GOOGLE_FHIR_SEQEX_BUNDLE_TO_SEQEX_UTIL_H_
