// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: geometry/geometry.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "geometry/geometry.pb.h"

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

namespace principia {
namespace serialization {

namespace {

const ::google::protobuf::Descriptor* R3Element_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  R3Element_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_geometry_2fgeometry_2eproto() {
  protobuf_AddDesc_geometry_2fgeometry_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "geometry/geometry.proto");
  GOOGLE_CHECK(file != NULL);
  R3Element_descriptor_ = file->message_type(0);
  static const int R3Element_offsets_[3] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(R3Element, x_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(R3Element, y_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(R3Element, z_),
  };
  R3Element_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      R3Element_descriptor_,
      R3Element::default_instance_,
      R3Element_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(R3Element, _has_bits_[0]),
      -1,
      -1,
      sizeof(R3Element),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(R3Element, _internal_metadata_),
      -1);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_geometry_2fgeometry_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      R3Element_descriptor_, &R3Element::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_geometry_2fgeometry_2eproto() {
  delete R3Element::default_instance_;
  delete R3Element_reflection_;
}

void protobuf_AddDesc_geometry_2fgeometry_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::principia::serialization::protobuf_AddDesc_quantities_2fquantities_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\027geometry/geometry.proto\022\027principia.ser"
    "ialization\032\033quantities/quantities.proto\""
    "\225\001\n\tR3Element\022,\n\001x\030\001 \002(\0132!.principia.ser"
    "ialization.Quantity\022,\n\001y\030\002 \002(\0132!.princip"
    "ia.serialization.Quantity\022,\n\001z\030\003 \002(\0132!.p"
    "rincipia.serialization.Quantity", 231);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "geometry/geometry.proto", &protobuf_RegisterTypes);
  R3Element::default_instance_ = new R3Element();
  R3Element::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_geometry_2fgeometry_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_geometry_2fgeometry_2eproto {
  StaticDescriptorInitializer_geometry_2fgeometry_2eproto() {
    protobuf_AddDesc_geometry_2fgeometry_2eproto();
  }
} static_descriptor_initializer_geometry_2fgeometry_2eproto_;

namespace {

static void MergeFromFail(int line) GOOGLE_ATTRIBUTE_COLD;
static void MergeFromFail(int line) {
  GOOGLE_CHECK(false) << __FILE__ << ":" << line;
}

}  // namespace


// ===================================================================

#ifndef _MSC_VER
const int R3Element::kXFieldNumber;
const int R3Element::kYFieldNumber;
const int R3Element::kZFieldNumber;
#endif  // !_MSC_VER

R3Element::R3Element()
  : ::google::protobuf::Message() , _internal_metadata_(NULL)  {
  SharedCtor();
  // @@protoc_insertion_point(constructor:principia.serialization.R3Element)
}

void R3Element::InitAsDefaultInstance() {
  x_ = const_cast< ::principia::serialization::Quantity*>(&::principia::serialization::Quantity::default_instance());
  y_ = const_cast< ::principia::serialization::Quantity*>(&::principia::serialization::Quantity::default_instance());
  z_ = const_cast< ::principia::serialization::Quantity*>(&::principia::serialization::Quantity::default_instance());
}

R3Element::R3Element(const R3Element& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:principia.serialization.R3Element)
}

void R3Element::SharedCtor() {
  _cached_size_ = 0;
  x_ = NULL;
  y_ = NULL;
  z_ = NULL;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

R3Element::~R3Element() {
  // @@protoc_insertion_point(destructor:principia.serialization.R3Element)
  SharedDtor();
}

void R3Element::SharedDtor() {
  if (this != default_instance_) {
    delete x_;
    delete y_;
    delete z_;
  }
}

void R3Element::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* R3Element::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return R3Element_descriptor_;
}

const R3Element& R3Element::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_geometry_2fgeometry_2eproto();
  return *default_instance_;
}

R3Element* R3Element::default_instance_ = NULL;

