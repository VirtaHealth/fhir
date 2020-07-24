# Introduction

This repository is forked from https://github.com/google/fhir. Includes fixes for https://github.com/google/fhir/issues/17 and miscellaneous other small changes. The [upstream branch](https://github.com/VirtaHealth/fhir/tree/upstream) tracks the original repo. To update the fork from upstream:

    $ git pull --rebase https://github.com/google/fhir.git master:upstream

FhirProto is Google’s implementation of the [FHIR Standard](http://hl7.org/fhir/) for Health Care data using [Protocol Buffers](https://developers.google.com/protocol-buffers).  By leveraging Google’s core data storage format, FhirProto provides a type-safe, strongly validated FHIR format with cross-language support at a fraction of the size on disk, making it a great data model to develop your Health Care application on.  Structured Codes and Extensions guarantee that your data will be in the correct format.  Support for generating and validating against custom Implementation Guides allow you to customize FhirProto to your dataset and requirements.  Parsing and Printing libraries make it easy to go back and forth between FhirProto format and JSON.

# Getting Started
We think the best way to get an idea of how FhirProto works is to get in and start playing with it.  To that end, we provide [https://github.com/google/fhir-examples](https://github.com/google/fhir-examples). This repo contains a script for using [Synthea](https://github.com/synthetichealth/synthea) to create a synthetic FHIR JSON dataset, and then shows some examples of parsing, printing, validating, profiling and querying.  The repo also contains a [walkthrough](https://github.com/google/fhir-examples/blob/master/EXAMPLES.md) of many of the examples.

A Reference User Guide with in-depth descriptions of different concepts can be found [here](https://github.com/google/fhir-examples/blob/master/USERGUIDE.md).

## Trademark
FHIR® is the registered trademark of HL7 and is used with the permission of HL7. Use of the FHIR trademark does not constitute endorsement of this product by HL7.

