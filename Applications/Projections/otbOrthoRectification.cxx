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
#include "otbWrapperNumericalParameter.h"
#include "otbGenericRSResampleImageFilter.h"
#include "otbImageToGenericRSOutputParameters.h"
#include "otbMapProjections.h"

#include "itkLinearInterpolateImageFunction.h"
#include "otbBCOInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "otbMacro.h"

#include "otbWrapperRAMParameter.h"

namespace otb
{

enum
{
  Map_Utm,
  Map_Lambert2,
  Map_Epsg
};

enum
{
  Interpolator_Linear,
  Interpolator_NNeighbor,
  Interpolator_BCO
};

enum
{
  Mode_UserDefined,
  Mode_AutomaticSize,
  Mode_AutomaticSpacing
};

namespace Wrapper
{

class OrthoRectification : public Application
{
public:
/** Standard class typedefs. */
  typedef OrthoRectification            Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(OrthoRectification, otb::Application);

  /** Generic Remote Sensor Resampler */
  typedef otb::GenericRSResampleImageFilter<FloatVectorImageType,
                                            FloatVectorImageType>       ResampleFilterType;
  
  /** Interpolators typedefs*/
  typedef itk::LinearInterpolateImageFunction<FloatVectorImageType,
                                              double>              LinearInterpolationType;
  typedef itk::NearestNeighborInterpolateImageFunction<FloatVectorImageType,
                                                       double>     NearestNeighborInterpolationType;
  typedef otb::BCOInterpolateImageFunction<FloatVectorImageType>   BCOInterpolationType;

private:
  OrthoRectification()
  {
    SetName("OrthoRectification");
    std::ostringstream oss;
    oss << "This application allows to ortho-rectify optical images from supported sensors." << std::endl;
    SetDescription(oss.str());
    // Documentation
    SetDocName("Ortho-rectification application");
    oss.str("");
    oss<<"An inverse sensor model is built from the input image metadata to convert geographical to raw geometry coordinates. ";
    oss<<"This inverse sensor model is then combined with the chosen map projection to build a global coordinate mapping grid. Last, this grid is used to resample using the chosen interpolation algorithm. ";
    oss<<"A Digital Elevation Model can be specified to account for terrain deformations. "<<std::endl;
    oss<<"In case of SPOT5 images, the sensor model can be approximated by an RPC model in order to speed-up computation.";
    SetDocLongDescription(oss.str());
    SetDocLimitations("Supported sensors are SPOT5 (TIF format), Ikonos, Quickbird, Worldview2, GeoEye.");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso("Ortho-rectification chapter from the OTB Software Guide");
    SetDocCLExample(" ");
    AddDocTag(Tags::Manip);
    AddDocTag(Tags::Geometry);
  }

