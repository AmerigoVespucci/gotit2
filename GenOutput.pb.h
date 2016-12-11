// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: GenOutput.proto

#ifndef PROTOBUF_GenOutput_2eproto__INCLUDED
#define PROTOBUF_GenOutput_2eproto__INCLUDED

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
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_GenOutput_2eproto();
void protobuf_AssignDesc_GenOutput_2eproto();
void protobuf_ShutdownFile_GenOutput_2eproto();

class CaffeGenOutput;

// ===================================================================

class CaffeGenOutput : public ::google::protobuf::Message {
 public:
  CaffeGenOutput();
  virtual ~CaffeGenOutput();

  CaffeGenOutput(const CaffeGenOutput& from);

  inline CaffeGenOutput& operator=(const CaffeGenOutput& from) {
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
  static const CaffeGenOutput& default_instance();

  void Swap(CaffeGenOutput* other);

  // implements Message ----------------------------------------------

  CaffeGenOutput* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CaffeGenOutput& from);
  void MergeFrom(const CaffeGenOutput& from);
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

  // accessors -------------------------------------------------------

  // required int32 num_input_nodes = 1;
  inline bool has_num_input_nodes() const;
  inline void clear_num_input_nodes();
  static const int kNumInputNodesFieldNumber = 1;
  inline ::google::protobuf::int32 num_input_nodes() const;
  inline void set_num_input_nodes(::google::protobuf::int32 value);

  // required int32 num_output_nodes = 2;
  inline bool has_num_output_nodes() const;
  inline void clear_num_output_nodes();
  static const int kNumOutputNodesFieldNumber = 2;
  inline ::google::protobuf::int32 num_output_nodes() const;
  inline void set_num_output_nodes(::google::protobuf::int32 value);

  // required int32 input_layer_idx = 3;
  inline bool has_input_layer_idx() const;
  inline void clear_input_layer_idx();
  static const int kInputLayerIdxFieldNumber = 3;
  inline ::google::protobuf::int32 input_layer_idx() const;
  inline void set_input_layer_idx(::google::protobuf::int32 value);

  // required int32 input_layer_top_idx = 4;
  inline bool has_input_layer_top_idx() const;
  inline void clear_input_layer_top_idx();
  static const int kInputLayerTopIdxFieldNumber = 4;
  inline ::google::protobuf::int32 input_layer_top_idx() const;
  inline void set_input_layer_top_idx(::google::protobuf::int32 value);

  // required int32 output_layer_idx = 5;
  inline bool has_output_layer_idx() const;
  inline void clear_output_layer_idx();
  static const int kOutputLayerIdxFieldNumber = 5;
  inline ::google::protobuf::int32 output_layer_idx() const;
  inline void set_output_layer_idx(::google::protobuf::int32 value);

  // required int32 output_layer_top_idx = 6;
  inline bool has_output_layer_top_idx() const;
  inline void clear_output_layer_top_idx();
  static const int kOutputLayerTopIdxFieldNumber = 6;
  inline ::google::protobuf::int32 output_layer_top_idx() const;
  inline void set_output_layer_top_idx(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:CaffeGenOutput)
 private:
  inline void set_has_num_input_nodes();
  inline void clear_has_num_input_nodes();
  inline void set_has_num_output_nodes();
  inline void clear_has_num_output_nodes();
  inline void set_has_input_layer_idx();
  inline void clear_has_input_layer_idx();
  inline void set_has_input_layer_top_idx();
  inline void clear_has_input_layer_top_idx();
  inline void set_has_output_layer_idx();
  inline void clear_has_output_layer_idx();
  inline void set_has_output_layer_top_idx();
  inline void clear_has_output_layer_top_idx();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::int32 num_input_nodes_;
  ::google::protobuf::int32 num_output_nodes_;
  ::google::protobuf::int32 input_layer_idx_;
  ::google::protobuf::int32 input_layer_top_idx_;
  ::google::protobuf::int32 output_layer_idx_;
  ::google::protobuf::int32 output_layer_top_idx_;
  friend void  protobuf_AddDesc_GenOutput_2eproto();
  friend void protobuf_AssignDesc_GenOutput_2eproto();
  friend void protobuf_ShutdownFile_GenOutput_2eproto();

  void InitAsDefaultInstance();
  static CaffeGenOutput* default_instance_;
};
// ===================================================================


// ===================================================================

// CaffeGenOutput

// required int32 num_input_nodes = 1;
inline bool CaffeGenOutput::has_num_input_nodes() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CaffeGenOutput::set_has_num_input_nodes() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CaffeGenOutput::clear_has_num_input_nodes() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CaffeGenOutput::clear_num_input_nodes() {
  num_input_nodes_ = 0;
  clear_has_num_input_nodes();
}
inline ::google::protobuf::int32 CaffeGenOutput::num_input_nodes() const {
  // @@protoc_insertion_point(field_get:CaffeGenOutput.num_input_nodes)
  return num_input_nodes_;
}
inline void CaffeGenOutput::set_num_input_nodes(::google::protobuf::int32 value) {
  set_has_num_input_nodes();
  num_input_nodes_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenOutput.num_input_nodes)
}

