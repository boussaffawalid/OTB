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

#include <boost/algorithm/string.hpp>

#include "otbStreamingStatisticsVectorImageFilter.h"
#include "otbEigenvalueLikelihoodMaximisation.h"
#include "otbVcaImageFilter.h"
#include "otbUnConstrainedLeastSquareImageFilter.h"
#include "otbISRAUnmixingImageFilter.h"
#include "otbNCLSUnmixingImageFilter.h"
#include "otbFCLSUnmixingImageFilter.h"

#include "otbVectorImageToMatrixImageFilter.h"


namespace otb
{
namespace Wrapper
{

const unsigned int Dimension = 2;

typedef otb::StreamingStatisticsVectorImageFilter<DoubleVectorImageType> StreamingStatisticsVectorImageFilterType;
typedef otb::EigenvalueLikelihoodMaximisation<double> ELMType;
typedef otb::VCAImageFilter<DoubleVectorImageType> VCAFilterType;

typedef otb::UnConstrainedLeastSquareImageFilter<DoubleVectorImageType, DoubleVectorImageType, double> UCLSUnmixingFilterType;
typedef otb::ISRAUnmixingImageFilter<DoubleVectorImageType, DoubleVectorImageType, double>             ISRAUnmixingFilterType;
typedef otb::NCLSUnmixingImageFilter<DoubleVectorImageType, DoubleVectorImageType, double>             NCLSUnmixingFilterType;
typedef otb::FCLSUnmixingImageFilter<DoubleVectorImageType, DoubleVectorImageType, double>             FCLSUnmixingFilterType;

typedef otb::VectorImageToMatrixImageFilter<DoubleVectorImageType> VectorImageToMatrixImageFilterType;

typedef vnl_vector<double> VectorType;
typedef vnl_matrix<double> MatrixType;


enum DimReductionMethod
{
  DimReductionMethod_NONE,
  DimReductionMethod_PCA,
  DimReductionMethod_MNF
};

enum DimensionalityEstimationMethod
{
  DimensionalityEstimationMethod_ELM
};

enum EndmembersEstimationMethod
{
  EndmembersEstimationMethod_VCA
};

enum UnMixingMethod
{
  UnMixingMethod_UCLS,
  UnMixingMethod_ISRA,
  UnMixingMethod_NCLS,
  UnMixingMethod_FCLS,
};

const char* UnMixingMethodNames [] = { "UCLS", "ISRA", "NCLS", "FCLS", };


class HyperspectralUnmixing : public Application
{
public:
  /** Standard class typedefs. */
  typedef HyperspectralUnmixing         Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(HyperspectralUnmixing, otb::Application);

private:

  HyperspectralUnmixing()
  {
    SetName("HyperspectralUnmixing");
    SetDescription("Unmix an hyperspectral image");

    // Documentation
    SetDocName("Hyperspectral data unmixing");
    SetDocLongDescription("Applies an unmixing algorithm to an hyperspectral data cube");
    SetDocLimitations("None");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso(" ");
    SetDocCLExample("otbApplicationLauncherCommandLine VertexComponentAnalysis ${OTB-BIN}/bin --in ${INPUTDATA}/Hyperspectral/synthetic/hsi_cube.tif --ie ${INPUTDATA}/Hyperspectral/synthetic/endmembers.tif --out ${TEMP}/apTvHyHyperspectralUnmixing_UCLS.tif double --ua ucls");
    AddDocTag("Hyperspectral");
  }

  virtual ~HyperspectralUnmixing()
  {
  }

