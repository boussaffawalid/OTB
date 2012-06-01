/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/*===========================================================================*/
/*===============================[ Includes ]================================*/
/*===========================================================================*/
#include "otbGeometriesToGeometriesFilter.h"
#include "otbGeometriesSet.h"
#include "itkMacro.h"
#include "itkTimeProbe.h"
#include "otbMacro.h"

/*===========================================================================*/
/*==============================[ other stuff ]==============================*/
/*===========================================================================*/

otb::GeometriesToGeometriesFilter::GeometriesToGeometriesFilter()
{
}

/*virtual*/  otb::GeometriesToGeometriesFilter::~GeometriesToGeometriesFilter()
{
}

/*virtual*/ void otb::GeometriesToGeometriesFilter::SetInput(
  const InputGeometriesType * input)
{
  // Process object is not const-correct so the const_cast is required here
  this->itk::ProcessObject::SetNthInput(
    0,
    const_cast<InputGeometriesType *>(input));
}

const otb::GeometriesToGeometriesFilter::InputGeometriesType *
otb::GeometriesToGeometriesFilter::GetInput(void )
{
  return static_cast <InputGeometriesType*>(Superclass::GetInput(0));
}


struct ProcessVisitor : boost::static_visitor<>
{
  ProcessVisitor(otb::GeometriesToGeometriesFilter const& filter)
    : m_filter(filter) {}
  void operator()(otb::ogr::Layer const& source, otb::ogr::Layer & destination) const
    {
    std::cout << "G2GF: Process Visitor: L -> L ("<< source.GetName()<<")...\n";
    m_filter.DoProcessLayer(source, destination);
    }
  void operator()(otb::ogr::DataSource::Pointer source, otb::ogr::DataSource::Pointer destination) const
    {
    std::cout << "G2GF: Process Visitor: DS("<<source->ogr().GetName()<<") -> DS("<<source->ogr().GetName()<<") ...\n";
    for (otb::ogr::DataSource::const_iterator b = source->begin(), e = source->end()
     ; b != e
     ; ++b
    )
      {
      otb::ogr::Layer const& sourceLayer = *b;
      otb::ogr::Layer destLayer = destination->CreateLayer(
        sourceLayer.GetName(), 0, sourceLayer.GetGeomType());
      m_filter.DoProcessLayer(sourceLayer, destLayer);
      }
    }

  template <typename GT1, typename GT2> void operator()(GT1 const&, GT2 &) const
      {
      assert(!"You shall not mix DataSources and Layers in GeometriesToGeometriesFilter");
      itkGenericExceptionMacro(<<"You shall not mix DataSources and Layers in GeometriesToGeometriesFilter");
      }
private:
  otb::GeometriesToGeometriesFilter const& m_filter;
};

/*virtual*/ void otb::GeometriesToGeometriesFilter::Process(
  InputGeometriesType const& source, OutputGeometriesType & destination)
{
  std::cout << "G2GF: Processing ...\n";
  // si layer, appelle virt process layer
  // si DS, loop et appelle virt process layer
  source.apply(ProcessVisitor(*this), destination);
}

/*virtual*/
void otb::GeometriesToGeometriesFilter::GenerateOutputInformation(void )
{
  Superclass::GenerateOutputInformation();

#if 0
  // Apply only with data sources
  OutputGeometriesType::Pointer output = this->GetOutput();
  InputGeometriesType::Pointer  input = this->GetInput();
  output->SetMetaDataDictionary(input->GetMetaDataDictionary());
#endif
}

/*virtual*/
void otb::GeometriesToGeometriesFilter::GenerateData(void )
{
  // this->AllocateOutputs();
  OutputGeometriesType::Pointer      output = this->GetOutput();
  InputGeometriesType::ConstPointer  input  = this->GetInput();

  // Start recursive processing
  itk::TimeProbe chrono;
  chrono.Start();
  this->Process(*input, *output);
  chrono.Stop();
  otbMsgDevMacro(<< "GeometriesToGeometriesFilter: geometries processed in " << chrono.GetMeanTime() << " seconds.");
}
