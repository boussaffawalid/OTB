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
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbMeanShiftVectorImageFilter.h"

namespace otb
{
namespace Wrapper
{


class MeanShiftSegmentation : public Application
{
public:
  /** Standard class typedefs. */
  typedef MeanShiftSegmentation         Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  typedef otb::MeanShiftVectorImageFilter<FloatVectorImageType, FloatVectorImageType> MSFilterType;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(MeanShiftSegmentation, otb::Application);

private:
  void DoInit()
  {
    SetName("MeanShiftSegmentation");
    SetDescription("Perform mean shift clustering");

    // Documentation
    SetDocName("Mean Shift segmentation");
    SetDocLongDescription("This application performs mean shift clustering.");
    SetDocLimitations("This application relies on old mean shift implementation (EDISON), which is going to be replaced by new one.");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso(" ");

    AddDocTag(Tags::Segmentation);

    AddParameter(ParameterType_InputImage,   "in",     "Input Image");
    SetParameterDescription( "in", "The input image." );
    AddParameter(ParameterType_OutputImage,  "fout",    "Filtered output");
    SetParameterDescription( "fout", "The filtered output image." );
    AddParameter(ParameterType_OutputImage,  "cout",    "Clustered output");
    SetParameterDescription( "cout", "The clustered output image." );
    AddParameter(ParameterType_OutputImage,  "lout",    "Label output");
    SetParameterDescription( "lout", "The label output image." );
    AddParameter(ParameterType_OutputImage,  "cbout",   "Cluster Boundaries output");
    SetParameterDescription( "cbout", "The cluster boundaries output image." );
    MandatoryOff("fout");
    MandatoryOff("cout");
    MandatoryOff("lout");
    MandatoryOff("cbout");

    AddParameter(ParameterType_Int,          "spatialr",   "Spatial radius");
    SetParameterDescription( "spatialr", "Spatial radius defining neighborhood." );
    AddParameter(ParameterType_Float,        "ranger",   "Range radius");
    SetParameterDescription( "ranger", "Range radius defining the interval in the color space." );
    AddParameter(ParameterType_Int,          "minsize",   "Min region size");
    SetParameterDescription( "minsize", "Minimun size of a region to be kept after clustering." );
    AddParameter(ParameterType_Float,        "scale",   "Scale");
    SetParameterDescription( "scale", "Scale to stretch the image before processing." );
    SetDefaultParameterInt("spatialr",   5);
    SetDefaultParameterFloat("ranger",   15.0);
    SetDefaultParameterInt("minsize",    100);
    SetDefaultParameterFloat("scale",    100000.);

    // Doc example parameter settings
    SetDocExampleParameterValue("in", "maur_rgb.png");
    SetDocExampleParameterValue("cout", "MeanShift_ClusteredOutput.tif");
    SetDocExampleParameterValue("fout", "MeanShift_FilterOutput.tif");
    SetDocExampleParameterValue("lout", "MeanShift_LabeledClusteredOutput.tif uint32");
    SetDocExampleParameterValue("cbout", "MeanShift_ClusterBoundariesOutput.tif uint8");
    SetDocExampleParameterValue("spatialr", "16");
    SetDocExampleParameterValue("ranger", "16");
    SetDocExampleParameterValue("minsize", "10");
    SetDocExampleParameterValue("scale", "1.0");

  }
  
  void DoUpdateParameters()
  {
    // Nothing to do here : all parameters are independent
  }

  void DoExecute()
  {
    FloatVectorImageType* input = GetParameterImage("in");

    m_Filter = MSFilterType::New();

    m_Filter->SetInput(input);
    m_Filter->SetSpatialRadius(GetParameterInt("spatialr"));
    m_Filter->SetRangeRadius(GetParameterFloat("ranger"));
    m_Filter->SetMinimumRegionSize(GetParameterInt("minsize"));
    m_Filter->SetScale(GetParameterFloat("scale"));

    if (IsParameterEnabled("fout") && HasValue("fout")) SetParameterOutputImage("fout", m_Filter->GetOutput());
    if (IsParameterEnabled("cout") && HasValue("cout")) SetParameterOutputImage("cout", m_Filter->GetClusteredOutput());
    if (IsParameterEnabled("lout") && HasValue("lout")) SetParameterOutputImage<UInt16ImageType> ("lout", m_Filter->GetLabeledClusteredOutput());
    if (IsParameterEnabled("cbout") && HasValue("cbout")) SetParameterOutputImage<UInt16ImageType> ("cbout", m_Filter->GetClusterBoundariesOutput());
  }

  MSFilterType::Pointer m_Filter;

};


}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::MeanShiftSegmentation)
