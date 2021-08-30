//    Copyright 2020 Google Inc.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        https://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
// Dummy proto to facilitate testing of the protopath library.

// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.27.1
// 	protoc        v3.14.0
// source: go/jsonformat/internal/protopath/protopathtest.proto

package protopathtest_go_proto

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type MessageType int32

const (
	MessageType_INVALID_UNINITIALIZED MessageType = 0
	MessageType_TYPE_1                MessageType = 1
	MessageType_TYPE_2                MessageType = 2
)

// Enum value maps for MessageType.
var (
	MessageType_name = map[int32]string{
		0: "INVALID_UNINITIALIZED",
		1: "TYPE_1",
		2: "TYPE_2",
	}
	MessageType_value = map[string]int32{
		"INVALID_UNINITIALIZED": 0,
		"TYPE_1":                1,
		"TYPE_2":                2,
	}
)

func (x MessageType) Enum() *MessageType {
	p := new(MessageType)
	*p = x
	return p
}

func (x MessageType) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (MessageType) Descriptor() protoreflect.EnumDescriptor {
	return file_go_jsonformat_internal_protopath_protopathtest_proto_enumTypes[0].Descriptor()
}

func (MessageType) Type() protoreflect.EnumType {
	return &file_go_jsonformat_internal_protopath_protopathtest_proto_enumTypes[0]
}

func (x MessageType) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use MessageType.Descriptor instead.
func (MessageType) EnumDescriptor() ([]byte, []int) {
	return file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescGZIP(), []int{0}
}

// Message type that will never be present in Message.
type Missing struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields
}

func (x *Missing) Reset() {
	*x = Missing{}
	if protoimpl.UnsafeEnabled {
		mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Missing) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Missing) ProtoMessage() {}

func (x *Missing) ProtoReflect() protoreflect.Message {
	mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Missing.ProtoReflect.Descriptor instead.
func (*Missing) Descriptor() ([]byte, []int) {
	return file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescGZIP(), []int{0}
}

type Message struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	Int32 int32 `protobuf:"varint,1,opt,name=int32,proto3" json:"int32,omitempty"`
	// Types that are assignable to Oneof:
	//	*Message_OneofPrimitiveField
	//	*Message_OneofConflictingPrimitiveField_1
	//	*Message_OneofConflictingPrimitiveField_2
	//	*Message_OneofMessageField
	//	*Message_OneofConflictingMessageField_1
	//	*Message_OneofConflictingMessageField_2
	Oneof                    isMessage_Oneof         `protobuf_oneof:"oneof"`
	Type                     MessageType             `protobuf:"varint,8,opt,name=type,proto3,enum=fhir.go.jsonformat.internal.protopath.MessageType" json:"type,omitempty"`
	MessageField             *Message_InnerMessage   `protobuf:"bytes,9,opt,name=message_field,json=messageField,proto3" json:"message_field,omitempty"`
	RepeatedMessageField     []*Message_InnerMessage `protobuf:"bytes,10,rep,name=repeated_message_field,json=repeatedMessageField,proto3" json:"repeated_message_field,omitempty"`
	StringField              string                  `protobuf:"bytes,11,opt,name=string_field,json=stringField,proto3" json:"string_field,omitempty"`
	JsonAnnotatedStringField string                  `protobuf:"bytes,12,opt,name=json_annotated_string_field,json=jsonAnnotated,proto3" json:"json_annotated_string_field,omitempty"`
}

func (x *Message) Reset() {
	*x = Message{}
	if protoimpl.UnsafeEnabled {
		mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Message) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Message) ProtoMessage() {}

