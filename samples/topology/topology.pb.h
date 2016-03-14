// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: topology.proto

#ifndef PROTOBUF_topology_2eproto__INCLUDED
#define PROTOBUF_topology_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
#include "gtpo.pb.h"
// @@protoc_insertion_point(includes)

namespace qan {
namespace pb {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_topology_2eproto();
void protobuf_AssignDesc_topology_2eproto();
void protobuf_ShutdownFile_topology_2eproto();

class QanImgNode;

// ===================================================================

class QanImgNode : public ::google::protobuf::Message {
 public:
  QanImgNode();
  virtual ~QanImgNode();

  QanImgNode(const QanImgNode& from);

  inline QanImgNode& operator=(const QanImgNode& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const QanImgNode& default_instance();

  void Swap(QanImgNode* other);

  // implements Message ----------------------------------------------

  inline QanImgNode* New() const { return New(NULL); }

  QanImgNode* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const QanImgNode& from);
  void MergeFrom(const QanImgNode& from);
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
  void InternalSwap(QanImgNode* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .gtpo.pb.GTpoNode base = 1;
  bool has_base() const;
  void clear_base();
  static const int kBaseFieldNumber = 1;
  const ::gtpo::pb::GTpoNode& base() const;
  ::gtpo::pb::GTpoNode* mutable_base();
  ::gtpo::pb::GTpoNode* release_base();
  void set_allocated_base(::gtpo::pb::GTpoNode* base);

  // optional int32 img_data_size = 4;
  void clear_img_data_size();
  static const int kImgDataSizeFieldNumber = 4;
  ::google::protobuf::int32 img_data_size() const;
  void set_img_data_size(::google::protobuf::int32 value);

  // optional bytes img_data = 5;
  void clear_img_data();
  static const int kImgDataFieldNumber = 5;
  const ::std::string& img_data() const;
  void set_img_data(const ::std::string& value);
  void set_img_data(const char* value);
  void set_img_data(const void* value, size_t size);
  ::std::string* mutable_img_data();
  ::std::string* release_img_data();
  void set_allocated_img_data(::std::string* img_data);

  // @@protoc_insertion_point(class_scope:qan.pb.QanImgNode)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::gtpo::pb::GTpoNode* base_;
  ::google::protobuf::internal::ArenaStringPtr img_data_;
  ::google::protobuf::int32 img_data_size_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_topology_2eproto();
  friend void protobuf_AssignDesc_topology_2eproto();
  friend void protobuf_ShutdownFile_topology_2eproto();

  void InitAsDefaultInstance();
  static QanImgNode* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// QanImgNode

// optional .gtpo.pb.GTpoNode base = 1;
inline bool QanImgNode::has_base() const {
  return !_is_default_instance_ && base_ != NULL;
}
inline void QanImgNode::clear_base() {
  if (GetArenaNoVirtual() == NULL && base_ != NULL) delete base_;
  base_ = NULL;
}
inline const ::gtpo::pb::GTpoNode& QanImgNode::base() const {
  // @@protoc_insertion_point(field_get:qan.pb.QanImgNode.base)
  return base_ != NULL ? *base_ : *default_instance_->base_;
}
inline ::gtpo::pb::GTpoNode* QanImgNode::mutable_base() {
  
  if (base_ == NULL) {
    base_ = new ::gtpo::pb::GTpoNode;
  }
  // @@protoc_insertion_point(field_mutable:qan.pb.QanImgNode.base)
  return base_;
}
inline ::gtpo::pb::GTpoNode* QanImgNode::release_base() {
  
  ::gtpo::pb::GTpoNode* temp = base_;
  base_ = NULL;
  return temp;
}
inline void QanImgNode::set_allocated_base(::gtpo::pb::GTpoNode* base) {
  delete base_;
  base_ = base;
  if (base) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:qan.pb.QanImgNode.base)
}

// optional int32 img_data_size = 4;
inline void QanImgNode::clear_img_data_size() {
  img_data_size_ = 0;
}
inline ::google::protobuf::int32 QanImgNode::img_data_size() const {
  // @@protoc_insertion_point(field_get:qan.pb.QanImgNode.img_data_size)
  return img_data_size_;
}
inline void QanImgNode::set_img_data_size(::google::protobuf::int32 value) {
  
  img_data_size_ = value;
  // @@protoc_insertion_point(field_set:qan.pb.QanImgNode.img_data_size)
}

// optional bytes img_data = 5;
inline void QanImgNode::clear_img_data() {
  img_data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& QanImgNode::img_data() const {
  // @@protoc_insertion_point(field_get:qan.pb.QanImgNode.img_data)
  return img_data_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void QanImgNode::set_img_data(const ::std::string& value) {
  
  img_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:qan.pb.QanImgNode.img_data)
}
inline void QanImgNode::set_img_data(const char* value) {
  
  img_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:qan.pb.QanImgNode.img_data)
}
inline void QanImgNode::set_img_data(const void* value, size_t size) {
  
  img_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:qan.pb.QanImgNode.img_data)
}
inline ::std::string* QanImgNode::mutable_img_data() {
  
  // @@protoc_insertion_point(field_mutable:qan.pb.QanImgNode.img_data)
  return img_data_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* QanImgNode::release_img_data() {
  
  return img_data_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void QanImgNode::set_allocated_img_data(::std::string* img_data) {
  if (img_data != NULL) {
    
  } else {
    
  }
  img_data_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), img_data);
  // @@protoc_insertion_point(field_set_allocated:qan.pb.QanImgNode.img_data)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace pb
}  // namespace qan

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_topology_2eproto__INCLUDED