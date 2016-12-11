// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: GenTbl.proto

#ifndef PROTOBUF_GenTbl_2eproto__INCLUDED
#define PROTOBUF_GenTbl_2eproto__INCLUDED

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
void  protobuf_AddDesc_GenTbl_2eproto();
void protobuf_AssignDesc_GenTbl_2eproto();
void protobuf_ShutdownFile_GenTbl_2eproto();

class CaffeGenTbl;

enum CaffeGenTbl_TblType {
  CaffeGenTbl_TblType_TBL_TYPE_WORD = 1,
  CaffeGenTbl_TblType_TBL_TYPE_POS = 2,
  CaffeGenTbl_TblType_TBL_TYPE_DEP = 3
};
bool CaffeGenTbl_TblType_IsValid(int value);
const CaffeGenTbl_TblType CaffeGenTbl_TblType_TblType_MIN = CaffeGenTbl_TblType_TBL_TYPE_WORD;
const CaffeGenTbl_TblType CaffeGenTbl_TblType_TblType_MAX = CaffeGenTbl_TblType_TBL_TYPE_DEP;
const int CaffeGenTbl_TblType_TblType_ARRAYSIZE = CaffeGenTbl_TblType_TblType_MAX + 1;

const ::google::protobuf::EnumDescriptor* CaffeGenTbl_TblType_descriptor();
inline const ::std::string& CaffeGenTbl_TblType_Name(CaffeGenTbl_TblType value) {
  return ::google::protobuf::internal::NameOfEnum(
    CaffeGenTbl_TblType_descriptor(), value);
}
inline bool CaffeGenTbl_TblType_Parse(
    const ::std::string& name, CaffeGenTbl_TblType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<CaffeGenTbl_TblType>(
    CaffeGenTbl_TblType_descriptor(), name, value);
}
// ===================================================================

class CaffeGenTbl : public ::google::protobuf::Message {
 public:
  CaffeGenTbl();
  virtual ~CaffeGenTbl();

  CaffeGenTbl(const CaffeGenTbl& from);

  inline CaffeGenTbl& operator=(const CaffeGenTbl& from) {
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
  static const CaffeGenTbl& default_instance();

  void Swap(CaffeGenTbl* other);

  // implements Message ----------------------------------------------

  CaffeGenTbl* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CaffeGenTbl& from);
  void MergeFrom(const CaffeGenTbl& from);
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