func (x *Message) ProtoReflect() protoreflect.Message {
	mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Message.ProtoReflect.Descriptor instead.
func (*Message) Descriptor() ([]byte, []int) {
	return file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescGZIP(), []int{1}
}

func (x *Message) GetInt32() int32 {
	if x != nil {
		return x.Int32
	}
	return 0
}

func (m *Message) GetOneof() isMessage_Oneof {
	if m != nil {
		return m.Oneof
	}
	return nil
}

func (x *Message) GetOneofPrimitiveField() bool {
	if x, ok := x.GetOneof().(*Message_OneofPrimitiveField); ok {
		return x.OneofPrimitiveField
	}
	return false
}

func (x *Message) GetOneofConflictingPrimitiveField_1() int32 {
	if x, ok := x.GetOneof().(*Message_OneofConflictingPrimitiveField_1); ok {
		return x.OneofConflictingPrimitiveField_1
	}
	return 0
}

func (x *Message) GetOneofConflictingPrimitiveField_2() int32 {
	if x, ok := x.GetOneof().(*Message_OneofConflictingPrimitiveField_2); ok {
		return x.OneofConflictingPrimitiveField_2
	}
	return 0
}

func (x *Message) GetOneofMessageField() *Message_InnerMessage {
	if x, ok := x.GetOneof().(*Message_OneofMessageField); ok {
		return x.OneofMessageField
	}
	return nil
}

func (x *Message) GetOneofConflictingMessageField_1() *Message_InnerMessage2 {
	if x, ok := x.GetOneof().(*Message_OneofConflictingMessageField_1); ok {
		return x.OneofConflictingMessageField_1
	}
	return nil
}

func (x *Message) GetOneofConflictingMessageField_2() *Message_InnerMessage2 {
	if x, ok := x.GetOneof().(*Message_OneofConflictingMessageField_2); ok {
		return x.OneofConflictingMessageField_2
	}
	return nil
}

func (x *Message) GetType() MessageType {
	if x != nil {
		return x.Type
	}
	return MessageType_INVALID_UNINITIALIZED
}

func (x *Message) GetMessageField() *Message_InnerMessage {
	if x != nil {
		return x.MessageField
	}
	return nil
}

func (x *Message) GetRepeatedMessageField() []*Message_InnerMessage {
	if x != nil {
		return x.RepeatedMessageField
	}
	return nil
}

func (x *Message) GetStringField() string {
	if x != nil {
		return x.StringField
	}
	return ""
}

func (x *Message) GetJsonAnnotatedStringField() string {
	if x != nil {
		return x.JsonAnnotatedStringField
	}
	return ""
}

type isMessage_Oneof interface {
	isMessage_Oneof()
}

type Message_OneofPrimitiveField struct {
	OneofPrimitiveField bool `protobuf:"varint,2,opt,name=oneof_primitive_field,json=oneofPrimitiveField,proto3,oneof"`
}

type Message_OneofConflictingPrimitiveField_1 struct {
	// Duplicate primitive fields to test type conflict.
	OneofConflictingPrimitiveField_1 int32 `protobuf:"varint,3,opt,name=oneof_conflicting_primitive_field_1,json=oneofConflictingPrimitiveField1,proto3,oneof"`
}

type Message_OneofConflictingPrimitiveField_2 struct {
	OneofConflictingPrimitiveField_2 int32 `protobuf:"varint,4,opt,name=oneof_conflicting_primitive_field_2,json=oneofConflictingPrimitiveField2,proto3,oneof"`
}

type Message_OneofMessageField struct {
	OneofMessageField *Message_InnerMessage `protobuf:"bytes,5,opt,name=oneof_message_field,json=oneofMessageField,proto3,oneof"`
}

type Message_OneofConflictingMessageField_1 struct {
	// Duplicate message fields to test type conflict.
	OneofConflictingMessageField_1 *Message_InnerMessage2 `protobuf:"bytes,6,opt,name=oneof_conflicting_message_field_1,json=oneofConflictingMessageField1,proto3,oneof"`
}

type Message_OneofConflictingMessageField_2 struct {
	OneofConflictingMessageField_2 *Message_InnerMessage2 `protobuf:"bytes,7,opt,name=oneof_conflicting_message_field_2,json=oneofConflictingMessageField2,proto3,oneof"`
}

func (*Message_OneofPrimitiveField) isMessage_Oneof() {}

func (*Message_OneofConflictingPrimitiveField_1) isMessage_Oneof() {}

func (*Message_OneofConflictingPrimitiveField_2) isMessage_Oneof() {}

func (*Message_OneofMessageField) isMessage_Oneof() {}

func (*Message_OneofConflictingMessageField_1) isMessage_Oneof() {}

func (*Message_OneofConflictingMessageField_2) isMessage_Oneof() {}

type Message_InnerMessage struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	InnerField                int32                    `protobuf:"varint,1,opt,name=inner_field,json=innerField,proto3" json:"inner_field,omitempty"`
	RepeatedInnerField        []int32                  `protobuf:"varint,2,rep,packed,name=repeated_inner_field,json=repeatedInnerField,proto3" json:"repeated_inner_field,omitempty"`
	RepeatedInnerMessageField []*Message_InnerMessage2 `protobuf:"bytes,3,rep,name=repeated_inner_message_field,json=repeatedInnerMessageField,proto3" json:"repeated_inner_message_field,omitempty"`
	// Types that are assignable to InnerOneof:
	//	*Message_InnerMessage_InnerOneofMessageField
	InnerOneof isMessage_InnerMessage_InnerOneof `protobuf_oneof:"inner_oneof"`
}

