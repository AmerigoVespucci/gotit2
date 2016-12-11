// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: GenOutput.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "GenOutput.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace {

const ::google::protobuf::Descriptor* CaffeGenOutput_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  CaffeGenOutput_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_GenOutput_2eproto() {
  protobuf_AddDesc_GenOutput_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "GenOutput.proto");
  GOOGLE_CHECK(file != NULL);
  CaffeGenOutput_descriptor_ = file->message_type(0);
  static const int CaffeGenOutput_offsets_[6] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, num_input_nodes_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, num_output_nodes_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, input_layer_idx_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, input_layer_top_idx_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, output_layer_idx_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, output_layer_top_idx_),
  };
  CaffeGenOutput_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      CaffeGenOutput_descriptor_,
      CaffeGenOutput::default_instance_,
      CaffeGenOutput_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenOutput, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(CaffeGenOutput));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_GenOutput_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    CaffeGenOutput_descriptor_, &CaffeGenOutput::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_GenOutput_2eproto() {
  delete CaffeGenOutput::default_instance_;
  delete CaffeGenOutput_reflection_;
}

void protobuf_AddDesc_GenOutput_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\017GenOutput.proto\"\261\001\n\016CaffeGenOutput\022\027\n\017"
    "num_input_nodes\030\001 \002(\005\022\030\n\020num_output_node"
    "s\030\002 \002(\005\022\027\n\017input_layer_idx\030\003 \002(\005\022\033\n\023inpu"
    "t_layer_top_idx\030\004 \002(\005\022\030\n\020output_layer_id"
    "x\030\005 \002(\005\022\034\n\024output_layer_top_idx\030\006 \002(\005", 197);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "GenOutput.proto", &protobuf_RegisterTypes);
  CaffeGenOutput::default_instance_ = new CaffeGenOutput();
  CaffeGenOutput::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_GenOutput_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_GenOutput_2eproto {
  StaticDescriptorInitializer_GenOutput_2eproto() {
    protobuf_AddDesc_GenOutput_2eproto();
  }
} static_descriptor_initializer_GenOutput_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int CaffeGenOutput::kNumInputNodesFieldNumber;
const int CaffeGenOutput::kNumOutputNodesFieldNumber;
const int CaffeGenOutput::kInputLayerIdxFieldNumber;
const int CaffeGenOutput::kInputLayerTopIdxFieldNumber;
const int CaffeGenOutput::kOutputLayerIdxFieldNumber;
const int CaffeGenOutput::kOutputLayerTopIdxFieldNumber;
#endif  // !_MSC_VER

CaffeGenOutput::CaffeGenOutput()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:CaffeGenOutput)
}

void CaffeGenOutput::InitAsDefaultInstance() {
}

CaffeGenOutput::CaffeGenOutput(const CaffeGenOutput& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:CaffeGenOutput)
}

void CaffeGenOutput::SharedCtor() {
  _cached_size_ = 0;
  num_input_nodes_ = 0;
  num_output_nodes_ = 0;
  input_layer_idx_ = 0;
  input_layer_top_idx_ = 0;
  output_layer_idx_ = 0;
  output_layer_top_idx_ = 0;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CaffeGenOutput::~CaffeGenOutput() {
  // @@protoc_insertion_point(destructor:CaffeGenOutput)
  SharedDtor();
}

void CaffeGenOutput::SharedDtor() {
  if (this != default_instance_) {
  }
}

void CaffeGenOutput::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* CaffeGenOutput::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return CaffeGenOutput_descriptor_;
}

const CaffeGenOutput& CaffeGenOutput::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_GenOutput_2eproto();
  return *default_instance_;
}

CaffeGenOutput* CaffeGenOutput::default_instance_ = NULL;

CaffeGenOutput* CaffeGenOutput::New() const {
  return new CaffeGenOutput;
}

void CaffeGenOutput::Clear() {
#define OFFSET_OF_FIELD_(f) (reinterpret_cast<char*>(      \
  &reinterpret_cast<CaffeGenOutput*>(16)->f) - \
   reinterpret_cast<char*>(16))

#define ZR_(first, last) do {                              \
    size_t f = OFFSET_OF_FIELD_(first);                    \
    size_t n = OFFSET_OF_FIELD_(last) - f + sizeof(last);  \
    ::memset(&first, 0, n);                                \
  } while (0)

  if (_has_bits_[0 / 32] & 63) {
    ZR_(num_input_nodes_, output_layer_top_idx_);
  }