  void DoCreateParameters()
  {
    // Set the parameters
    AddParameter(ParameterType_Group,"io","Input and output data");
    SetParameterDescription("io","This group of parameters allows to set the input and output images.");
    AddParameter(ParameterType_InputImage, "io.in", "Input Image");
    SetParameterDescription("io.in","The input image to ortho-rectify");
    AddParameter(ParameterType_OutputImage, "io.out", "Output Image");
    SetParameterDescription("io.out","The ortho-rectified output image");

    // Built the Output Map Projection
    AddParameter(ParameterType_Choice, "map", "Output Map Projection");
    SetParameterDescription("map","Parameters of the ouptut map projection.");
    
    AddChoice("map.utm",   "Universal Trans-Mercator (UTM)");
    SetParameterDescription("map.utm","A system of transverse mercator projections dividing the surface of Earth between 80S and 84N latitude.");
    AddParameter(ParameterType_Int, "map.utm.zone", "Zone number");
    SetParameterDescription("map.utm.zone","The zone number ranges from 1 to 60 and allows to define the transverse mercator projection (along with the hemisphere)");
    AddParameter(ParameterType_Empty, "map.utm.hem",  "Northern Hemisphere");
    SetParameterDescription("map.utm.hem","The transverse mercator projections are defined by their zone number as well as the hemisphere. Activate this parameter if your image is in the northern hemisphere.");
    AddChoice("map.lambert2",  "Lambert II Etendu");
    SetParameterDescription("map.lambert2","This is a Lambert Conformal Conic projection mainly used in France.");

    AddChoice("map.epsg","EPSG Code");
    SetParameterDescription("map.epsg","This code is a generic way of identifying map projections, and allows to specify a large amount of them. See www.spatialreference.org to find which EPSG code is associated to your projection;");
    AddParameter(ParameterType_Int, "map.epsg.code", "EPSG Code");
    SetParameterDescription("map.epsg.code","See www.spatialreference.org to find which EPSG code is associated to your projection");
    SetDefaultParameterInt("map.epsg.code", 32631);
    SetParameterString("map", "epsg");

    
    // Add the output paramters in a group
    AddParameter(ParameterType_Group, "outputs", "Output Image Grid");
    SetParameterDescription("outputs","This group of parameters allows to define the grid on which the input image will be resampled.");

    // UserDefined values
    AddParameter(ParameterType_Choice, "outputs.mode", "Parameters estimation modes");
    AddChoice("outputs.mode.auto", "User Defined");
    SetParameterDescription("outputs.mode.auto","This mode allows you to fully modify default values.");
    AddChoice("outputs.mode.autosize", "Automatic Size from Spacing");
    SetParameterDescription("outputs.mode.autosize","This mode allows you to automatically compute the optimal image size from given spacing (pixel size) values");
    AddChoice("outputs.mode.autospacing", "Automatic Spacing from Size");
    SetParameterDescription("outputs.mode.autospacing","This mode allows you to automatically compute the optimal image spacing (pixel size) from the given size");
    // Upper left point coordinates
    AddParameter(ParameterType_Float, "outputs.ulx", "Upper Left X");
    SetParameterDescription("outputs.ulx","Cartographic X coordinate of upper-left corner (meters for cartographic projections, degrees for geographic ones)");

    AddParameter(ParameterType_Float, "outputs.uly", "Upper Left Y");
    SetParameterDescription("outputs.uly","Cartographic Y coordinate of the upper-left corner (meters for cartographic projections, degrees for geographic ones)");

    // Size of the output image
    AddParameter(ParameterType_Int, "outputs.sizex", "Size X");
    SetParameterDescription("outputs.sizex","Size of projected image along X (in pixels)");

    AddParameter(ParameterType_Int, "outputs.sizey", "Size Y");
    SetParameterDescription("outputs.sizey","Size of projected image along Y (in pixels)");
    
    // Spacing of the output image
    AddParameter(ParameterType_Float, "outputs.spacingx", "Pixel Size X");
    SetParameterDescription("outputs.spacingx","Size of each pixel along X axis (meters for cartographic projections, degrees for geographic ones)");


    AddParameter(ParameterType_Float, "outputs.spacingy", "Pixel Size Y");
    SetParameterDescription("outputs.spacingy","Size of each pixel along Y axis (meters for cartographic projections, degrees for geographic ones)");

    AddParameter(ParameterType_Empty,"outputs.isotropic","Force isotropic spacing by default");
    std::ostringstream isotropOss;
    isotropOss << "Default spacing (pixel size) values are estimated from the sensor modeling of the image. It can therefore result in a non-isotropic spacing. ";
    isotropOss << "This option allows you to force default values to be isotropic (in this case, the minimum of spacing in both direction is applied. ";
    isotropOss << "Values overriden by user are not affected by this option.";
    SetParameterDescription("outputs.isotropic", isotropOss.str());
    EnableParameter("outputs.isotropic");
    
    AddParameter(ParameterType_Group,"elev","Elevation management");
    SetParameterDescription("elev","This group of parameters allows to manage elevation values in the ortho-rectification process.");

    // DEM
    AddParameter(ParameterType_Directory, "elev.dem",   "DEM directory");
    SetParameterDescription("elev.dem","This parameter allows to select a directory containing Digital Elevation Model tiles. Supported formats are SRTM, DTED or any geotiff processed by the DEM import application");
    MandatoryOff("elev.dem");

    // Interpolators
    AddParameter(ParameterType_Choice,   "interpolator", "Interpolation");
    SetParameterDescription("interpolator","This group of parameters allows to define how the input image will be interpolated during resampling.");
    AddChoice("interpolator.nn",     "Nearest Neighbor interpolation");
    SetParameterDescription("interpolator.nn","Nearest neighbor interpolation leads to poor image quality, but it is very fast.");
    AddChoice("interpolator.linear", "Linear interpolation");
    SetParameterDescription("interpolator.linear","Linear interpolation leads to average image quality but is quite fast");
    AddChoice("interpolator.bco",    "Bicubic interpolation");
    AddParameter(ParameterType_Radius, "interpolator.bco.radius", "Radius for bicubic interpolation");
    SetParameterDescription("interpolator.bco.radius","This parameter allows to control the size of the bicubic interpolation filter. If the target pixel size is higher than the input pixel size, increasing this parameter will reduce aliasing artefacts.");
    SetDefaultParameterInt("interpolator.bco.radius", 2);

    AddParameter(ParameterType_Group,"opt","Speed optimization parameters");
    SetParameterDescription("opt","This group of parameters allows to optimize processing time.");

    // Estimate a RPC model (for spot image for instance)
    AddParameter(ParameterType_Int, "opt.rpc", "RPC modeling (points per axis)");
    SetDefaultParameterInt("opt.rpc", 10);
    SetParameterDescription("opt.rpc","Enabling RPC modeling allows to speed-up SPOT5 ortho-rectification. Value is the number of control points per axis for RPC estimation");
    DisableParameter("opt.rpc");
    MandatoryOff("opt.rpc");

    // RAM available
    AddParameter(ParameterType_RAM, "opt.ram", "Available memory for processing (in MB)");
    SetParameterDescription("opt.ram","This allows to set the maximum amount of RAM available for processing. As the writing task is time consuming, it is better to write large pieces of data, which can be achieved by increasing this parameter (pay attention to your system capabilities)");
    SetDefaultParameterInt("opt.ram", 256);
    MandatoryOff("opt.ram");

    // Deformation Field Spacing
    AddParameter(ParameterType_Float, "opt.gridspacing", "Resampling grid spacing");
    SetDefaultParameterFloat("opt.gridspacing", 4.);
    SetParameterDescription("opt.gridspacing", "Resampling is done according to a coordinate mapping grid, whose pixel size is set by this parameter. The closer to the output spacing this parameter is, the more precise will be the ortho-rectified image, but increasing this parameter allows to reduce processing time.");
    MandatoryOff("opt.gridspacing");
  }