func (x *Message_InnerMessage) Reset() {
	*x = Message_InnerMessage{}
	if protoimpl.UnsafeEnabled {
		mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Message_InnerMessage) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Message_InnerMessage) ProtoMessage() {}

func (x *Message_InnerMessage) ProtoReflect() protoreflect.Message {
	mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Message_InnerMessage.ProtoReflect.Descriptor instead.
func (*Message_InnerMessage) Descriptor() ([]byte, []int) {
	return file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescGZIP(), []int{1, 0}
}

func (x *Message_InnerMessage) GetInnerField() int32 {
	if x != nil {
		return x.InnerField
	}
	return 0
}

func (x *Message_InnerMessage) GetRepeatedInnerField() []int32 {
	if x != nil {
		return x.RepeatedInnerField
	}
	return nil
}

func (x *Message_InnerMessage) GetRepeatedInnerMessageField() []*Message_InnerMessage2 {
	if x != nil {
		return x.RepeatedInnerMessageField
	}
	return nil
}

func (m *Message_InnerMessage) GetInnerOneof() isMessage_InnerMessage_InnerOneof {
	if m != nil {
		return m.InnerOneof
	}
	return nil
}

func (x *Message_InnerMessage) GetInnerOneofMessageField() *Message_InnerMessage2 {
	if x, ok := x.GetInnerOneof().(*Message_InnerMessage_InnerOneofMessageField); ok {
		return x.InnerOneofMessageField
	}
	return nil
}

type isMessage_InnerMessage_InnerOneof interface {
	isMessage_InnerMessage_InnerOneof()
}

type Message_InnerMessage_InnerOneofMessageField struct {
	InnerOneofMessageField *Message_InnerMessage2 `protobuf:"bytes,4,opt,name=inner_oneof_message_field,json=innerOneofMessageField,proto3,oneof"`
}

func (*Message_InnerMessage_InnerOneofMessageField) isMessage_InnerMessage_InnerOneof() {}

type Message_InnerMessage2 struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	InnerField2 int32 `protobuf:"varint,1,opt,name=inner_field2,json=innerField2,proto3" json:"inner_field2,omitempty"`
}

func (x *Message_InnerMessage2) Reset() {
	*x = Message_InnerMessage2{}
	if protoimpl.UnsafeEnabled {
		mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *Message_InnerMessage2) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Message_InnerMessage2) ProtoMessage() {}

