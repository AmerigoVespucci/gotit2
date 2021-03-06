// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: GenSeed.proto

#ifndef PROTOBUF_GenSeed_2eproto__INCLUDED
#define PROTOBUF_GenSeed_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_GenSeed_2eproto();
void protobuf_AssignDesc_GenSeed_2eproto();
void protobuf_ShutdownFile_GenSeed_2eproto();

class CaffeGenSeed;

enum CaffeGenSeed_NetEndType {
  CaffeGenSeed_NetEndType_END_VALID = 1,
  CaffeGenSeed_NetEndType_END_ONE_HOT = 2,
  CaffeGenSeed_NetEndType_END_MULTI_HOT = 3
};
bool CaffeGenSeed_NetEndType_IsValid(int value);
const CaffeGenSeed_NetEndType CaffeGenSeed_NetEndType_NetEndType_MIN = CaffeGenSeed_NetEndType_END_VALID;
const CaffeGenSeed_NetEndType CaffeGenSeed_NetEndType_NetEndType_MAX = CaffeGenSeed_NetEndType_END_MULTI_HOT;
const int CaffeGenSeed_NetEndType_NetEndType_ARRAYSIZE = CaffeGenSeed_NetEndType_NetEndType_MAX + 1;

const ::google::protobuf::EnumDescriptor* CaffeGenSeed_NetEndType_descriptor();
inline const ::std::string& CaffeGenSeed_NetEndType_Name(CaffeGenSeed_NetEndType value) {
  return ::google::protobuf::internal::NameOfEnum(
    CaffeGenSeed_NetEndType_descriptor(), value);
}
inline bool CaffeGenSeed_NetEndType_Parse(
    const ::std::string& name, CaffeGenSeed_NetEndType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<CaffeGenSeed_NetEndType>(
    CaffeGenSeed_NetEndType_descriptor(), name, value);
}
// ===================================================================

class CaffeGenSeed : public ::google::protobuf::Message {
 public:
  CaffeGenSeed();
  virtual ~CaffeGenSeed();

  CaffeGenSeed(const CaffeGenSeed& from);

