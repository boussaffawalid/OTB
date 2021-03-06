// Copyright 2008, Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This file contains the declatation of the EntityMapper class and the
// CreateExpandedEntities function.

#ifndef KML_ENGINE_ENTITY_MAPPER_H__
#define KML_ENGINE_ENTITY_MAPPER_H__

#include "kml/dom.h"
#include "kml/base/string_util.h"
#include "kml/engine/engine_types.h"
#include "kml/engine/kml_file.h"

namespace kmlengine {

// The EntityMapper walks through a given FeaturePtr in a given KmlFile
// and builds a map of any replacable entities and their replacement text.
// For an overview of how this is used within KML, see:
// http://code.google.com/apis/kml/documentation/extendeddata.html
// Usage:
// kmlbase::StringMap your_entity_map;
// EntityMapper entity_mapper(kml_file, &your_entity_map);
// entity_mapper.GetEntityFields(your_feature_ptr);
class EntityMapper {
 public:
  // Instantiate the class with a reference to a KmlFile object.
  // It is the caller's responsibility to ensure that the pointer to the
  // StringMap instance is not NULL.
  EntityMapper(const KmlFilePtr& kml_file, kmlbase::StringMap* string_map);
  ~EntityMapper();

  // Fills the given StringMap with a mapping of all replaceable entities
  // in the given feature to their replacment text. The StringMap is not
  // modified in any way before being written into. It is the caller's
  // responsibilty to ensure that the FeaturePtr exists within the KmlFile
  // from which the class was instantiated.
  void GetEntityFields(const kmldom::FeaturePtr& feature);

 private:
  void GatherObjectFields(const kmldom::FeaturePtr& feature);
  void GatherFeatureFields(const kmldom::FeaturePtr& feature);
  void GatherExtendedDataFields(const kmldom::FeaturePtr& feature);
  void GatherDataFields(const kmldom::DataPtr& feature);
  void GatherSchemaDataFields(const kmldom::SchemaDataPtr& feature);
  void GatherSimpleFieldFields(const kmldom::SimpleFieldPtr& simplefield,
                               const kmldom::SchemaPtr& schema);
  void GatherSimpleDataFields(const kmldom::SimpleDataPtr& simpledata);
  const KmlFilePtr kml_file_;
  kmlbase::StringMap* entity_map_;
  string schemadata_prefix_;
};

// Walks through the given string, replacing all keys in StringMap
// with the corresponding key value. The key strings are wrapped with the
// $[xxx] entity format before searching the string. Returns a new string with
// the replaced entities. The entity_map is typically built with the
// EntityMapper class declared in this file.
string CreateExpandedEntities(const string & in,
                                   const kmlbase::StringMap& entity_map);

}  // end namespace kmlengine

#endif  // KML_ENGINE_ENTITY_MAPPER_H__
