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

#include "otbStereorectificationDeformationFieldSource.h"

// Elevation handler
#include "otbWrapperElevationParametersHandler.h"
#include "itkInverseDeformationFieldImageFilter.h"
#include "itkVectorCastImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbDEMToImageGenerator.h"
#include "otbStreamingStatisticsImageFilter.h"

namespace otb
{
namespace Wrapper
{

class StereoRectificationGridGenerator : public Application
{
public:
  /** Standard class typedefs. */
  typedef StereoRectificationGridGenerator    Self;
  typedef Application                         Superclass;
  typedef itk::SmartPointer<Self>             Pointer;
  typedef itk::SmartPointer<const Self>       ConstPointer;

  typedef otb::StereorectificationDeformationFieldSource
  <FloatVectorImageType,FloatVectorImageType> DeformationFieldSourceType;
  
  typedef itk::Vector<double,2>               DeformationType;
  typedef otb::Image<DeformationType>         DeformationFieldType;
  
  typedef itk::VectorCastImageFilter
  <FloatVectorImageType,
   DeformationFieldType>                      DeformationFieldCastFilterType;

  typedef itk::InverseDeformationFieldImageFilter
  <DeformationFieldType,DeformationFieldType> InverseDeformationFieldFilterType;

  typedef itk::VectorIndexSelectionCastImageFilter<DeformationFieldType,
                                                   FloatImageType> IndexSelectionCastFilterType;

  typedef otb::ImageList<FloatImageType>      ImageListType;
  typedef otb::ImageListToVectorImageFilter<ImageListType,FloatVectorImageType> ImageListFilterType;

  typedef otb::DEMToImageGenerator<FloatImageType >           DEMToImageGeneratorType;
  typedef otb::StreamingStatisticsImageFilter<FloatImageType> StatisticsFilterType;
  
  
  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(StereoRectificationGridGenerator, otb::Application);

private:

  StereoRectificationGridGenerator()
  {
    // Instanciate deformation field source
    m_DeformationFieldSource = DeformationFieldSourceType::New();
    m_LeftInvertDeformationFieldFilter = InverseDeformationFieldFilterType::New();
    m_RightInvertDeformationFieldFilter = InverseDeformationFieldFilterType::New();

    m_LeftDeformationFieldCaster = DeformationFieldCastFilterType::New();
    m_LeftIndexSelectionFilter1  = IndexSelectionCastFilterType::New();
    m_LeftIndexSelectionFilter2  = IndexSelectionCastFilterType::New();
    m_LeftImageList              = ImageListType::New();
    m_LeftImageListFilter        = ImageListFilterType::New();

    m_RightDeformationFieldCaster = DeformationFieldCastFilterType::New();
    m_RightIndexSelectionFilter1  = IndexSelectionCastFilterType::New();
    m_RightIndexSelectionFilter2  = IndexSelectionCastFilterType::New();
    m_RightImageList              = ImageListType::New();
    m_RightImageListFilter        = ImageListFilterType::New();

    m_DEMToImageGenerator         = DEMToImageGeneratorType::New();
    m_StatisticsFilter            = StatisticsFilterType::New();
  }