R3Element* R3Element::New(::google::protobuf::Arena* arena) const {
  R3Element* n = new R3Element;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void R3Element::Clear() {
  if (_has_bits_[0 / 32] & 7) {
    if (has_x()) {
      if (x_ != NULL) x_->::principia::serialization::Quantity::Clear();
    }
    if (has_y()) {
      if (y_ != NULL) y_->::principia::serialization::Quantity::Clear();
    }
    if (has_z()) {
      if (z_ != NULL) z_->::principia::serialization::Quantity::Clear();
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  if (_internal_metadata_.have_unknown_fields()) {
    mutable_unknown_fields()->Clear();
  }
}

bool R3Element::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:principia.serialization.R3Element)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .principia.serialization.Quantity x = 1;
      case 1: {
        if (tag == 10) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_x()));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_y;
        break;
      }

      // required .principia.serialization.Quantity y = 2;
      case 2: {
        if (tag == 18) {
         parse_y:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_y()));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(26)) goto parse_z;
        break;
      }

      // required .principia.serialization.Quantity z = 3;
      case 3: {
        if (tag == 26) {
         parse_z:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_z()));
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
  // @@protoc_insertion_point(parse_success:principia.serialization.R3Element)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:principia.serialization.R3Element)
  return false;
#undef DO_
}

void R3Element::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:principia.serialization.R3Element)
  // required .principia.serialization.Quantity x = 1;
  if (has_x()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      1, *this->x_, output);
  }

  // required .principia.serialization.Quantity y = 2;
  if (has_y()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      2, *this->y_, output);
  }

  // required .principia.serialization.Quantity z = 3;
  if (has_z()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      3, *this->z_, output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:principia.serialization.R3Element)
}

::google::protobuf::uint8* R3Element::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:principia.serialization.R3Element)
  // required .principia.serialization.Quantity x = 1;
  if (has_x()) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        1, *this->x_, target);
  }

  // required .principia.serialization.Quantity y = 2;
  if (has_y()) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        2, *this->y_, target);
  }

  // required .principia.serialization.Quantity z = 3;
  if (has_z()) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        3, *this->z_, target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:principia.serialization.R3Element)
  return target;
}

int R3Element::RequiredFieldsByteSizeFallback() const {
  int total_size = 0;

  if (has_x()) {
    // required .principia.serialization.Quantity x = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->x_);
  }

  if (has_y()) {
    // required .principia.serialization.Quantity y = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->y_);
  }

  if (has_z()) {
    // required .principia.serialization.Quantity z = 3;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->z_);
  }

  return total_size;
}
int R3Element::ByteSize() const {
  int total_size = 0;

  if (((_has_bits_[0] & 0x00000007) ^ 0x00000007) == 0) {  // All required fields are present.
    // required .principia.serialization.Quantity x = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->x_);

    // required .principia.serialization.Quantity y = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->y_);

    // required .principia.serialization.Quantity z = 3;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->z_);

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void R3Element::MergeFrom(const ::google::protobuf::Message& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const R3Element* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const R3Element*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void R3Element::MergeFrom(const R3Element& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_x()) {
      mutable_x()->::principia::serialization::Quantity::MergeFrom(from.x());
    }
    if (from.has_y()) {
      mutable_y()->::principia::serialization::Quantity::MergeFrom(from.y());
    }
    if (from.has_z()) {
      mutable_z()->::principia::serialization::Quantity::MergeFrom(from.z());
    }
  }
  if (from._internal_metadata_.have_unknown_fields()) {
    mutable_unknown_fields()->MergeFrom(from.unknown_fields());
  }
}

void R3Element::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void R3Element::CopyFrom(const R3Element& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool R3Element::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;

  if (has_x()) {
    if (!this->x_->IsInitialized()) return false;
  }
  if (has_y()) {
    if (!this->y_->IsInitialized()) return false;
  }
  if (has_z()) {
    if (!this->z_->IsInitialized()) return false;
  }
  return true;
}

void R3Element::Swap(R3Element* other) {
  if (other == this) return;
  InternalSwap(other);
}
void R3Element::InternalSwap(R3Element* other) {
  std::swap(x_, other->x_);
  std::swap(y_, other->y_);
  std::swap(z_, other->z_);
  std::swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata R3Element::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = R3Element_descriptor_;
  metadata.reflection = R3Element_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace serialization
}  // namespace principia

// @@protoc_insertion_point(global_scope)