  inline CaffeGenSeed& operator=(const CaffeGenSeed& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CaffeGenSeed& default_instance();

  void Swap(CaffeGenSeed* other);

  // implements Message ----------------------------------------------

  CaffeGenSeed* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CaffeGenSeed& from);
  void MergeFrom(const CaffeGenSeed& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef CaffeGenSeed_NetEndType NetEndType;
  static const NetEndType END_VALID = CaffeGenSeed_NetEndType_END_VALID;
  static const NetEndType END_ONE_HOT = CaffeGenSeed_NetEndType_END_ONE_HOT;
  static const NetEndType END_MULTI_HOT = CaffeGenSeed_NetEndType_END_MULTI_HOT;
  static inline bool NetEndType_IsValid(int value) {
    return CaffeGenSeed_NetEndType_IsValid(value);
  }
  static const NetEndType NetEndType_MIN =
    CaffeGenSeed_NetEndType_NetEndType_MIN;
  static const NetEndType NetEndType_MAX =
    CaffeGenSeed_NetEndType_NetEndType_MAX;
  static const int NetEndType_ARRAYSIZE =
    CaffeGenSeed_NetEndType_NetEndType_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  NetEndType_descriptor() {
    return CaffeGenSeed_NetEndType_descriptor();
  }
  static inline const ::std::string& NetEndType_Name(NetEndType value) {
    return CaffeGenSeed_NetEndType_Name(value);
  }
  static inline bool NetEndType_Parse(const ::std::string& name,
      NetEndType* value) {
    return CaffeGenSeed_NetEndType_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // required string test_list_file_name = 1;
  inline bool has_test_list_file_name() const;
  inline void clear_test_list_file_name();
  static const int kTestListFileNameFieldNumber = 1;
  inline const ::std::string& test_list_file_name() const;
  inline void set_test_list_file_name(const ::std::string& value);
  inline void set_test_list_file_name(const char* value);
  inline void set_test_list_file_name(const char* value, size_t size);
  inline ::std::string* mutable_test_list_file_name();
  inline ::std::string* release_test_list_file_name();
  inline void set_allocated_test_list_file_name(::std::string* test_list_file_name);

  // required string train_list_file_name = 2;
  inline bool has_train_list_file_name() const;
  inline void clear_train_list_file_name();
  static const int kTrainListFileNameFieldNumber = 2;
  inline const ::std::string& train_list_file_name() const;
  inline void set_train_list_file_name(const ::std::string& value);
  inline void set_train_list_file_name(const char* value);
  inline void set_train_list_file_name(const char* value, size_t size);
  inline ::std::string* mutable_train_list_file_name();
  inline ::std::string* release_train_list_file_name();
  inline void set_allocated_train_list_file_name(::std::string* train_list_file_name);

  // required int32 num_test_cases = 3;
  inline bool has_num_test_cases() const;
  inline void clear_num_test_cases();
  static const int kNumTestCasesFieldNumber = 3;
  inline ::google::protobuf::int32 num_test_cases() const;
  inline void set_num_test_cases(::google::protobuf::int32 value);

  // required .CaffeGenSeed.NetEndType net_end_type = 4;
  inline bool has_net_end_type() const;
  inline void clear_net_end_type();
  static const int kNetEndTypeFieldNumber = 4;
  inline ::CaffeGenSeed_NetEndType net_end_type() const;
  inline void set_net_end_type(::CaffeGenSeed_NetEndType value);

  // optional int32 num_output_nodes = 5;
  inline bool has_num_output_nodes() const;
  inline void clear_num_output_nodes();
  static const int kNumOutputNodesFieldNumber = 5;
  inline ::google::protobuf::int32 num_output_nodes() const;
  inline void set_num_output_nodes(::google::protobuf::int32 value);

  // required string proto_file_name = 6;
  inline bool has_proto_file_name() const;
  inline void clear_proto_file_name();
  static const int kProtoFileNameFieldNumber = 6;
  inline const ::std::string& proto_file_name() const;
  inline void set_proto_file_name(const ::std::string& value);
  inline void set_proto_file_name(const char* value);
  inline void set_proto_file_name(const char* value, size_t size);
  inline ::std::string* mutable_proto_file_name();
  inline ::std::string* release_proto_file_name();
  inline void set_allocated_proto_file_name(::std::string* proto_file_name);

  // required string model_file_name = 7;
  inline bool has_model_file_name() const;
  inline void clear_model_file_name();
  static const int kModelFileNameFieldNumber = 7;
  inline const ::std::string& model_file_name() const;
  inline void set_model_file_name(const ::std::string& value);
  inline void set_model_file_name(const char* value);
  inline void set_model_file_name(const char* value, size_t size);
  inline ::std::string* mutable_model_file_name();
  inline ::std::string* release_model_file_name();
  inline void set_allocated_model_file_name(::std::string* model_file_name);

  // optional int32 num_accuracy_candidates = 8;
  inline bool has_num_accuracy_candidates() const;
  inline void clear_num_accuracy_candidates();
  static const int kNumAccuracyCandidatesFieldNumber = 8;
  inline ::google::protobuf::int32 num_accuracy_candidates() const;
  inline void set_num_accuracy_candidates(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:CaffeGenSeed)
 private:
  inline void set_has_test_list_file_name();
  inline void clear_has_test_list_file_name();
  inline void set_has_train_list_file_name();
  inline void clear_has_train_list_file_name();
  inline void set_has_num_test_cases();
  inline void clear_has_num_test_cases();
  inline void set_has_net_end_type();
  inline void clear_has_net_end_type();
  inline void set_has_num_output_nodes();
  inline void clear_has_num_output_nodes();
  inline void set_has_proto_file_name();
  inline void clear_has_proto_file_name();
  inline void set_has_model_file_name();
  inline void clear_has_model_file_name();
  inline void set_has_num_accuracy_candidates();
  inline void clear_has_num_accuracy_candidates();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* test_list_file_name_;
  ::std::string* train_list_file_name_;
  ::google::protobuf::int32 num_test_cases_;
  int net_end_type_;
  ::std::string* proto_file_name_;
  ::google::protobuf::int32 num_output_nodes_;
  ::google::protobuf::int32 num_accuracy_candidates_;
  ::std::string* model_file_name_;
  friend void  protobuf_AddDesc_GenSeed_2eproto();
  friend void protobuf_AssignDesc_GenSeed_2eproto();
  friend void protobuf_ShutdownFile_GenSeed_2eproto();

  void InitAsDefaultInstance();
  static CaffeGenSeed* default_instance_;
};
// ===================================================================


// ===================================================================

// CaffeGenSeed

// required string test_list_file_name = 1;
inline bool CaffeGenSeed::has_test_list_file_name() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CaffeGenSeed::set_has_test_list_file_name() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CaffeGenSeed::clear_has_test_list_file_name() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CaffeGenSeed::clear_test_list_file_name() {
  if (test_list_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    test_list_file_name_->clear();
  }
  clear_has_test_list_file_name();
}
inline const ::std::string& CaffeGenSeed::test_list_file_name() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.test_list_file_name)
  return *test_list_file_name_;
}
inline void CaffeGenSeed::set_test_list_file_name(const ::std::string& value) {
  set_has_test_list_file_name();
  if (test_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    test_list_file_name_ = new ::std::string;
  }
  test_list_file_name_->assign(value);
  // @@protoc_insertion_point(field_set:CaffeGenSeed.test_list_file_name)
}
inline void CaffeGenSeed::set_test_list_file_name(const char* value) {
  set_has_test_list_file_name();
  if (test_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    test_list_file_name_ = new ::std::string;
  }
  test_list_file_name_->assign(value);
  // @@protoc_insertion_point(field_set_char:CaffeGenSeed.test_list_file_name)
}
inline void CaffeGenSeed::set_test_list_file_name(const char* value, size_t size) {
  set_has_test_list_file_name();
  if (test_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    test_list_file_name_ = new ::std::string;
  }
  test_list_file_name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CaffeGenSeed.test_list_file_name)
}
inline ::std::string* CaffeGenSeed::mutable_test_list_file_name() {
  set_has_test_list_file_name();
  if (test_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    test_list_file_name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:CaffeGenSeed.test_list_file_name)
  return test_list_file_name_;
}
inline ::std::string* CaffeGenSeed::release_test_list_file_name() {
  clear_has_test_list_file_name();
  if (test_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = test_list_file_name_;
    test_list_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void CaffeGenSeed::set_allocated_test_list_file_name(::std::string* test_list_file_name) {
  if (test_list_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete test_list_file_name_;
  }
  if (test_list_file_name) {
    set_has_test_list_file_name();
    test_list_file_name_ = test_list_file_name;
  } else {
    clear_has_test_list_file_name();
    test_list_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:CaffeGenSeed.test_list_file_name)
}

// required string train_list_file_name = 2;
inline bool CaffeGenSeed::has_train_list_file_name() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CaffeGenSeed::set_has_train_list_file_name() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CaffeGenSeed::clear_has_train_list_file_name() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CaffeGenSeed::clear_train_list_file_name() {
  if (train_list_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    train_list_file_name_->clear();
  }
  clear_has_train_list_file_name();
}
inline const ::std::string& CaffeGenSeed::train_list_file_name() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.train_list_file_name)
  return *train_list_file_name_;
}
inline void CaffeGenSeed::set_train_list_file_name(const ::std::string& value) {
  set_has_train_list_file_name();
  if (train_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    train_list_file_name_ = new ::std::string;
  }
  train_list_file_name_->assign(value);
  // @@protoc_insertion_point(field_set:CaffeGenSeed.train_list_file_name)
}
inline void CaffeGenSeed::set_train_list_file_name(const char* value) {
  set_has_train_list_file_name();
  if (train_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    train_list_file_name_ = new ::std::string;
  }
  train_list_file_name_->assign(value);
  // @@protoc_insertion_point(field_set_char:CaffeGenSeed.train_list_file_name)
}
inline void CaffeGenSeed::set_train_list_file_name(const char* value, size_t size) {
  set_has_train_list_file_name();
  if (train_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    train_list_file_name_ = new ::std::string;
  }
  train_list_file_name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CaffeGenSeed.train_list_file_name)
}
inline ::std::string* CaffeGenSeed::mutable_train_list_file_name() {
  set_has_train_list_file_name();
  if (train_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    train_list_file_name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:CaffeGenSeed.train_list_file_name)
  return train_list_file_name_;
}
inline ::std::string* CaffeGenSeed::release_train_list_file_name() {
  clear_has_train_list_file_name();
  if (train_list_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = train_list_file_name_;
    train_list_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void CaffeGenSeed::set_allocated_train_list_file_name(::std::string* train_list_file_name) {
  if (train_list_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete train_list_file_name_;
  }
  if (train_list_file_name) {
    set_has_train_list_file_name();
    train_list_file_name_ = train_list_file_name;
  } else {
    clear_has_train_list_file_name();
    train_list_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:CaffeGenSeed.train_list_file_name)
}

// required int32 num_test_cases = 3;
inline bool CaffeGenSeed::has_num_test_cases() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void CaffeGenSeed::set_has_num_test_cases() {
  _has_bits_[0] |= 0x00000004u;
}
inline void CaffeGenSeed::clear_has_num_test_cases() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void CaffeGenSeed::clear_num_test_cases() {
  num_test_cases_ = 0;
  clear_has_num_test_cases();
}
inline ::google::protobuf::int32 CaffeGenSeed::num_test_cases() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.num_test_cases)
  return num_test_cases_;
}
inline void CaffeGenSeed::set_num_test_cases(::google::protobuf::int32 value) {
  set_has_num_test_cases();
  num_test_cases_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenSeed.num_test_cases)
}

