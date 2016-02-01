// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: GenTbl.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "GenTbl.pb.h"

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

const ::google::protobuf::Descriptor* CaffeGenTbl_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  CaffeGenTbl_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* CaffeGenTbl_TblType_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_GenTbl_2eproto() {
  protobuf_AddDesc_GenTbl_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "GenTbl.proto");
  GOOGLE_CHECK(file != NULL);
  CaffeGenTbl_descriptor_ = file->message_type(0);
  static const int CaffeGenTbl_offsets_[7] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, name_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, files_core_dir_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, tbl_type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, input_vec_file_name_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, max_words_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, alpha_only_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, one_hot_),
  };
  CaffeGenTbl_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      CaffeGenTbl_descriptor_,
      CaffeGenTbl::default_instance_,
      CaffeGenTbl_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(CaffeGenTbl, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(CaffeGenTbl));
  CaffeGenTbl_TblType_descriptor_ = CaffeGenTbl_descriptor_->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_GenTbl_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    CaffeGenTbl_descriptor_, &CaffeGenTbl::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_GenTbl_2eproto() {
  delete CaffeGenTbl::default_instance_;
  delete CaffeGenTbl_reflection_;
}

void protobuf_AddDesc_GenTbl_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\014GenTbl.proto\"\362\001\n\013CaffeGenTbl\022\014\n\004name\030\001"
    " \002(\t\022\026\n\016files_core_dir\030\002 \002(\t\022&\n\010tbl_type"
    "\030\003 \002(\0162\024.CaffeGenTbl.TblType\022\033\n\023input_ve"
    "c_file_name\030\004 \001(\t\022\021\n\tmax_words\030\005 \001(\005\022\022\n\n"
    "alpha_only\030\006 \001(\010\022\017\n\007one_hot\030\007 \001(\010\"@\n\007Tbl"
    "Type\022\021\n\rTBL_TYPE_WORD\020\001\022\020\n\014TBL_TYPE_POS\020"
    "\002\022\020\n\014TBL_TYPE_DEP\020\003", 259);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "GenTbl.proto", &protobuf_RegisterTypes);
  CaffeGenTbl::default_instance_ = new CaffeGenTbl();
  CaffeGenTbl::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_GenTbl_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_GenTbl_2eproto {
  StaticDescriptorInitializer_GenTbl_2eproto() {
    protobuf_AddDesc_GenTbl_2eproto();
  }
} static_descriptor_initializer_GenTbl_2eproto_;

// ===================================================================

