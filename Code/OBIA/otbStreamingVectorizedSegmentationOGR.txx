/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.

  Some parts of this code are derived from ITK. See ITKCopyright.txt
  for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbStreamingVectorizedSegmentationOGR_txx
#define __otbStreamingVectorizedSegmentationOGR_txx

#include "otbStreamingVectorizedSegmentationOGR.h"

#include "otbVectorDataTransformFilter.h"
#include "itkAffineTransform.h"

#include "itkTimeProbe.h"
#include "otbMacro.h"

namespace otb
{

template <class TImageType, class TSegmentationFilter>
PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>
::PersistentStreamingLabelImageToOGRDataFilter() : m_TileMaxLabel(0), m_StartLabel(0), m_Use8Connected(false),
  m_FilterSmallObject(false), m_MinimumObjectSize(1),m_Simplify(false), m_SimplificationTolerance(0.3)
{
   this->SetNumberOfInputs(3);
   this->SetNumberOfRequiredInputs(2);
   m_SegmentationFilter = SegmentationFilterType::New();
   m_TileNumber = 1;
}

template <class TImageType, class TSegmentationFilter>
PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>
::~PersistentStreamingLabelImageToOGRDataFilter()
{
}

template <class TImageType, class TSegmentationFilter>
void
PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>
::SetInputMask(const LabelImageType *mask)
{
  this->itk::ProcessObject::SetNthInput(2, const_cast<LabelImageType *>(mask));
}

template <class TImageType, class TSegmentationFilter>
const typename PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>
::LabelImageType *
PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>
::GetInputMask(void)
{
  if (this->GetNumberOfInputs() < 3)
    {
    return 0;
    }

  return static_cast<const LabelImageType *>(this->itk::ProcessObject::GetInput(2));
}

template <class TImageType, class TSegmentationFilter>
void
PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>
::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
}

template <class TImageType, class TSegmentationFilter>
typename PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>::OGRDataSourcePointerType
PersistentStreamingLabelImageToOGRDataFilter<TImageType, TSegmentationFilter>
::ProcessTile()
{
  otbMsgDebugMacro(<< "tile number : " << m_TileNumber);
  m_TileNumber = m_TileNumber + 1;
  itk::TimeProbe tileChrono;
  tileChrono.Start();
  
  // Apply an ExtractImageFilter to avoid problems with filters asking for the LargestPossibleRegion
  typedef itk::ExtractImageFilter<InputImageType, InputImageType> ExtractImageFilterType;
  typename ExtractImageFilterType::Pointer extract = ExtractImageFilterType::New();
  extract->SetInput( this->GetInput() );
  extract->SetExtractionRegion( this->GetInput()->GetRequestedRegion() );
  extract->Update();

  // WARNING: itk::ExtractImageFilter does not copy the MetadataDictionnary
  extract->GetOutput()->SetMetaDataDictionary(this->GetInput()->GetMetaDataDictionary());
  
  const unsigned int labelImageIndex = LabeledOutputAccessor<SegmentationFilterType>::LabeledOutputIndex;

  typename LabelImageToOGRDataSourceFilterType::Pointer labelImageToOGRDataFilter =
                                              LabelImageToOGRDataSourceFilterType::New();
  
  itk::TimeProbe chrono1;
  chrono1.Start();
  m_SegmentationFilter->SetInput(extract->GetOutput());
  m_SegmentationFilter->UpdateLargestPossibleRegion();
  m_SegmentationFilter->Update();
  
  chrono1.Stop();
  otbMsgDebugMacro(<<"segmentation took " << chrono1.GetTotal() << " sec");

  itk::TimeProbe chrono2;
  chrono2.Start();
  typename LabelImageType::ConstPointer inputMask = this->GetInputMask();
  if (!inputMask.IsNull())
  {
     // Apply an ExtractImageFilter to avoid problems with filters asking for the LargestPossibleRegion
     typedef itk::ExtractImageFilter<LabelImageType, LabelImageType> ExtractLabelImageFilterType;
     typename ExtractLabelImageFilterType::Pointer maskExtract = ExtractLabelImageFilterType::New();
     maskExtract->SetInput( this->GetInputMask() );
     maskExtract->SetExtractionRegion( this->GetInputMask()->GetRequestedRegion() );
     maskExtract->Update();
     // WARNING: itk::ExtractImageFilter does not copy the MetadataDictionnary
     maskExtract->GetOutput()->SetMetaDataDictionary(this->GetInputMask()->GetMetaDataDictionary());

     labelImageToOGRDataFilter->SetInputMask(maskExtract->GetOutput());
  }

  labelImageToOGRDataFilter->SetInput(dynamic_cast<LabelImageType *>(m_SegmentationFilter->GetOutputs().at(labelImageIndex).GetPointer()));
  labelImageToOGRDataFilter->SetFieldName(this->GetFieldName());
  labelImageToOGRDataFilter->SetUse8Connected(m_Use8Connected);
  labelImageToOGRDataFilter->Update();
  
  chrono2.Stop();
  otbMsgDebugMacro(<< "vectorization took " << chrono2.GetTotal() << " sec");

  //Relabeling & simplication of geometries & filtering small objects
  itk::TimeProbe chrono3;
  chrono3.Start();
  OGRDataSourcePointerType tmpDS = const_cast<OGRDataSourceType *>(labelImageToOGRDataFilter->GetOutput());
  OGRLayerType tmpLayer = tmpDS->GetLayer(0);

  typename OGRLayerType::iterator featIt = tmpLayer.begin();
  for(featIt = tmpLayer.begin(); featIt!=tmpLayer.end(); ++featIt)
  {
     ogr::Field field = (*featIt)[0];
     field.Unset();
     field.SetValue(m_TileMaxLabel);
     m_TileMaxLabel++;

     //Simplify the geometry
     if (m_Simplify)
     {
        #if GDAL_VERSION_NUM < 1800
           itkGenericExceptionMacro("Simplify geometry is not supported by OGR v"
               << GDAL_VERSION_NUM << ". Upgrade to a version >= 1.9.0, and recompile OTB.")
        #elif GDAL_VERSION_NUM < 1900
           (*featIt).SetGeometry((*featIt).GetGeometry()->Simplify(m_SimplificationTolerance));
        #else
           (*featIt).SetGeometry((*featIt).GetGeometry()->SimplifyPreserveTopology(m_SimplificationTolerance));
        #endif
     }
     //Need to rewrite the feature otherwise changes are not considered.
     tmpLayer.SetFeature(*featIt);
     
     //Filter small objects.
     if(m_FilterSmallObject)
     {
        double area = static_cast<const OGRPolygon *>((*featIt).GetGeometry())->get_Area();
        //convert into pixel coordinates
        typename InputImageType::SpacingType spacing = this->GetInput()->GetSpacing();
        double pixelsArea = area / (vcl_abs(spacing[0]*spacing[1]));
        otbMsgDebugMacro(<<"DN = "<<field.GetValue<int>()<<", area = "<<pixelsArea);
        if( pixelsArea < m_MinimumObjectSize )
        {
           tmpLayer.DeleteFeature((*featIt).GetFID());
        }
     }
  }
  chrono3.Stop();
  otbMsgDebugMacro(<< "relabeling, filtering small objects and simplifying geometries took " << chrono3.GetTotal() << " sec");
  
  return tmpDS;
}


} // end namespace otb
#endif