  void DoInit()
  {
    SetName("StereoRectificationGridGenerator");
    SetDescription("Generates two deformation fields to stereo-rectify (i.e. resample in epipolar geometry) a pair of stereo images up to the sensor model precision");

    SetDocName("Stereo-rectification deformation grid generator");
    SetDocLongDescription("This application generates a pair of deformation grid to stereo-rectify a pair of stereo images according to sensor modelling and a mean elevation hypothesis. The deformation grids can be passed to the StereoRectificationGridGenerator application for actual resampling in epipolar geometry.");
    SetDocLimitations("Generation of the deformation grid is not streamable, pay attention to this fact when setting the grid step.");
    SetDocAuthors("OTB-Team");

    AddDocTag(Tags::Geometry);

    SetDocSeeAlso("otbGridBasedImageResampling");

    AddParameter(ParameterType_Group,"io","Input and output data");
    SetParameterDescription("io","This group of parameters allows to set the input and output images.");
    AddParameter(ParameterType_InputImage,"io.inleft","Left input image");
    SetParameterDescription("io.inleft","The left input image to resample");

    AddParameter(ParameterType_InputImage,"io.inright","Right input image");
    SetParameterDescription("io.inright","The right input image to resample");

    AddParameter(ParameterType_OutputImage, "io.outleft", "Left output deformation grid");
    SetParameterDescription("io.outleft","The output deformation grid to be used to resample the left input image");

    AddParameter(ParameterType_OutputImage, "io.outright", "Right output deformation grid");
    SetParameterDescription("io.outright","The output deformation grid to be used to resample the right input image");

    AddParameter(ParameterType_Group,"epi","Epipolar  geometry and grid parameters");
    SetParameterDescription("epi","Parameters of the epipolar geometry and output grids");
    
    AddParameter(ParameterType_Choice,"epi.elevation","Elevation management");
    SetParameterDescription("epi.elevation","Manage elevation source for stereo-rectification");
    AddChoice("epi.elevation.avg","User-defined average elevation");
    SetParameterDescription("epi.elevation.avg","Average elevation defined by user");

    AddParameter(ParameterType_Float,"epi.elevation.avg.value","Average elevation value");
    SetParameterDescription("epi.elevation.avg.value","Average elevation value");

    AddChoice("epi.elevation.avgdem","Average elevation computed from DEM");
    SetParameterDescription("epi.elevation.avgdem","Average elevation computed from the provided DEM");

    AddParameter(ParameterType_Directory,"epi.elevation.avgdem.path","DEM directory");
    SetParameterDescription("epi.elevation.avgdem.path","Path to the DEM directory");
    
    AddParameter(ParameterType_InputFilename,"epi.elevation.avgdem.geoid","Geoid file");
    SetParameterDescription("epi.elevation.avgdem.geoid","Path to the geoid file");
    MandatoryOff("epi.elevation.avgdem.geoid");

     AddParameter(ParameterType_Int,"epi.elevation.avgdem.step","Sub-sampling step");
    SetParameterDescription("epi.elevation.avgdem.step","Step of sub-sampling for average elevation estimation");
    SetDefaultParameterInt("epi.elevation.avgdem.step",1);
    SetMinimumParameterIntValue("epi.elevation.avgdem.step",1);

    AddParameter(ParameterType_Float,"epi.elevation.avgdem.value","Average elevation value");
    SetParameterDescription("epi.elevation.avgdem.value","Average elevation value estimated from DEM");
    SetParameterRole("epi.elevation.avgdem.value",Role_Output);

    AddParameter(ParameterType_Float,"epi.elevation.avgdem.mindisp","Minimum disparity from DEM");
    SetParameterDescription("epi.elevation.avgdem.mindisp","Disparity corresponding to estimated minimum elevation over the left image");
    SetParameterRole("epi.elevation.avgdem.mindisp",Role_Output);

    AddParameter(ParameterType_Float,"epi.elevation.avgdem.maxdisp","Maximum disparity from DEM");
    SetParameterDescription("epi.elevation.avgdem.maxdisp","Disparity corresponding to estimated maximum elevation over the left image");
    SetParameterRole("epi.elevation.avgdem.maxdisp",Role_Output);
    
    AddChoice("epi.elevation.dem","Elevation from DEM");
    SetParameterDescription("epi.elevation.dem","Local elevations from the provided DEM");

    AddParameter(ParameterType_Directory,"epi.elevation.dem.path","DEM directory");
    SetParameterDescription("epi.elevation.dem.path","Path to the DEM directory");
    
    AddParameter(ParameterType_InputFilename,"epi.elevation.dem.geoid","Geoid file");
    SetParameterDescription("epi.elevation.dem.geoid","Path to the geoid file");
    MandatoryOff("epi.elevation.dem.geoid");

    AddParameter(ParameterType_Float,"epi.scale","Scale of epipolar images");
    SetParameterDescription("epi.scale","The scale parameter allows to generated zoomed-in (scale < 1) or zoomed-out (scale > 1) epipolar images.");
    SetDefaultParameterFloat("epi.scale",1.);
    
    AddParameter(ParameterType_Int,"epi.step","Step of the deformation grid (in nb. of pixels)");
    SetParameterDescription("epi.step","Stereo-rectification deformation grid only varies slowly. Therefore, it is recommanded to use a coarser grid (higher step value) in case of large images");
    SetDefaultParameterInt("epi.step",1);

    AddParameter(ParameterType_Int,"epi.rectsizex","Rectified image size X");
    SetParameterDescription("epi.rectsizex","The application computes the optimal rectified image size so that the whole left input image fits into the rectified area. However, due to the scale and step parameter, this size may not match the size of the deformation field output. In this case, one can use these output values.");
    SetParameterRole("epi.rectsizex", Role_Output);

    AddParameter(ParameterType_Int,"epi.rectsizey","Rectified image size Y");
    SetParameterDescription("epi.rectsizey","The application computes the optimal rectified image size so that the whole left input image fits into the rectified area. However, due to the scale and step parameter, this size may not match the size of the deformation field output. In this case, one can use these output values.");
    SetParameterRole("epi.rectsizey", Role_Output);

    AddParameter(ParameterType_Float,"epi.baseline","Mean baseline ratio");
    SetParameterDescription("epi.baseline","This parameter is the mean value, in meters, of the baseline to sensor altitude ratio. It can be used to convert disparities to physical elevation, since a disparity of one pixel will corresspond to an elevation offset of this value with respect to the mean elevation.");
    SetParameterRole("epi.baseline", Role_Output);

    AddParameter(ParameterType_Group,"inverse","Write inverse fields");
    SetParameterDescription("inverse","This group of parameter allows to generate the inverse fields as well");

    AddParameter(ParameterType_OutputImage, "inverse.outleft", "Left inverse deformation grid");
    SetParameterDescription("inverse.outleft","The output deformation grid to be used to resample the epipolar left image");
    MandatoryOff("inverse.outleft");

    AddParameter(ParameterType_OutputImage, "inverse.outright", "Right inverse deformation grid");
    SetParameterDescription("inverse.outright","The output deformation grid to be used to resample the epipolar right image");
    MandatoryOff("inverse.outright");

    AddParameter(ParameterType_Int, "inverse.ssrate", "Sub-sampling rate for inversion");
    SetParameterDescription("inverse.ssrate","Grid inversion is an heavy process that implies spline regression on control points. To avoid eating to much memory, this parameter allows to first sub-sample the field to invert.");
    SetDefaultParameterInt("inverse.ssrate",16);
    SetMinimumParameterIntValue("inverse.ssrate",1);

    // Doc example
    SetDocExampleParameterValue("io.inleft","wv2_xs_left.tif");
    SetDocExampleParameterValue("io.inright","wv2_xs_left.tif");
    SetDocExampleParameterValue("io.outleft","wv2_xs_left_epi_field.tif");
    SetDocExampleParameterValue("io.outright","wv2_xs_right_epi_field.tif");
    SetDocExampleParameterValue("epi.elevation.average","400");
  }