const ::google::protobuf::EnumDescriptor* CaffeGenTbl_TblType_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return CaffeGenTbl_TblType_descriptor_;
}
bool CaffeGenTbl_TblType_IsValid(int value) {
  switch(value) {
    case 1:
    case 2:
    case 3:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const CaffeGenTbl_TblType CaffeGenTbl::TBL_TYPE_WORD;
const CaffeGenTbl_TblType CaffeGenTbl::TBL_TYPE_POS;
const CaffeGenTbl_TblType CaffeGenTbl::TBL_TYPE_DEP;
const CaffeGenTbl_TblType CaffeGenTbl::TblType_MIN;
const CaffeGenTbl_TblType CaffeGenTbl::TblType_MAX;
const int CaffeGenTbl::TblType_ARRAYSIZE;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int CaffeGenTbl::kNameFieldNumber;
const int CaffeGenTbl::kFilesCoreDirFieldNumber;
const int CaffeGenTbl::kTblTypeFieldNumber;
const int CaffeGenTbl::kInputVecFileNameFieldNumber;
const int CaffeGenTbl::kMaxWordsFieldNumber;
const int CaffeGenTbl::kAlphaOnlyFieldNumber;
const int CaffeGenTbl::kOneHotFieldNumber;
#endif  // !_MSC_VER

CaffeGenTbl::CaffeGenTbl()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void CaffeGenTbl::InitAsDefaultInstance() {
}

CaffeGenTbl::CaffeGenTbl(const CaffeGenTbl& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void CaffeGenTbl::SharedCtor() {
  _cached_size_ = 0;
  name_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  files_core_dir_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  tbl_type_ = 1;
  input_vec_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  max_words_ = 0;
  alpha_only_ = false;
  one_hot_ = false;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

CaffeGenTbl::~CaffeGenTbl() {
  SharedDtor();
}

void CaffeGenTbl::SharedDtor() {
  if (name_ != &::google::protobuf::internal::kEmptyString) {
    delete name_;
  }
  if (files_core_dir_ != &::google::protobuf::internal::kEmptyString) {
    delete files_core_dir_;
  }
  if (input_vec_file_name_ != &::google::protobuf::internal::kEmptyString) {
    delete input_vec_file_name_;
  }
  if (this != default_instance_) {
  }
}

void CaffeGenTbl::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* CaffeGenTbl::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return CaffeGenTbl_descriptor_;
}

const CaffeGenTbl& CaffeGenTbl::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_GenTbl_2eproto();
  return *default_instance_;
}

CaffeGenTbl* CaffeGenTbl::default_instance_ = NULL;

CaffeGenTbl* CaffeGenTbl::New() const {
  return new CaffeGenTbl;
}

void CaffeGenTbl::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_name()) {
      if (name_ != &::google::protobuf::internal::kEmptyString) {
        name_->clear();
      }
    }
    if (has_files_core_dir()) {
      if (files_core_dir_ != &::google::protobuf::internal::kEmptyString) {
        files_core_dir_->clear();
      }
    }
    tbl_type_ = 1;
    if (has_input_vec_file_name()) {
      if (input_vec_file_name_ != &::google::protobuf::internal::kEmptyString) {
        input_vec_file_name_->clear();
      }
    }
    max_words_ = 0;
    alpha_only_ = false;
    one_hot_ = false;
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool CaffeGenTbl::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string name = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->name().data(), this->name().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_files_core_dir;
        break;
      }

      // required string files_core_dir = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_files_core_dir:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_files_core_dir()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->files_core_dir().data(), this->files_core_dir().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(24)) goto parse_tbl_type;
        break;
      }

      // required .CaffeGenTbl.TblType tbl_type = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_tbl_type:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::CaffeGenTbl_TblType_IsValid(value)) {
            set_tbl_type(static_cast< ::CaffeGenTbl_TblType >(value));
          } else {
            mutable_unknown_fields()->AddVarint(3, value);
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(34)) goto parse_input_vec_file_name;
        break;
      }

      // optional string input_vec_file_name = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_input_vec_file_name:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_input_vec_file_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->input_vec_file_name().data(), this->input_vec_file_name().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(40)) goto parse_max_words;
        break;
      }

      // optional int32 max_words = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_max_words:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &max_words_)));
          set_has_max_words();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(48)) goto parse_alpha_only;
        break;
      }

      // optional bool alpha_only = 6;
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_alpha_only:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   bool, ::google::protobuf::internal::WireFormatLite::TYPE_BOOL>(
                 input, &alpha_only_)));
          set_has_alpha_only();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(56)) goto parse_one_hot;
        break;
      }

      // optional bool one_hot = 7;
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_one_hot:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   bool, ::google::protobuf::internal::WireFormatLite::TYPE_BOOL>(
                 input, &one_hot_)));
          set_has_one_hot();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void CaffeGenTbl::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required string name = 1;
  if (has_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->name().data(), this->name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      1, this->name(), output);
  }

  // required string files_core_dir = 2;
  if (has_files_core_dir()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->files_core_dir().data(), this->files_core_dir().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->files_core_dir(), output);
  }

  // required .CaffeGenTbl.TblType tbl_type = 3;
  if (has_tbl_type()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      3, this->tbl_type(), output);
  }

  // optional string input_vec_file_name = 4;
  if (has_input_vec_file_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->input_vec_file_name().data(), this->input_vec_file_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      4, this->input_vec_file_name(), output);
  }

  // optional int32 max_words = 5;
  if (has_max_words()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(5, this->max_words(), output);
  }

  // optional bool alpha_only = 6;
  if (has_alpha_only()) {
    ::google::protobuf::internal::WireFormatLite::WriteBool(6, this->alpha_only(), output);
  }

  // optional bool one_hot = 7;
  if (has_one_hot()) {
    ::google::protobuf::internal::WireFormatLite::WriteBool(7, this->one_hot(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* CaffeGenTbl::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required string name = 1;
  if (has_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->name().data(), this->name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->name(), target);
  }

  // required string files_core_dir = 2;
  if (has_files_core_dir()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->files_core_dir().data(), this->files_core_dir().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->files_core_dir(), target);
  }

  // required .CaffeGenTbl.TblType tbl_type = 3;
  if (has_tbl_type()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      3, this->tbl_type(), target);
  }

  // optional string input_vec_file_name = 4;
  if (has_input_vec_file_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->input_vec_file_name().data(), this->input_vec_file_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        4, this->input_vec_file_name(), target);
  }

  // optional int32 max_words = 5;
  if (has_max_words()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(5, this->max_words(), target);
  }

  // optional bool alpha_only = 6;
  if (has_alpha_only()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteBoolToArray(6, this->alpha_only(), target);
  }

  // optional bool one_hot = 7;
  if (has_one_hot()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteBoolToArray(7, this->one_hot(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int CaffeGenTbl::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required string name = 1;
    if (has_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->name());
    }

    // required string files_core_dir = 2;
    if (has_files_core_dir()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->files_core_dir());
    }

    // required .CaffeGenTbl.TblType tbl_type = 3;
    if (has_tbl_type()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->tbl_type());
    }

    // optional string input_vec_file_name = 4;
    if (has_input_vec_file_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->input_vec_file_name());
    }

    // optional int32 max_words = 5;
    if (has_max_words()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->max_words());
    }

    // optional bool alpha_only = 6;
    if (has_alpha_only()) {
      total_size += 1 + 1;
    }

    // optional bool one_hot = 7;
    if (has_one_hot()) {
      total_size += 1 + 1;
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

void CaffeGenTbl::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const CaffeGenTbl* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const CaffeGenTbl*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void CaffeGenTbl::MergeFrom(const CaffeGenTbl& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_name()) {
      set_name(from.name());
    }
    if (from.has_files_core_dir()) {
      set_files_core_dir(from.files_core_dir());
    }
    if (from.has_tbl_type()) {
      set_tbl_type(from.tbl_type());
    }
    if (from.has_input_vec_file_name()) {
      set_input_vec_file_name(from.input_vec_file_name());
    }
    if (from.has_max_words()) {
      set_max_words(from.max_words());
    }
    if (from.has_alpha_only()) {
      set_alpha_only(from.alpha_only());
    }
    if (from.has_one_hot()) {
      set_one_hot(from.one_hot());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void CaffeGenTbl::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void CaffeGenTbl::CopyFrom(const CaffeGenTbl& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool CaffeGenTbl::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;

  return true;
}

void CaffeGenTbl::Swap(CaffeGenTbl* other) {
  if (other != this) {
    std::swap(name_, other->name_);
    std::swap(files_core_dir_, other->files_core_dir_);
    std::swap(tbl_type_, other->tbl_type_);
    std::swap(input_vec_file_name_, other->input_vec_file_name_);
    std::swap(max_words_, other->max_words_);
    std::swap(alpha_only_, other->alpha_only_);
    std::swap(one_hot_, other->one_hot_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata CaffeGenTbl::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = CaffeGenTbl_descriptor_;
  metadata.reflection = CaffeGenTbl_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)