  typedef CaffeGenTbl_TblType TblType;
  static const TblType TBL_TYPE_WORD = CaffeGenTbl_TblType_TBL_TYPE_WORD;
  static const TblType TBL_TYPE_POS = CaffeGenTbl_TblType_TBL_TYPE_POS;
  static const TblType TBL_TYPE_DEP = CaffeGenTbl_TblType_TBL_TYPE_DEP;
  static inline bool TblType_IsValid(int value) {
    return CaffeGenTbl_TblType_IsValid(value);
  }
  static const TblType TblType_MIN =
    CaffeGenTbl_TblType_TblType_MIN;
  static const TblType TblType_MAX =
    CaffeGenTbl_TblType_TblType_MAX;
  static const int TblType_ARRAYSIZE =
    CaffeGenTbl_TblType_TblType_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  TblType_descriptor() {
    return CaffeGenTbl_TblType_descriptor();
  }
  static inline const ::std::string& TblType_Name(TblType value) {
    return CaffeGenTbl_TblType_Name(value);
  }
  static inline bool TblType_Parse(const ::std::string& name,
      TblType* value) {
    return CaffeGenTbl_TblType_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // required string name = 1;
  inline bool has_name() const;
  inline void clear_name();
  static const int kNameFieldNumber = 1;
  inline const ::std::string& name() const;
  inline void set_name(const ::std::string& value);
  inline void set_name(const char* value);
  inline void set_name(const char* value, size_t size);
  inline ::std::string* mutable_name();
  inline ::std::string* release_name();
  inline void set_allocated_name(::std::string* name);

  // required string files_core_dir = 2;
  inline bool has_files_core_dir() const;
  inline void clear_files_core_dir();
  static const int kFilesCoreDirFieldNumber = 2;
  inline const ::std::string& files_core_dir() const;
  inline void set_files_core_dir(const ::std::string& value);
  inline void set_files_core_dir(const char* value);
  inline void set_files_core_dir(const char* value, size_t size);
  inline ::std::string* mutable_files_core_dir();
  inline ::std::string* release_files_core_dir();
  inline void set_allocated_files_core_dir(::std::string* files_core_dir);

  // required .CaffeGenTbl.TblType tbl_type = 3;
  inline bool has_tbl_type() const;
  inline void clear_tbl_type();
  static const int kTblTypeFieldNumber = 3;
  inline ::CaffeGenTbl_TblType tbl_type() const;
  inline void set_tbl_type(::CaffeGenTbl_TblType value);

  // optional string input_vec_file_name = 4;
  inline bool has_input_vec_file_name() const;
  inline void clear_input_vec_file_name();
  static const int kInputVecFileNameFieldNumber = 4;
  inline const ::std::string& input_vec_file_name() const;
  inline void set_input_vec_file_name(const ::std::string& value);
  inline void set_input_vec_file_name(const char* value);
  inline void set_input_vec_file_name(const char* value, size_t size);
  inline ::std::string* mutable_input_vec_file_name();
  inline ::std::string* release_input_vec_file_name();
  inline void set_allocated_input_vec_file_name(::std::string* input_vec_file_name);

  // optional int32 max_words = 5;
  inline bool has_max_words() const;
  inline void clear_max_words();
  static const int kMaxWordsFieldNumber = 5;
  inline ::google::protobuf::int32 max_words() const;
  inline void set_max_words(::google::protobuf::int32 value);

  // optional bool alpha_only = 6;
  inline bool has_alpha_only() const;
  inline void clear_alpha_only();
  static const int kAlphaOnlyFieldNumber = 6;
  inline bool alpha_only() const;
  inline void set_alpha_only(bool value);

  // optional bool one_hot = 7;
  inline bool has_one_hot() const;
  inline void clear_one_hot();
  static const int kOneHotFieldNumber = 7;
  inline bool one_hot() const;
  inline void set_one_hot(bool value);

  // @@protoc_insertion_point(class_scope:CaffeGenTbl)
 private:
  inline void set_has_name();
  inline void clear_has_name();
  inline void set_has_files_core_dir();
  inline void clear_has_files_core_dir();
  inline void set_has_tbl_type();
  inline void clear_has_tbl_type();
  inline void set_has_input_vec_file_name();
  inline void clear_has_input_vec_file_name();
  inline void set_has_max_words();
  inline void clear_has_max_words();
  inline void set_has_alpha_only();
  inline void clear_has_alpha_only();
  inline void set_has_one_hot();
  inline void clear_has_one_hot();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* name_;
  ::std::string* files_core_dir_;
  ::std::string* input_vec_file_name_;
  int tbl_type_;
  ::google::protobuf::int32 max_words_;
  bool alpha_only_;
  bool one_hot_;
  friend void  protobuf_AddDesc_GenTbl_2eproto();
  friend void protobuf_AssignDesc_GenTbl_2eproto();
  friend void protobuf_ShutdownFile_GenTbl_2eproto();

  void InitAsDefaultInstance();
  static CaffeGenTbl* default_instance_;
};
// ===================================================================


// ===================================================================

// CaffeGenTbl

// required string name = 1;
inline bool CaffeGenTbl::has_name() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CaffeGenTbl::set_has_name() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CaffeGenTbl::clear_has_name() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CaffeGenTbl::clear_name() {
  if (name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_->clear();
  }
  clear_has_name();
}
inline const ::std::string& CaffeGenTbl::name() const {
  // @@protoc_insertion_point(field_get:CaffeGenTbl.name)
  return *name_;
}
inline void CaffeGenTbl::set_name(const ::std::string& value) {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  name_->assign(value);
  // @@protoc_insertion_point(field_set:CaffeGenTbl.name)
}
inline void CaffeGenTbl::set_name(const char* value) {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  name_->assign(value);
  // @@protoc_insertion_point(field_set_char:CaffeGenTbl.name)
}
inline void CaffeGenTbl::set_name(const char* value, size_t size) {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CaffeGenTbl.name)
}
inline ::std::string* CaffeGenTbl::mutable_name() {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:CaffeGenTbl.name)
  return name_;
}
inline ::std::string* CaffeGenTbl::release_name() {
  clear_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = name_;
    name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void CaffeGenTbl::set_allocated_name(::std::string* name) {
  if (name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete name_;
  }
  if (name) {
    set_has_name();
    name_ = name;
  } else {
    clear_has_name();
    name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:CaffeGenTbl.name)
}

// required string files_core_dir = 2;
inline bool CaffeGenTbl::has_files_core_dir() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CaffeGenTbl::set_has_files_core_dir() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CaffeGenTbl::clear_has_files_core_dir() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CaffeGenTbl::clear_files_core_dir() {
  if (files_core_dir_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    files_core_dir_->clear();
  }
  clear_has_files_core_dir();
}
inline const ::std::string& CaffeGenTbl::files_core_dir() const {
  // @@protoc_insertion_point(field_get:CaffeGenTbl.files_core_dir)
  return *files_core_dir_;
}
inline void CaffeGenTbl::set_files_core_dir(const ::std::string& value) {
  set_has_files_core_dir();
  if (files_core_dir_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    files_core_dir_ = new ::std::string;
  }
  files_core_dir_->assign(value);
  // @@protoc_insertion_point(field_set:CaffeGenTbl.files_core_dir)
}
inline void CaffeGenTbl::set_files_core_dir(const char* value) {
  set_has_files_core_dir();
  if (files_core_dir_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    files_core_dir_ = new ::std::string;
  }
  files_core_dir_->assign(value);
  // @@protoc_insertion_point(field_set_char:CaffeGenTbl.files_core_dir)
}
inline void CaffeGenTbl::set_files_core_dir(const char* value, size_t size) {
  set_has_files_core_dir();
  if (files_core_dir_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    files_core_dir_ = new ::std::string;
  }
  files_core_dir_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CaffeGenTbl.files_core_dir)
}
inline ::std::string* CaffeGenTbl::mutable_files_core_dir() {
  set_has_files_core_dir();
  if (files_core_dir_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    files_core_dir_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:CaffeGenTbl.files_core_dir)
  return files_core_dir_;
}
inline ::std::string* CaffeGenTbl::release_files_core_dir() {
  clear_has_files_core_dir();
  if (files_core_dir_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = files_core_dir_;
    files_core_dir_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void CaffeGenTbl::set_allocated_files_core_dir(::std::string* files_core_dir) {
  if (files_core_dir_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete files_core_dir_;
  }
  if (files_core_dir) {
    set_has_files_core_dir();
    files_core_dir_ = files_core_dir;
  } else {
    clear_has_files_core_dir();
    files_core_dir_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:CaffeGenTbl.files_core_dir)
}

// required .CaffeGenTbl.TblType tbl_type = 3;
inline bool CaffeGenTbl::has_tbl_type() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void CaffeGenTbl::set_has_tbl_type() {
  _has_bits_[0] |= 0x00000004u;
}
inline void CaffeGenTbl::clear_has_tbl_type() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void CaffeGenTbl::clear_tbl_type() {
  tbl_type_ = 1;
  clear_has_tbl_type();
}
inline ::CaffeGenTbl_TblType CaffeGenTbl::tbl_type() const {
  // @@protoc_insertion_point(field_get:CaffeGenTbl.tbl_type)
  return static_cast< ::CaffeGenTbl_TblType >(tbl_type_);
}
inline void CaffeGenTbl::set_tbl_type(::CaffeGenTbl_TblType value) {
  assert(::CaffeGenTbl_TblType_IsValid(value));
  set_has_tbl_type();
  tbl_type_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenTbl.tbl_type)
}

// optional string input_vec_file_name = 4;
inline bool CaffeGenTbl::has_input_vec_file_name() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void CaffeGenTbl::set_has_input_vec_file_name() {
  _has_bits_[0] |= 0x00000008u;
}
inline void CaffeGenTbl::clear_has_input_vec_file_name() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void CaffeGenTbl::clear_input_vec_file_name() {
  if (input_vec_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    input_vec_file_name_->clear();
  }
  clear_has_input_vec_file_name();
}
inline const ::std::string& CaffeGenTbl::input_vec_file_name() const {
  // @@protoc_insertion_point(field_get:CaffeGenTbl.input_vec_file_name)
  return *input_vec_file_name_;
}
inline void CaffeGenTbl::set_input_vec_file_name(const ::std::string& value) {
  set_has_input_vec_file_name();
  if (input_vec_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    input_vec_file_name_ = new ::std::string;
  }
  input_vec_file_name_->assign(value);
  // @@protoc_insertion_point(field_set:CaffeGenTbl.input_vec_file_name)
}
inline void CaffeGenTbl::set_input_vec_file_name(const char* value) {
  set_has_input_vec_file_name();
  if (input_vec_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    input_vec_file_name_ = new ::std::string;
  }
  input_vec_file_name_->assign(value);
  // @@protoc_insertion_point(field_set_char:CaffeGenTbl.input_vec_file_name)
}
inline void CaffeGenTbl::set_input_vec_file_name(const char* value, size_t size) {
  set_has_input_vec_file_name();
  if (input_vec_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    input_vec_file_name_ = new ::std::string;
  }
  input_vec_file_name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:CaffeGenTbl.input_vec_file_name)
}
inline ::std::string* CaffeGenTbl::mutable_input_vec_file_name() {
  set_has_input_vec_file_name();
  if (input_vec_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    input_vec_file_name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:CaffeGenTbl.input_vec_file_name)
  return input_vec_file_name_;
}
inline ::std::string* CaffeGenTbl::release_input_vec_file_name() {
  clear_has_input_vec_file_name();
  if (input_vec_file_name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = input_vec_file_name_;
    input_vec_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void CaffeGenTbl::set_allocated_input_vec_file_name(::std::string* input_vec_file_name) {
  if (input_vec_file_name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete input_vec_file_name_;
  }
  if (input_vec_file_name) {
    set_has_input_vec_file_name();
    input_vec_file_name_ = input_vec_file_name;
  } else {
    clear_has_input_vec_file_name();
    input_vec_file_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:CaffeGenTbl.input_vec_file_name)
}

// optional int32 max_words = 5;
inline bool CaffeGenTbl::has_max_words() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void CaffeGenTbl::set_has_max_words() {
  _has_bits_[0] |= 0x00000010u;
}
inline void CaffeGenTbl::clear_has_max_words() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void CaffeGenTbl::clear_max_words() {
  max_words_ = 0;
  clear_has_max_words();
}
inline ::google::protobuf::int32 CaffeGenTbl::max_words() const {
  // @@protoc_insertion_point(field_get:CaffeGenTbl.max_words)
  return max_words_;
}
inline void CaffeGenTbl::set_max_words(::google::protobuf::int32 value) {
  set_has_max_words();
  max_words_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenTbl.max_words)
}

// optional bool alpha_only = 6;
inline bool CaffeGenTbl::has_alpha_only() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void CaffeGenTbl::set_has_alpha_only() {
  _has_bits_[0] |= 0x00000020u;
}
inline void CaffeGenTbl::clear_has_alpha_only() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void CaffeGenTbl::clear_alpha_only() {
  alpha_only_ = false;
  clear_has_alpha_only();
}
inline bool CaffeGenTbl::alpha_only() const {
  // @@protoc_insertion_point(field_get:CaffeGenTbl.alpha_only)
  return alpha_only_;
}
inline void CaffeGenTbl::set_alpha_only(bool value) {
  set_has_alpha_only();
  alpha_only_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenTbl.alpha_only)
}

// optional bool one_hot = 7;
inline bool CaffeGenTbl::has_one_hot() const {
  return (_has_bits_[0] & 0x00000040u) != 0;
}
inline void CaffeGenTbl::set_has_one_hot() {
  _has_bits_[0] |= 0x00000040u;
}
inline void CaffeGenTbl::clear_has_one_hot() {
  _has_bits_[0] &= ~0x00000040u;
}
inline void CaffeGenTbl::clear_one_hot() {
  one_hot_ = false;
  clear_has_one_hot();
}
inline bool CaffeGenTbl::one_hot() const {
  // @@protoc_insertion_point(field_get:CaffeGenTbl.one_hot)
  return one_hot_;
}
inline void CaffeGenTbl::set_one_hot(bool value) {
  set_has_one_hot();
  one_hot_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenTbl.one_hot)
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::CaffeGenTbl_TblType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::CaffeGenTbl_TblType>() {
  return ::CaffeGenTbl_TblType_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_GenTbl_2eproto__INCLUDED