// required int32 num_output_nodes = 2;
inline bool CaffeGenOutput::has_num_output_nodes() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CaffeGenOutput::set_has_num_output_nodes() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CaffeGenOutput::clear_has_num_output_nodes() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CaffeGenOutput::clear_num_output_nodes() {
  num_output_nodes_ = 0;
  clear_has_num_output_nodes();
}
inline ::google::protobuf::int32 CaffeGenOutput::num_output_nodes() const {
  // @@protoc_insertion_point(field_get:CaffeGenOutput.num_output_nodes)
  return num_output_nodes_;
}
inline void CaffeGenOutput::set_num_output_nodes(::google::protobuf::int32 value) {
  set_has_num_output_nodes();
  num_output_nodes_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenOutput.num_output_nodes)
}

// required int32 input_layer_idx = 3;
inline bool CaffeGenOutput::has_input_layer_idx() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void CaffeGenOutput::set_has_input_layer_idx() {
  _has_bits_[0] |= 0x00000004u;
}
inline void CaffeGenOutput::clear_has_input_layer_idx() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void CaffeGenOutput::clear_input_layer_idx() {
  input_layer_idx_ = 0;
  clear_has_input_layer_idx();
}
inline ::google::protobuf::int32 CaffeGenOutput::input_layer_idx() const {
  // @@protoc_insertion_point(field_get:CaffeGenOutput.input_layer_idx)
  return input_layer_idx_;
}
inline void CaffeGenOutput::set_input_layer_idx(::google::protobuf::int32 value) {
  set_has_input_layer_idx();
  input_layer_idx_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenOutput.input_layer_idx)
}

// required int32 input_layer_top_idx = 4;
inline bool CaffeGenOutput::has_input_layer_top_idx() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void CaffeGenOutput::set_has_input_layer_top_idx() {
  _has_bits_[0] |= 0x00000008u;
}
inline void CaffeGenOutput::clear_has_input_layer_top_idx() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void CaffeGenOutput::clear_input_layer_top_idx() {
  input_layer_top_idx_ = 0;
  clear_has_input_layer_top_idx();
}
inline ::google::protobuf::int32 CaffeGenOutput::input_layer_top_idx() const {
  // @@protoc_insertion_point(field_get:CaffeGenOutput.input_layer_top_idx)
  return input_layer_top_idx_;
}
inline void CaffeGenOutput::set_input_layer_top_idx(::google::protobuf::int32 value) {
  set_has_input_layer_top_idx();
  input_layer_top_idx_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenOutput.input_layer_top_idx)
}

// required int32 output_layer_idx = 5;
inline bool CaffeGenOutput::has_output_layer_idx() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void CaffeGenOutput::set_has_output_layer_idx() {
  _has_bits_[0] |= 0x00000010u;
}
inline void CaffeGenOutput::clear_has_output_layer_idx() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void CaffeGenOutput::clear_output_layer_idx() {
  output_layer_idx_ = 0;
  clear_has_output_layer_idx();
}
inline ::google::protobuf::int32 CaffeGenOutput::output_layer_idx() const {
  // @@protoc_insertion_point(field_get:CaffeGenOutput.output_layer_idx)
  return output_layer_idx_;
}
inline void CaffeGenOutput::set_output_layer_idx(::google::protobuf::int32 value) {
  set_has_output_layer_idx();
  output_layer_idx_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenOutput.output_layer_idx)
}

// required int32 output_layer_top_idx = 6;
inline bool CaffeGenOutput::has_output_layer_top_idx() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void CaffeGenOutput::set_has_output_layer_top_idx() {
  _has_bits_[0] |= 0x00000020u;
}
inline void CaffeGenOutput::clear_has_output_layer_top_idx() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void CaffeGenOutput::clear_output_layer_top_idx() {
  output_layer_top_idx_ = 0;
  clear_has_output_layer_top_idx();
}
inline ::google::protobuf::int32 CaffeGenOutput::output_layer_top_idx() const {
  // @@protoc_insertion_point(field_get:CaffeGenOutput.output_layer_top_idx)
  return output_layer_top_idx_;
}
inline void CaffeGenOutput::set_output_layer_top_idx(::google::protobuf::int32 value) {
  set_has_output_layer_top_idx();
  output_layer_top_idx_ = value;
  // @@protoc_insertion_point(field_set:CaffeGenOutput.output_layer_top_idx)
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_GenOutput_2eproto__INCLUDED