// required .CaffeGenSeed.NetEndType net_end_type = 4;
inline bool CaffeGenSeed::has_net_end_type() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void CaffeGenSeed::set_has_net_end_type() {
  _has_bits_[0] |= 0x00000008u;
}
inline void CaffeGenSeed::clear_has_net_end_type() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void CaffeGenSeed::clear_net_end_type() {
  net_end_type_ = 1;
  clear_has_net_end_type();
}
inline ::CaffeGenSeed_NetEndType CaffeGenSeed::net_end_type() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.net_end_type)
  return static_cast< ::CaffeGenSeed_NetEndType >(net_end_type_);
}
inline void CaffeGenSeed::set_net_end_type(::CaffeGenSeed_NetEndType value) {
  assert(::CaffeGenSeed_NetEndType_IsValid(value));
  set_has_net_end_type();
  net_end_type_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenSeed.net_end_type)
}

// optional int32 num_output_nodes = 5;
inline bool CaffeGenSeed::has_num_output_nodes() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void CaffeGenSeed::set_has_num_output_nodes() {
  _has_bits_[0] |= 0x00000010u;
}
inline void CaffeGenSeed::clear_has_num_output_nodes() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void CaffeGenSeed::clear_num_output_nodes() {
  num_output_nodes_ = 0;
  clear_has_num_output_nodes();
}
inline ::google::protobuf::int32 CaffeGenSeed::num_output_nodes() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.num_output_nodes)
  return num_output_nodes_;
}
inline void CaffeGenSeed::set_num_output_nodes(::google::protobuf::int32 value) {
  set_has_num_output_nodes();
  num_output_nodes_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenSeed.num_output_nodes)
}