  void DoUpdateParameters()
  {
    if (HasValue("io.in"))
      {
      // input image
      FloatVectorImageType::Pointer inImage = GetParameterImage("io.in");

      // Update the UTM zone params
      InitializeUTMParameters();
      // Get the output projection Ref
      this->UpdateOutputProjectionRef();

      // Compute the output image spacing and size
      typedef otb::ImageToGenericRSOutputParameters<FloatVectorImageType> OutputParametersEstimatorType;
      OutputParametersEstimatorType::Pointer genericRSEstimator = OutputParametersEstimatorType::New();
      
      if(IsParameterEnabled("outputs.isotropic"))
        {
        genericRSEstimator->EstimateIsotropicSpacingOn();
        }
      else
        {
        genericRSEstimator->EstimateIsotropicSpacingOff();
        }
      
      genericRSEstimator->SetInput(inImage);
      genericRSEstimator->SetOutputProjectionRef(m_OutputProjectionRef);
      genericRSEstimator->Compute();
      
      // Fill the Gui with the computed parameters
      if (!HasUserValue("outputs.ulx"))
        {
        SetParameterFloat("outputs.ulx", genericRSEstimator->GetOutputOrigin()[0]);
        }
      
      if (!HasUserValue("outputs.uly"))
        SetParameterFloat("outputs.uly", genericRSEstimator->GetOutputOrigin()[1]);

      if (!HasUserValue("outputs.sizex"))
        SetParameterInt("outputs.sizex", genericRSEstimator->GetOutputSize()[0]);

      if (!HasUserValue("outputs.sizey"))
        SetParameterInt("outputs.sizey", genericRSEstimator->GetOutputSize()[1]);

      if (!HasUserValue("outputs.spacingx"))
        SetParameterFloat("outputs.spacingx", genericRSEstimator->GetOutputSpacing()[0]);

      if (!HasUserValue("outputs.spacingy"))
        SetParameterFloat("outputs.spacingy", genericRSEstimator->GetOutputSpacing()[1]);

      // Handle the spacing and size field following the mode
      // choosed by the user
      switch (GetParameterInt("outputs.mode") )
        {
        case Mode_UserDefined:
        {
        // Automatic set to off
        AutomaticValueOff("outputs.sizex");
        AutomaticValueOff("outputs.sizey");
        AutomaticValueOff("outputs.spacingx");
        AutomaticValueOff("outputs.spacingy");

        // Enable add the parameters
        EnableParameter("outputs.spacingx");
        EnableParameter("outputs.spacingy");
        EnableParameter("outputs.sizex");
        EnableParameter("outputs.sizey");

        // Make all the parameters in this mode mandatory
        MandatoryOn("outputs.spacingx");
        MandatoryOn("outputs.spacingy");
        MandatoryOn("outputs.sizex");
        MandatoryOn("outputs.sizey");
        }
        break;
        case Mode_AutomaticSize:
        {
        // Disable the size fields
        DisableParameter("outputs.sizex");
        DisableParameter("outputs.sizey");
        EnableParameter("outputs.spacingx");
        EnableParameter("outputs.spacingy");

        // Update the automatic value mode of each filed
        AutomaticValueOn("outputs.sizex");
        AutomaticValueOn("outputs.sizey");
        AutomaticValueOff("outputs.spacingx");
        AutomaticValueOff("outputs.spacingy");

        // Adapat the status of the param to this mode
        MandatoryOn("outputs.spacingx");
        MandatoryOn("outputs.spacingy");
        MandatoryOff("outputs.sizex");
        MandatoryOff("outputs.sizey");

        ResampleFilterType::SpacingType spacing;
        spacing[0] = GetParameterFloat("outputs.spacingx");
        spacing[1] = GetParameterFloat("outputs.spacingy");
        
        genericRSEstimator->ForceSpacingTo(spacing);
        genericRSEstimator->Compute();
        
        // Set the  processed size relative to this forced spacing
        SetParameterInt("outputs.sizex", genericRSEstimator->GetOutputSize()[0]);
        SetParameterInt("outputs.sizey", genericRSEstimator->GetOutputSize()[1]);
        }
        break;
        case Mode_AutomaticSpacing:
        {
        // Disable the spacing fields and enable the size fields
        DisableParameter("outputs.spacingx");
        DisableParameter("outputs.spacingy");
        EnableParameter("outputs.sizex");
        EnableParameter("outputs.sizey");

        // Update the automatic value mode of each filed
        AutomaticValueOn("outputs.spacingx");
        AutomaticValueOn("outputs.spacingy");
        AutomaticValueOff("outputs.sizex");
        AutomaticValueOff("outputs.sizey");

        // Adapat the status of the param to this mode
        MandatoryOff("outputs.spacingx");
        MandatoryOff("outputs.spacingy");
        MandatoryOn("outputs.sizex");
        MandatoryOn("outputs.sizey");

        ResampleFilterType::SizeType size;
        size[0] = GetParameterInt("outputs.sizex");
        size[1] = GetParameterInt("outputs.sizey");

        genericRSEstimator->ForceSizeTo(size);
        genericRSEstimator->Compute();
        
        // Set the  processed spacing relative to this forced size
        SetParameterFloat("outputs.spacingx", genericRSEstimator->GetOutputSpacing()[0]);
        SetParameterFloat("outputs.spacingy", genericRSEstimator->GetOutputSpacing()[1]);
        }
        break;
        }
      }
  }