func (x *Message_InnerMessage2) ProtoReflect() protoreflect.Message {
	mi := &file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Message_InnerMessage2.ProtoReflect.Descriptor instead.
func (*Message_InnerMessage2) Descriptor() ([]byte, []int) {
	return file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescGZIP(), []int{1, 1}
}

func (x *Message_InnerMessage2) GetInnerField2() int32 {
	if x != nil {
		return x.InnerField2
	}
	return 0
}

var File_go_jsonformat_internal_protopath_protopathtest_proto protoreflect.FileDescriptor

var file_go_jsonformat_internal_protopath_protopathtest_proto_rawDesc = []byte{
	0x0a, 0x34, 0x67, 0x6f, 0x2f, 0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x2f,
	0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x61, 0x6c, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61,
	0x74, 0x68, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x74, 0x65, 0x73, 0x74,
	0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x25, 0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e,
	0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72,
	0x6e, 0x61, 0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x22, 0x09, 0x0a,
	0x07, 0x4d, 0x69, 0x73, 0x73, 0x69, 0x6e, 0x67, 0x22, 0x98, 0x0b, 0x0a, 0x07, 0x4d, 0x65, 0x73,
	0x73, 0x61, 0x67, 0x65, 0x12, 0x14, 0x0a, 0x05, 0x69, 0x6e, 0x74, 0x33, 0x32, 0x18, 0x01, 0x20,
	0x01, 0x28, 0x05, 0x52, 0x05, 0x69, 0x6e, 0x74, 0x33, 0x32, 0x12, 0x34, 0x0a, 0x15, 0x6f, 0x6e,
	0x65, 0x6f, 0x66, 0x5f, 0x70, 0x72, 0x69, 0x6d, 0x69, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x66, 0x69,
	0x65, 0x6c, 0x64, 0x18, 0x02, 0x20, 0x01, 0x28, 0x08, 0x48, 0x00, 0x52, 0x13, 0x6f, 0x6e, 0x65,
	0x6f, 0x66, 0x50, 0x72, 0x69, 0x6d, 0x69, 0x74, 0x69, 0x76, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64,
	0x12, 0x4e, 0x0a, 0x23, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x5f, 0x63, 0x6f, 0x6e, 0x66, 0x6c, 0x69,
	0x63, 0x74, 0x69, 0x6e, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6d, 0x69, 0x74, 0x69, 0x76, 0x65, 0x5f,
	0x66, 0x69, 0x65, 0x6c, 0x64, 0x5f, 0x31, 0x18, 0x03, 0x20, 0x01, 0x28, 0x05, 0x48, 0x00, 0x52,
	0x1f, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x43, 0x6f, 0x6e, 0x66, 0x6c, 0x69, 0x63, 0x74, 0x69, 0x6e,
	0x67, 0x50, 0x72, 0x69, 0x6d, 0x69, 0x74, 0x69, 0x76, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x31,
	0x12, 0x4e, 0x0a, 0x23, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x5f, 0x63, 0x6f, 0x6e, 0x66, 0x6c, 0x69,
	0x63, 0x74, 0x69, 0x6e, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6d, 0x69, 0x74, 0x69, 0x76, 0x65, 0x5f,
	0x66, 0x69, 0x65, 0x6c, 0x64, 0x5f, 0x32, 0x18, 0x04, 0x20, 0x01, 0x28, 0x05, 0x48, 0x00, 0x52,
	0x1f, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x43, 0x6f, 0x6e, 0x66, 0x6c, 0x69, 0x63, 0x74, 0x69, 0x6e,
	0x67, 0x50, 0x72, 0x69, 0x6d, 0x69, 0x74, 0x69, 0x76, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x32,
	0x12, 0x6d, 0x0a, 0x13, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x5f, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67,
	0x65, 0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x18, 0x05, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x3b, 0x2e,
	0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e, 0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d,
	0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x61, 0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74,
	0x6f, 0x70, 0x61, 0x74, 0x68, 0x2e, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2e, 0x49, 0x6e,
	0x6e, 0x65, 0x72, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x48, 0x00, 0x52, 0x11, 0x6f, 0x6e,
	0x65, 0x6f, 0x66, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x12,
	0x88, 0x01, 0x0a, 0x21, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x5f, 0x63, 0x6f, 0x6e, 0x66, 0x6c, 0x69,
	0x63, 0x74, 0x69, 0x6e, 0x67, 0x5f, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x5f, 0x66, 0x69,
	0x65, 0x6c, 0x64, 0x5f, 0x31, 0x18, 0x06, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x3c, 0x2e, 0x66, 0x68,
	0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e, 0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74,
	0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x61, 0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70,
	0x61, 0x74, 0x68, 0x2e, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2e, 0x49, 0x6e, 0x6e, 0x65,
	0x72, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x32, 0x48, 0x00, 0x52, 0x1d, 0x6f, 0x6e, 0x65,
	0x6f, 0x66, 0x43, 0x6f, 0x6e, 0x66, 0x6c, 0x69, 0x63, 0x74, 0x69, 0x6e, 0x67, 0x4d, 0x65, 0x73,
	0x73, 0x61, 0x67, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x31, 0x12, 0x88, 0x01, 0x0a, 0x21, 0x6f,
	0x6e, 0x65, 0x6f, 0x66, 0x5f, 0x63, 0x6f, 0x6e, 0x66, 0x6c, 0x69, 0x63, 0x74, 0x69, 0x6e, 0x67,
	0x5f, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x5f, 0x32,
	0x18, 0x07, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x3c, 0x2e, 0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f,
	0x2e, 0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65,
	0x72, 0x6e, 0x61, 0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x2e, 0x4d,
	0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2e, 0x49, 0x6e, 0x6e, 0x65, 0x72, 0x4d, 0x65, 0x73, 0x73,
	0x61, 0x67, 0x65, 0x32, 0x48, 0x00, 0x52, 0x1d, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x43, 0x6f, 0x6e,
	0x66, 0x6c, 0x69, 0x63, 0x74, 0x69, 0x6e, 0x67, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x46,
	0x69, 0x65, 0x6c, 0x64, 0x32, 0x12, 0x46, 0x0a, 0x04, 0x74, 0x79, 0x70, 0x65, 0x18, 0x08, 0x20,
	0x01, 0x28, 0x0e, 0x32, 0x32, 0x2e, 0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e, 0x6a, 0x73,
	0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x61,
	0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x2e, 0x4d, 0x65, 0x73, 0x73,
	0x61, 0x67, 0x65, 0x54, 0x79, 0x70, 0x65, 0x52, 0x04, 0x74, 0x79, 0x70, 0x65, 0x12, 0x60, 0x0a,
	0x0d, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x18, 0x09,
	0x20, 0x01, 0x28, 0x0b, 0x32, 0x3b, 0x2e, 0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e, 0x6a,
	0x73, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e,
	0x61, 0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x2e, 0x4d, 0x65, 0x73,
	0x73, 0x61, 0x67, 0x65, 0x2e, 0x49, 0x6e, 0x6e, 0x65, 0x72, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67,
	0x65, 0x52, 0x0c, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x12,
	0x71, 0x0a, 0x16, 0x72, 0x65, 0x70, 0x65, 0x61, 0x74, 0x65, 0x64, 0x5f, 0x6d, 0x65, 0x73, 0x73,
	0x61, 0x67, 0x65, 0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x18, 0x0a, 0x20, 0x03, 0x28, 0x0b, 0x32,
	0x3b, 0x2e, 0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e, 0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f,
	0x72, 0x6d, 0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x61, 0x6c, 0x2e, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x2e, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2e,
	0x49, 0x6e, 0x6e, 0x65, 0x72, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x52, 0x14, 0x72, 0x65,
	0x70, 0x65, 0x61, 0x74, 0x65, 0x64, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x46, 0x69, 0x65,
	0x6c, 0x64, 0x12, 0x21, 0x0a, 0x0c, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x66, 0x69, 0x65,
	0x6c, 0x64, 0x18, 0x0b, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0b, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67,
	0x46, 0x69, 0x65, 0x6c, 0x64, 0x12, 0x32, 0x0a, 0x1b, 0x6a, 0x73, 0x6f, 0x6e, 0x5f, 0x61, 0x6e,
	0x6e, 0x6f, 0x74, 0x61, 0x74, 0x65, 0x64, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x66,
	0x69, 0x65, 0x6c, 0x64, 0x18, 0x0c, 0x20, 0x01, 0x28, 0x09, 0x52, 0x0d, 0x6a, 0x73, 0x6f, 0x6e,
	0x41, 0x6e, 0x6e, 0x6f, 0x74, 0x61, 0x74, 0x65, 0x64, 0x1a, 0xea, 0x02, 0x0a, 0x0c, 0x49, 0x6e,
	0x6e, 0x65, 0x72, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x12, 0x1f, 0x0a, 0x0b, 0x69, 0x6e,
	0x6e, 0x65, 0x72, 0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x05, 0x52,
	0x0a, 0x69, 0x6e, 0x6e, 0x65, 0x72, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x12, 0x30, 0x0a, 0x14, 0x72,
	0x65, 0x70, 0x65, 0x61, 0x74, 0x65, 0x64, 0x5f, 0x69, 0x6e, 0x6e, 0x65, 0x72, 0x5f, 0x66, 0x69,
	0x65, 0x6c, 0x64, 0x18, 0x02, 0x20, 0x03, 0x28, 0x05, 0x52, 0x12, 0x72, 0x65, 0x70, 0x65, 0x61,
	0x74, 0x65, 0x64, 0x49, 0x6e, 0x6e, 0x65, 0x72, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x12, 0x7d, 0x0a,
	0x1c, 0x72, 0x65, 0x70, 0x65, 0x61, 0x74, 0x65, 0x64, 0x5f, 0x69, 0x6e, 0x6e, 0x65, 0x72, 0x5f,
	0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x18, 0x03, 0x20,
	0x03, 0x28, 0x0b, 0x32, 0x3c, 0x2e, 0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e, 0x6a, 0x73,
	0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x61,
	0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x2e, 0x4d, 0x65, 0x73, 0x73,
	0x61, 0x67, 0x65, 0x2e, 0x49, 0x6e, 0x6e, 0x65, 0x72, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65,
	0x32, 0x52, 0x19, 0x72, 0x65, 0x70, 0x65, 0x61, 0x74, 0x65, 0x64, 0x49, 0x6e, 0x6e, 0x65, 0x72,
	0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x12, 0x79, 0x0a, 0x19,
	0x69, 0x6e, 0x6e, 0x65, 0x72, 0x5f, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x5f, 0x6d, 0x65, 0x73, 0x73,
	0x61, 0x67, 0x65, 0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x18, 0x04, 0x20, 0x01, 0x28, 0x0b, 0x32,
	0x3c, 0x2e, 0x66, 0x68, 0x69, 0x72, 0x2e, 0x67, 0x6f, 0x2e, 0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f,
	0x72, 0x6d, 0x61, 0x74, 0x2e, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x61, 0x6c, 0x2e, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x2e, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2e,
	0x49, 0x6e, 0x6e, 0x65, 0x72, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x32, 0x48, 0x00, 0x52,
	0x16, 0x69, 0x6e, 0x6e, 0x65, 0x72, 0x4f, 0x6e, 0x65, 0x6f, 0x66, 0x4d, 0x65, 0x73, 0x73, 0x61,
	0x67, 0x65, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x42, 0x0d, 0x0a, 0x0b, 0x69, 0x6e, 0x6e, 0x65, 0x72,
	0x5f, 0x6f, 0x6e, 0x65, 0x6f, 0x66, 0x1a, 0x32, 0x0a, 0x0d, 0x49, 0x6e, 0x6e, 0x65, 0x72, 0x4d,
	0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x32, 0x12, 0x21, 0x0a, 0x0c, 0x69, 0x6e, 0x6e, 0x65, 0x72,
	0x5f, 0x66, 0x69, 0x65, 0x6c, 0x64, 0x32, 0x18, 0x01, 0x20, 0x01, 0x28, 0x05, 0x52, 0x0b, 0x69,
	0x6e, 0x6e, 0x65, 0x72, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x32, 0x42, 0x07, 0x0a, 0x05, 0x6f, 0x6e,
	0x65, 0x6f, 0x66, 0x2a, 0x40, 0x0a, 0x0b, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x54, 0x79,
	0x70, 0x65, 0x12, 0x19, 0x0a, 0x15, 0x49, 0x4e, 0x56, 0x41, 0x4c, 0x49, 0x44, 0x5f, 0x55, 0x4e,
	0x49, 0x4e, 0x49, 0x54, 0x49, 0x41, 0x4c, 0x49, 0x5a, 0x45, 0x44, 0x10, 0x00, 0x12, 0x0a, 0x0a,
	0x06, 0x54, 0x59, 0x50, 0x45, 0x5f, 0x31, 0x10, 0x01, 0x12, 0x0a, 0x0a, 0x06, 0x54, 0x59, 0x50,
	0x45, 0x5f, 0x32, 0x10, 0x02, 0x42, 0x50, 0x5a, 0x4e, 0x67, 0x69, 0x74, 0x68, 0x75, 0x62, 0x2e,
	0x63, 0x6f, 0x6d, 0x2f, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2f, 0x66, 0x68, 0x69, 0x72, 0x2f,
	0x67, 0x6f, 0x2f, 0x6a, 0x73, 0x6f, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x2f, 0x69, 0x6e,
	0x74, 0x65, 0x72, 0x6e, 0x61, 0x6c, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68,
	0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x70, 0x61, 0x74, 0x68, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x67,
	0x6f, 0x5f, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescOnce sync.Once
	file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescData = file_go_jsonformat_internal_protopath_protopathtest_proto_rawDesc
)

func file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescGZIP() []byte {
	file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescOnce.Do(func() {
		file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescData = protoimpl.X.CompressGZIP(file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescData)
	})
	return file_go_jsonformat_internal_protopath_protopathtest_proto_rawDescData
}

var file_go_jsonformat_internal_protopath_protopathtest_proto_enumTypes = make([]protoimpl.EnumInfo, 1)
var file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes = make([]protoimpl.MessageInfo, 4)
var file_go_jsonformat_internal_protopath_protopathtest_proto_goTypes = []interface{}{
	(MessageType)(0),              // 0: fhir.go.jsonformat.internal.protopath.MessageType
	(*Missing)(nil),               // 1: fhir.go.jsonformat.internal.protopath.Missing
	(*Message)(nil),               // 2: fhir.go.jsonformat.internal.protopath.Message
	(*Message_InnerMessage)(nil),  // 3: fhir.go.jsonformat.internal.protopath.Message.InnerMessage
	(*Message_InnerMessage2)(nil), // 4: fhir.go.jsonformat.internal.protopath.Message.InnerMessage2
}
var file_go_jsonformat_internal_protopath_protopathtest_proto_depIdxs = []int32{
	3, // 0: fhir.go.jsonformat.internal.protopath.Message.oneof_message_field:type_name -> fhir.go.jsonformat.internal.protopath.Message.InnerMessage
	4, // 1: fhir.go.jsonformat.internal.protopath.Message.oneof_conflicting_message_field_1:type_name -> fhir.go.jsonformat.internal.protopath.Message.InnerMessage2
	4, // 2: fhir.go.jsonformat.internal.protopath.Message.oneof_conflicting_message_field_2:type_name -> fhir.go.jsonformat.internal.protopath.Message.InnerMessage2
	0, // 3: fhir.go.jsonformat.internal.protopath.Message.type:type_name -> fhir.go.jsonformat.internal.protopath.MessageType
	3, // 4: fhir.go.jsonformat.internal.protopath.Message.message_field:type_name -> fhir.go.jsonformat.internal.protopath.Message.InnerMessage
	3, // 5: fhir.go.jsonformat.internal.protopath.Message.repeated_message_field:type_name -> fhir.go.jsonformat.internal.protopath.Message.InnerMessage
	4, // 6: fhir.go.jsonformat.internal.protopath.Message.InnerMessage.repeated_inner_message_field:type_name -> fhir.go.jsonformat.internal.protopath.Message.InnerMessage2
	4, // 7: fhir.go.jsonformat.internal.protopath.Message.InnerMessage.inner_oneof_message_field:type_name -> fhir.go.jsonformat.internal.protopath.Message.InnerMessage2
	8, // [8:8] is the sub-list for method output_type
	8, // [8:8] is the sub-list for method input_type
	8, // [8:8] is the sub-list for extension type_name
	8, // [8:8] is the sub-list for extension extendee
	0, // [0:8] is the sub-list for field type_name
}

func init() { file_go_jsonformat_internal_protopath_protopathtest_proto_init() }
func file_go_jsonformat_internal_protopath_protopathtest_proto_init() {
	if File_go_jsonformat_internal_protopath_protopathtest_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Missing); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Message); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Message_InnerMessage); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*Message_InnerMessage2); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[1].OneofWrappers = []interface{}{
		(*Message_OneofPrimitiveField)(nil),
		(*Message_OneofConflictingPrimitiveField_1)(nil),
		(*Message_OneofConflictingPrimitiveField_2)(nil),
		(*Message_OneofMessageField)(nil),
		(*Message_OneofConflictingMessageField_1)(nil),
		(*Message_OneofConflictingMessageField_2)(nil),
	}
	file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes[2].OneofWrappers = []interface{}{
		(*Message_InnerMessage_InnerOneofMessageField)(nil),
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_go_jsonformat_internal_protopath_protopathtest_proto_rawDesc,
			NumEnums:      1,
			NumMessages:   4,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_go_jsonformat_internal_protopath_protopathtest_proto_goTypes,
		DependencyIndexes: file_go_jsonformat_internal_protopath_protopathtest_proto_depIdxs,
		EnumInfos:         file_go_jsonformat_internal_protopath_protopathtest_proto_enumTypes,
		MessageInfos:      file_go_jsonformat_internal_protopath_protopathtest_proto_msgTypes,
	}.Build()
	File_go_jsonformat_internal_protopath_protopathtest_proto = out.File
	file_go_jsonformat_internal_protopath_protopathtest_proto_rawDesc = nil
	file_go_jsonformat_internal_protopath_protopathtest_proto_goTypes = nil
	file_go_jsonformat_internal_protopath_protopathtest_proto_depIdxs = nil
}