  void DoUpdateParameters()
  {
    // Nothing to do here
  }

  void DoExecute()
  {
    m_DeformationFieldSource->SetLeftImage(GetParameterImage("io.inleft"));
    m_DeformationFieldSource->SetRightImage(GetParameterImage("io.inright"));
    m_DeformationFieldSource->SetGridStep(GetParameterInt("epi.step"));
    m_DeformationFieldSource->SetScale(GetParameterFloat("epi.scale"));

    if(GetParameterString("epi.elevation") == "avg")
      {
      
      m_DeformationFieldSource->SetAverageElevation(GetParameterFloat("epi.elevation.avg.value"));
      }
    else if(GetParameterString("epi.elevation") == "avgdem")
      {
      // TODO: Implement me
      FloatImageType::PointType   origin  = GetParameterImage("io.inleft")->GetOrigin();
      FloatImageType::SizeType    size    = GetParameterImage("io.inleft")->GetLargestPossibleRegion().GetSize();
      FloatImageType::SpacingType spacing = GetParameterImage("io.inleft")->GetSpacing();

      size[0]/=GetParameterInt("epi.elevation.avgdem.step");
      size[1]/=GetParameterInt("epi.elevation.avgdem.step");
      spacing[0]*=GetParameterInt("epi.elevation.avgdem.step");
      spacing[1]*=GetParameterInt("epi.elevation.avgdem.step");

      m_DEMToImageGenerator->SetDEMDirectoryPath(GetParameterString("epi.elevation.avgdem.path"));
      m_DEMToImageGenerator->SetOutputOrigin(origin);
      m_DEMToImageGenerator->SetOutputSize(size);
      m_DEMToImageGenerator->SetOutputSpacing(spacing);
      m_DEMToImageGenerator->SetOutputProjectionRef(GetParameterImage("io.inleft")->GetProjectionRef());
      m_DEMToImageGenerator->SetOutputKeywordList(GetParameterImage("io.inleft")->GetImageKeywordlist());
      // 0 by default
      m_DEMToImageGenerator->SetDefaultUnknownValue(0);

      m_DEMToImageGenerator->AboveEllipsoidOn();

      if(IsParameterEnabled("epi.elevation.avgdem.geoid"))
        {
        m_DEMToImageGenerator->SetGeoidFile(GetParameterString("epi.elevation.avgdem.geoid"));
        }

      m_StatisticsFilter->SetInput(m_DEMToImageGenerator->GetOutput());
      AddProcess(m_StatisticsFilter,"Computing DEM statistics ...");
      m_StatisticsFilter->Update();
      
      m_DeformationFieldSource->SetAverageElevation(m_StatisticsFilter->GetMean());

      SetParameterInt("epi.elevation.avgdem.value",m_StatisticsFilter->GetMean());

      }
    else if(GetParameterString("epi.elevation") == "dem")
      {
      m_DeformationFieldSource->SetDEMDirectory(GetParameterString("epi.elevation.dem.path"));

      if(IsParameterEnabled("epi.elevation.dem.geoid"))
        {
        m_DeformationFieldSource->SetGeoidFile(GetParameterString("epi.elevation.dem.geoid"));
        }
      }
                                                    
    AddProcess(m_DeformationFieldSource, "Computing epipolar grids ...");

    m_DeformationFieldSource->Update();

    SetParameterInt("epi.rectsizex",m_DeformationFieldSource->GetRectifiedImageSize()[0]);
    SetParameterInt("epi.rectsizey",m_DeformationFieldSource->GetRectifiedImageSize()[1]);
    SetParameterFloat("epi.baseline",m_DeformationFieldSource->GetMeanBaselineRatio());

    if(GetParameterString("epi.elevation") == "avgdem")
      {
      SetParameterFloat("epi.elevation.avgdem.mindisp",(m_StatisticsFilter->GetMinimum()-m_StatisticsFilter->GetMean())
                        /m_DeformationFieldSource->GetMeanBaselineRatio());
      SetParameterFloat("epi.elevation.avgdem.maxdisp",(m_StatisticsFilter->GetMaximum()-m_StatisticsFilter->GetMean())
                        /m_DeformationFieldSource->GetMeanBaselineRatio());
      }
    

    SetParameterOutputImage("io.outleft",m_DeformationFieldSource->GetLeftDeformationFieldOutput());
    SetParameterOutputImage("io.outright",m_DeformationFieldSource->GetRightDeformationFieldOutput());
  
    // Inverse part
    // lots of casting here ...

    // Left field inversion
    if(IsParameterEnabled("inverse.outleft"))
      {
      m_LeftDeformationFieldCaster->SetInput(m_DeformationFieldSource->GetLeftDeformationFieldOutput());

      m_LeftInvertDeformationFieldFilter->SetInput(m_LeftDeformationFieldCaster->GetOutput());
        
      FloatVectorImageType::PointType lorigin = GetParameterImage("io.inleft")->GetOrigin();
      FloatVectorImageType::SpacingType lspacing = GetParameterImage("io.inleft")->GetSpacing();
      FloatVectorImageType::SizeType lsize = GetParameterImage("io.inleft")->GetLargestPossibleRegion().GetSize();
      
      lspacing[0]*=GetParameterInt("epi.step");
      lspacing[1]*=GetParameterInt("epi.step");
       
      lsize[0]/=GetParameterInt("epi.step");
      lsize[1]/=GetParameterInt("epi.step");

      lsize[0]+=1;
      lsize[1]+=1;

      m_LeftInvertDeformationFieldFilter->SetOutputOrigin(lorigin);
      m_LeftInvertDeformationFieldFilter->SetOutputSpacing(lspacing);
      m_LeftInvertDeformationFieldFilter->SetSize(lsize);
      m_LeftInvertDeformationFieldFilter->SetSubsamplingFactor(GetParameterInt("inverse.ssrate"));
      AddProcess(m_LeftInvertDeformationFieldFilter, "Inverting left deformation field ...");
      m_LeftInvertDeformationFieldFilter->Update();

      m_LeftIndexSelectionFilter1->SetInput(m_LeftInvertDeformationFieldFilter->GetOutput());
      m_LeftIndexSelectionFilter1->SetIndex(0);
      m_LeftIndexSelectionFilter2->SetInput(m_LeftInvertDeformationFieldFilter->GetOutput());
      m_LeftIndexSelectionFilter2->SetIndex(1);
      m_LeftImageList->Clear();
      m_LeftImageList->PushBack(m_LeftIndexSelectionFilter1->GetOutput());
      m_LeftImageList->PushBack(m_LeftIndexSelectionFilter2->GetOutput());
      m_LeftImageListFilter->SetInput(m_LeftImageList);

      SetParameterOutputImage("inverse.outleft",m_LeftImageListFilter->GetOutput());
      }

    // Right field inversion

    if(IsParameterEnabled("inverse.outright"))
      {
      m_RightDeformationFieldCaster->SetInput(m_DeformationFieldSource->GetRightDeformationFieldOutput());

      m_RightInvertDeformationFieldFilter->SetInput(m_RightDeformationFieldCaster->GetOutput());
      FloatVectorImageType::PointType rorigin = GetParameterImage("io.inright")->GetOrigin();
      FloatVectorImageType::SpacingType rspacing = GetParameterImage("io.inright")->GetSpacing();
      FloatVectorImageType::SizeType rsize = GetParameterImage("io.inright")->GetLargestPossibleRegion().GetSize();

      rspacing[0]*=GetParameterInt("epi.step");
      rspacing[1]*=GetParameterInt("epi.step");
       
      rsize[0]/=GetParameterInt("epi.step");
      rsize[1]/=GetParameterInt("epi.step");

      rsize[0]+=1;
      rsize[1]+=1;

      m_RightInvertDeformationFieldFilter->SetOutputOrigin(rorigin);
      m_RightInvertDeformationFieldFilter->SetOutputSpacing(rspacing);
      m_RightInvertDeformationFieldFilter->SetSize(rsize);
      m_RightInvertDeformationFieldFilter->SetSubsamplingFactor(GetParameterInt("inverse.ssrate"));

      AddProcess(m_RightInvertDeformationFieldFilter, "Inverting right deformation field ...");
      m_RightInvertDeformationFieldFilter->Update();

      m_RightIndexSelectionFilter1->SetInput(m_RightInvertDeformationFieldFilter->GetOutput());
      m_RightIndexSelectionFilter1->SetIndex(0);
      m_RightIndexSelectionFilter2->SetInput(m_RightInvertDeformationFieldFilter->GetOutput());
      m_RightIndexSelectionFilter2->SetIndex(1);
      m_RightImageList->Clear();
      m_RightImageList->PushBack(m_RightIndexSelectionFilter1->GetOutput());
      m_RightImageList->PushBack(m_RightIndexSelectionFilter2->GetOutput());
      m_RightImageListFilter->SetInput(m_RightImageList);

      SetParameterOutputImage("inverse.outright",m_RightImageListFilter->GetOutput());
      }
  }