  void InitializeUTMParameters()
  {
    // Compute the zone and the hemisphere if not UserValue defined
    if(!HasUserValue("map.utm.zone")
       && HasValue("io.in")
       && !HasAutomaticValue("map.utm.zone"))
      {
      // Compute the Origin lat/long coordinate
      typedef otb::ImageToGenericRSOutputParameters<FloatVectorImageType> OutputParametersEstimatorType;
      OutputParametersEstimatorType::Pointer genericRSEstimator = OutputParametersEstimatorType::New();
      genericRSEstimator->SetInput(GetParameterImage("io.in"));
      genericRSEstimator->SetOutputProjectionRef(otb::GeoInformationConversion::ToWKT(4326));
      genericRSEstimator->Compute();

      int zone = otb::Utils::GetZoneFromGeoPoint(genericRSEstimator->GetOutputOrigin()[0],
                                                 genericRSEstimator->GetOutputOrigin()[1]);
      // Update the UTM Gui fields
      SetParameterInt("map.utm.zone", zone);
      if(genericRSEstimator->GetOutputOrigin()[0] > 0.)
        {
        EnableParameter("map.utm.hem");
        }

      AutomaticValueOn("map.utm.zone");
      AutomaticValueOn("map.utm.hem");
      }
  }

  void UpdateOutputProjectionRef()
  {
    switch ( GetParameterInt("map") )
      {
      case Map_Utm:
      {
      typedef UtmInverseProjection  UtmProjectionType;
      UtmProjectionType::Pointer    utmProjection = UtmProjectionType::New();

      // Set the zone
      utmProjection->SetZone(GetParameterInt("map.utm.zone"));

      // Set the hem
      std::string hem = "N";
      if (!IsParameterEnabled("map.utm.hem"))
        hem = "S";
      utmProjection->SetHemisphere(hem[0]);
        
      // Get the projection ref
      m_OutputProjectionRef = utmProjection->GetWkt();
      }
      break;
      case Map_Lambert2:
      {
      typedef Lambert2EtenduForwardProjection Lambert2ProjectionType;
      Lambert2ProjectionType::Pointer lambert2Projection = Lambert2ProjectionType::New();
      m_OutputProjectionRef = lambert2Projection->GetWkt();
      }
      break;
      case Map_Epsg:
      {
      m_OutputProjectionRef = otb::GeoInformationConversion::ToWKT(GetParameterInt("map.epsg.code"));
      }
      break;
      }
  }