  void DoCreateParameters()
  {
    AddParameter(ParameterType_InputImage,  "in",   "Input Image");
    SetParameterDescription("in","The hyperspectral data cube to unmix");

    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription("out","The output abundance map");

    AddParameter(ParameterType_InputImage,  "ie",   "Input endmembers");
    SetParameterDescription("ie","The endmembers to use for unmixing. Must be stored as a multispectral image, where each pixel is interpreted as an endmember");

    AddParameter(ParameterType_Choice, "ua", "Unmixing algorithm");
    SetParameterDescription("ua", "The algorithm to use for unmixing");
    MandatoryOff("ua");
    AddChoice("ua.ucls", "UCLS");
    SetParameterDescription("ua.ucls", "Unconstrained Least Square");

    AddChoice("ua.isra", "ISRA");
    SetParameterDescription("ua.isra", "Image Space Reconstruction Algorithm");

    AddChoice("ua.ncls", "NCLS");
    SetParameterDescription("ua.ncls", "Non-negative constrained Least Square");

    AddChoice("ua.fcls", "FCLS");
    SetParameterDescription("ua.fcls", "Fully constrained Least Square");

    SetParameterString("ua", "ucls");

  }

  void DoUpdateParameters()
  {
    // Nothing to do here : all parameters are independent
  }

  void DoExecute()
  {
    m_ProcessObjects.clear();

    DoubleVectorImageType::Pointer inputImage = GetParameterImage<DoubleVectorImageType>("in");
    DoubleVectorImageType::Pointer endmembersImage = GetParameterImage<DoubleVectorImageType>("ie");

    /*
     * Transform Endmembers image to matrix representation
     */
    std::cout << "Endmembers extracted" << std::endl;
    std::cout << "Converting endmembers to matrix" << std::endl;
    VectorImageToMatrixImageFilterType::Pointer endMember2Matrix = VectorImageToMatrixImageFilterType::New();
    endMember2Matrix->SetInput(endmembersImage);
    endMember2Matrix->Update();

    MatrixType endMembersMatrix = endMember2Matrix->GetMatrix();
    std::cout << "Endmembers matrix : " << endMembersMatrix << std::endl;

    /*
     * Unmix
     */
    DoubleVectorImageType::Pointer abundanceMap;

    switch ( static_cast<UnMixingMethod>(GetParameterInt("ua")) )
    {
    case UnMixingMethod_UCLS:
      {
      std::cout << "UCLS Unmixing" << std::endl;

      UCLSUnmixingFilterType::Pointer unmixer =
          UCLSUnmixingFilterType::New();

      unmixer->SetInput(inputImage);
      unmixer->SetMatrix(endMembersMatrix);
      unmixer->SetNumberOfThreads(1); // FIXME : currently buggy

      abundanceMap = unmixer->GetOutput();
      m_ProcessObjects.push_back(unmixer.GetPointer());

      }
      break;
    case UnMixingMethod_ISRA:
      {
      std::cout << "ISRA Unmixing" << std::endl;

      ISRAUnmixingFilterType::Pointer unmixer =
          ISRAUnmixingFilterType::New();

      unmixer->SetInput(inputImage);
      unmixer->SetEndmembersMatrix(endMembersMatrix);
      abundanceMap = unmixer->GetOutput();
      m_ProcessObjects.push_back(unmixer.GetPointer());

      }
      break;
    case UnMixingMethod_NCLS:
      {
      std::cout << "NCLS Unmixing" << std::endl;

      NCLSUnmixingFilterType::Pointer unmixer =
          NCLSUnmixingFilterType::New();

      unmixer->SetInput(inputImage);
      unmixer->SetEndmembersMatrix(endMembersMatrix);
      abundanceMap = unmixer->GetOutput();
      m_ProcessObjects.push_back(unmixer.GetPointer());

      }
      break;
    case UnMixingMethod_FCLS:
      {
      std::cout << "FCLS Unmixing" << std::endl;

      FCLSUnmixingFilterType::Pointer unmixer =
          FCLSUnmixingFilterType::New();

      unmixer->SetInput(inputImage);
      unmixer->SetEndmembersMatrix(endMembersMatrix);
      abundanceMap = unmixer->GetOutput();
      m_ProcessObjects.push_back(unmixer.GetPointer());

      }
      break;
    default:
      break;
    }

    SetParameterOutputImage<DoubleVectorImageType>("out", abundanceMap);

  }

  std::vector<itk::ProcessObject::Pointer> m_ProcessObjects;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::HyperspectralUnmixing)