#undef OFFSET_OF_FIELD_
#undef ZR_

  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool CaffeGenOutput::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:CaffeGenOutput)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 num_input_nodes = 1;
      case 1: {
        if (tag == 8) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &num_input_nodes_)));
          set_has_num_input_nodes();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(16)) goto parse_num_output_nodes;
        break;
      }

      // required int32 num_output_nodes = 2;
      case 2: {
        if (tag == 16) {
         parse_num_output_nodes:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &num_output_nodes_)));
          set_has_num_output_nodes();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(24)) goto parse_input_layer_idx;
        break;
      }

      // required int32 input_layer_idx = 3;
      case 3: {
        if (tag == 24) {
         parse_input_layer_idx:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &input_layer_idx_)));
          set_has_input_layer_idx();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(32)) goto parse_input_layer_top_idx;
        break;
      }

      // required int32 input_layer_top_idx = 4;
      case 4: {
        if (tag == 32) {
         parse_input_layer_top_idx:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &input_layer_top_idx_)));
          set_has_input_layer_top_idx();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(40)) goto parse_output_layer_idx;
        break;
      }

      // required int32 output_layer_idx = 5;
      case 5: {
        if (tag == 40) {
         parse_output_layer_idx:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &output_layer_idx_)));
          set_has_output_layer_idx();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(48)) goto parse_output_layer_top_idx;
        break;
      }

      // required int32 output_layer_top_idx = 6;
      case 6: {
        if (tag == 48) {
         parse_output_layer_top_idx:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &output_layer_top_idx_)));
          set_has_output_layer_top_idx();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:CaffeGenOutput)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:CaffeGenOutput)
  return false;
#undef DO_
}

void CaffeGenOutput::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:CaffeGenOutput)
  // required int32 num_input_nodes = 1;
  if (has_num_input_nodes()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(1, this->num_input_nodes(), output);
  }

  // required int32 num_output_nodes = 2;
  if (has_num_output_nodes()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(2, this->num_output_nodes(), output);
  }

  // required int32 input_layer_idx = 3;
  if (has_input_layer_idx()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->input_layer_idx(), output);
  }

  // required int32 input_layer_top_idx = 4;
  if (has_input_layer_top_idx()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(4, this->input_layer_top_idx(), output);
  }

  // required int32 output_layer_idx = 5;
  if (has_output_layer_idx()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(5, this->output_layer_idx(), output);
  }

  // required int32 output_layer_top_idx = 6;
  if (has_output_layer_top_idx()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(6, this->output_layer_top_idx(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:CaffeGenOutput)
}

::google::protobuf::uint8* CaffeGenOutput::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:CaffeGenOutput)
  // required int32 num_input_nodes = 1;
  if (has_num_input_nodes()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(1, this->num_input_nodes(), target);
  }

  // required int32 num_output_nodes = 2;
  if (has_num_output_nodes()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(2, this->num_output_nodes(), target);
  }

  // required int32 input_layer_idx = 3;
  if (has_input_layer_idx()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->input_layer_idx(), target);
  }

  // required int32 input_layer_top_idx = 4;
  if (has_input_layer_top_idx()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(4, this->input_layer_top_idx(), target);
  }

  // required int32 output_layer_idx = 5;
  if (has_output_layer_idx()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(5, this->output_layer_idx(), target);
  }

  // required int32 output_layer_top_idx = 6;
  if (has_output_layer_top_idx()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(6, this->output_layer_top_idx(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:CaffeGenOutput)
  return target;
}

int CaffeGenOutput::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 num_input_nodes = 1;
    if (has_num_input_nodes()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->num_input_nodes());
    }

    // required int32 num_output_nodes = 2;
    if (has_num_output_nodes()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->num_output_nodes());
    }

    // required int32 input_layer_idx = 3;
    if (has_input_layer_idx()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->input_layer_idx());
    }

    // required int32 input_layer_top_idx = 4;
    if (has_input_layer_top_idx()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->input_layer_top_idx());
    }

    // required int32 output_layer_idx = 5;
    if (has_output_layer_idx()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->output_layer_idx());
    }

    // required int32 output_layer_top_idx = 6;
    if (has_output_layer_top_idx()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->output_layer_top_idx());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void CaffeGenOutput::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const CaffeGenOutput* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const CaffeGenOutput*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void CaffeGenOutput::MergeFrom(const CaffeGenOutput& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_num_input_nodes()) {
      set_num_input_nodes(from.num_input_nodes());
    }
    if (from.has_num_output_nodes()) {
      set_num_output_nodes(from.num_output_nodes());
    }
    if (from.has_input_layer_idx()) {
      set_input_layer_idx(from.input_layer_idx());
    }
    if (from.has_input_layer_top_idx()) {
      set_input_layer_top_idx(from.input_layer_top_idx());
    }
    if (from.has_output_layer_idx()) {
      set_output_layer_idx(from.output_layer_idx());
    }
    if (from.has_output_layer_top_idx()) {
      set_output_layer_top_idx(from.output_layer_top_idx());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void CaffeGenOutput::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void CaffeGenOutput::CopyFrom(const CaffeGenOutput& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CaffeGenOutput::IsInitialized() const {
  if ((_has_bits_[0] & 0x0000003f) != 0x0000003f) return false;

  return true;
}

void CaffeGenOutput::Swap(CaffeGenOutput* other) {
  if (other != this) {
    std::swap(num_input_nodes_, other->num_input_nodes_);
    std::swap(num_output_nodes_, other->num_output_nodes_);
    std::swap(input_layer_idx_, other->input_layer_idx_);
    std::swap(input_layer_top_idx_, other->input_layer_top_idx_);
    std::swap(output_layer_idx_, other->output_layer_idx_);
    std::swap(output_layer_top_idx_, other->output_layer_top_idx_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata CaffeGenOutput::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = CaffeGenOutput_descriptor_;
  metadata.reflection = CaffeGenOutput_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)