// required string proto_file_name = 6;
inline bool CaffeGenSeed::has_proto_file_name() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void CaffeGenSeed::set_has_proto_file_name() {
  _has_bits_[0] |= 0x00000020u;
}
inline void CaffeGenSeed::clear_has_proto_file_name() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void CaffeGenSeed::clear_proto_file_name() {
  if (proto_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    proto_file_name_->clear();
  }
  clear_has_proto_file_name();
}
inline const ::std::string& CaffeGenSeed::proto_file_name() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.proto_file_name)
  return *proto_file_name_;
}
inline void CaffeGenSeed::set_proto_file_name(const ::std::string& value) {
  set_has_proto_file_name();
  if (proto_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    proto_file_name_ = new ::std::string;
  }
  proto_file_name_->assign(value);
  // @@protoc_insertion_point(field_set:CaffeGenSeed.proto_file_name)
}
inline void CaffeGenSeed::set_proto_file_name(const char* value) {
  set_has_proto_file_name();
  if (proto_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    proto_file_name_ = new ::std::string;
  }
  proto_file_name_->assign(value);
  // @@protoc_insertion_point(field_set_char:CaffeGenSeed.proto_file_name)
}
inline void CaffeGenSeed::set_proto_file_name(const char* value, size_t size) {
  set_has_proto_file_name();
  if (proto_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    proto_file_name_ = new ::std::string;
  }
  proto_file_name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CaffeGenSeed.proto_file_name)
}
inline ::std::string* CaffeGenSeed::mutable_proto_file_name() {
  set_has_proto_file_name();
  if (proto_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    proto_file_name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:CaffeGenSeed.proto_file_name)
  return proto_file_name_;
}
inline ::std::string* CaffeGenSeed::release_proto_file_name() {
  clear_has_proto_file_name();
  if (proto_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = proto_file_name_;
    proto_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void CaffeGenSeed::set_allocated_proto_file_name(::std::string* proto_file_name) {
  if (proto_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete proto_file_name_;
  }
  if (proto_file_name) {
    set_has_proto_file_name();
    proto_file_name_ = proto_file_name;
  } else {
    clear_has_proto_file_name();
    proto_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:CaffeGenSeed.proto_file_name)
}

// required string model_file_name = 7;
inline bool CaffeGenSeed::has_model_file_name() const {
  return (_has_bits_[0] & 0x00000040u) != 0;
}
inline void CaffeGenSeed::set_has_model_file_name() {
  _has_bits_[0] |= 0x00000040u;
}
inline void CaffeGenSeed::clear_has_model_file_name() {
  _has_bits_[0] &= ~0x00000040u;
}
inline void CaffeGenSeed::clear_model_file_name() {
  if (model_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    model_file_name_->clear();
  }
  clear_has_model_file_name();
}
inline const ::std::string& CaffeGenSeed::model_file_name() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.model_file_name)
  return *model_file_name_;
}
inline void CaffeGenSeed::set_model_file_name(const ::std::string& value) {
  set_has_model_file_name();
  if (model_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    model_file_name_ = new ::std::string;
  }
  model_file_name_->assign(value);
  // @@protoc_insertion_point(field_set:CaffeGenSeed.model_file_name)
}
inline void CaffeGenSeed::set_model_file_name(const char* value) {
  set_has_model_file_name();
  if (model_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    model_file_name_ = new ::std::string;
  }
  model_file_name_->assign(value);
  // @@protoc_insertion_point(field_set_char:CaffeGenSeed.model_file_name)
}
inline void CaffeGenSeed::set_model_file_name(const char* value, size_t size) {
  set_has_model_file_name();
  if (model_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    model_file_name_ = new ::std::string;
  }
  model_file_name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CaffeGenSeed.model_file_name)
}
inline ::std::string* CaffeGenSeed::mutable_model_file_name() {
  set_has_model_file_name();
  if (model_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    model_file_name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:CaffeGenSeed.model_file_name)
  return model_file_name_;
}
inline ::std::string* CaffeGenSeed::release_model_file_name() {
  clear_has_model_file_name();
  if (model_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = model_file_name_;
    model_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void CaffeGenSeed::set_allocated_model_file_name(::std::string* model_file_name) {
  if (model_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete model_file_name_;
  }
  if (model_file_name) {
    set_has_model_file_name();
    model_file_name_ = model_file_name;
  } else {
    clear_has_model_file_name();
    model_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:CaffeGenSeed.model_file_name)
}

// optional int32 num_accuracy_candidates = 8;
inline bool CaffeGenSeed::has_num_accuracy_candidates() const {
  return (_has_bits_[0] & 0x00000080u) != 0;
}
inline void CaffeGenSeed::set_has_num_accuracy_candidates() {
  _has_bits_[0] |= 0x00000080u;
}
inline void CaffeGenSeed::clear_has_num_accuracy_candidates() {
  _has_bits_[0] &= ~0x00000080u;
}
inline void CaffeGenSeed::clear_num_accuracy_candidates() {
  num_accuracy_candidates_ = 0;
  clear_has_num_accuracy_candidates();
}
inline ::google::protobuf::int32 CaffeGenSeed::num_accuracy_candidates() const {
  // @@protoc_insertion_point(field_get:CaffeGenSeed.num_accuracy_candidates)
  return num_accuracy_candidates_;
}
inline void CaffeGenSeed::set_num_accuracy_candidates(::google::protobuf::int32 value) {
  set_has_num_accuracy_candidates();
  num_accuracy_candidates_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenSeed.num_accuracy_candidates)
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::CaffeGenSeed_NetEndType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::CaffeGenSeed_NetEndType>() {
  return ::CaffeGenSeed_NetEndType_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_GenSeed_2eproto__INCLUDED