  void DoExecute()
    {
    GetLogger()->Debug("Entering DoExecute");
    
    // Get the input image
    FloatVectorImageType* inImage = GetParameterImage("io.in");

    // Resampler Instanciation
    m_ResampleFilter = ResampleFilterType::New();
    m_ResampleFilter->SetInput(inImage);

    // Set the output projection Ref
    m_ResampleFilter->SetInputProjectionRef(inImage->GetProjectionRef());
    m_ResampleFilter->SetInputKeywordList(inImage->GetImageKeywordlist());
    m_ResampleFilter->SetOutputProjectionRef(m_OutputProjectionRef);

    // Get Interpolator
    switch ( GetParameterInt("interpolator") )
      {
      case Interpolator_Linear:
      {
      typedef itk::LinearInterpolateImageFunction<FloatVectorImageType,
        double>          LinearInterpolationType;
      LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
      m_ResampleFilter->SetInterpolator(interpolator);
      }
      break;
      case Interpolator_NNeighbor:
      {
      typedef itk::NearestNeighborInterpolateImageFunction<FloatVectorImageType,
        double> NearestNeighborInterpolationType;
      NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
      m_ResampleFilter->SetInterpolator(interpolator);
      }
      break;
      case Interpolator_BCO:
      {
      typedef otb::BCOInterpolateImageFunction<FloatVectorImageType>     BCOInterpolationType;
      BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
      interpolator->SetRadius(GetParameterInt("interpolator.bco.radius"));
      m_ResampleFilter->SetInterpolator(interpolator);
      }
      break;
      }

    // DEM
    if (IsParameterEnabled("elev.dem") && HasValue("elev.dem"))
      {
      m_ResampleFilter->SetDEMDirectory(GetParameterString("elev.dem"));
      }
    else
      {
      if ( otb::ConfigurationFile::GetInstance()->IsValid() )
        {
        m_ResampleFilter->SetDEMDirectory(otb::ConfigurationFile::GetInstance()->GetDEMDirectory());
        }
      }

    // If activated, generate RPC model
    if(IsParameterEnabled("opt.rpc"))
      {
      //std::cout <<"Estimating the rpc Model " <<std::endl;
      m_ResampleFilter->EstimateInputRpcModelOn();
      m_ResampleFilter->SetInputRpcGridSize( GetParameterInt("opt.rpc"));
      }
    
    // Set Output information
    ResampleFilterType::SizeType size;
    size[0] = GetParameterInt("outputs.sizex");
    size[1] = GetParameterInt("outputs.sizey");
    m_ResampleFilter->SetOutputSize(size);

    ResampleFilterType::SpacingType spacing;
    spacing[0] = GetParameterFloat("outputs.spacingx");
    spacing[1] = GetParameterFloat("outputs.spacingy");
    m_ResampleFilter->SetOutputSpacing(spacing);
    
    ResampleFilterType::OriginType ul;
    ul[0] = GetParameterFloat("outputs.ulx");
    ul[1] = GetParameterFloat("outputs.uly");
    m_ResampleFilter->SetOutputOrigin(ul);

    // Deformation Field spacing
    ResampleFilterType::SpacingType gridSpacing;
    if (IsParameterEnabled("opt.gridspacing"))
      {
      gridSpacing[0] = GetParameterFloat("opt.gridspacing");
      gridSpacing[1] = -GetParameterFloat("opt.gridspacing");
      m_ResampleFilter->SetDeformationFieldSpacing(gridSpacing);
      }

    // Output Image
    SetParameterOutputImage("io.out", m_ResampleFilter->GetOutput());
    }

  ResampleFilterType::Pointer  m_ResampleFilter;
  std::string                  m_OutputProjectionRef;
  };

  } // namespace Wrapper
} // namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::OrthoRectification)