  DeformationFieldSourceType::Pointer          m_DeformationFieldSource;
  InverseDeformationFieldFilterType::Pointer   m_LeftInvertDeformationFieldFilter;
  DeformationFieldCastFilterType::Pointer      m_LeftDeformationFieldCaster;
  IndexSelectionCastFilterType::Pointer        m_LeftIndexSelectionFilter1;
  IndexSelectionCastFilterType::Pointer        m_LeftIndexSelectionFilter2;
  ImageListType::Pointer                       m_LeftImageList;
  ImageListFilterType::Pointer                 m_LeftImageListFilter;

  InverseDeformationFieldFilterType::Pointer   m_RightInvertDeformationFieldFilter;
  DeformationFieldCastFilterType::Pointer      m_RightDeformationFieldCaster;
  IndexSelectionCastFilterType::Pointer        m_RightIndexSelectionFilter1;
  IndexSelectionCastFilterType::Pointer        m_RightIndexSelectionFilter2;
  ImageListType::Pointer                       m_RightImageList;
  ImageListFilterType::Pointer                 m_RightImageListFilter;
  
  DEMToImageGeneratorType::Pointer             m_DEMToImageGenerator;
  StatisticsFilterType::Pointer                m_StatisticsFilter;

};
} // End namespace Wrapper
} // End namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::StereoRectificationGridGenerator)
