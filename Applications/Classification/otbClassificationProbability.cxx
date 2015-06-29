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

#include "otbChangeLabelImageFilter.h"
#include "otbStandardWriterWatcher.h"
#include "otbStatisticsXMLFileReader.h"
#include "otbShiftScaleVectorImageFilter.h"
#include "otbClassificationProbabilityFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbMachineLearningModelFactory.h"


#include "otbImageListToVectorImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageList.h"

namespace otb
{
namespace Wrapper
{

class ClassificationProbability : public Application
{
public:
  /** Standard class typedefs. */
  typedef ClassificationProbability            Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(ClassificationProbability, otb::Application);

  /** Filters typedef  */
    
  typedef FloatVectorImageType                                                                 InputImageType;
  typedef FloatImageType                                                                       OutputImageType;
  typedef UInt8ImageType                                                                       MaskImageType;
  typedef otb::ClassificationProbabilityFilter<InputImageType, OutputImageType, MaskImageType> ClassificationProbabilityFilterType;
  typedef ClassificationProbabilityFilterType::Pointer                                                    ClassificationFilterPointerType;
  typedef ClassificationProbabilityFilterType::ModelType                                                  ModelType;
  typedef ModelType::Pointer                                                                   ModelPointerType;
  typedef ClassificationProbabilityFilterType::ValueType                                                  ValueType;
  typedef ClassificationProbabilityFilterType::ProbaType                                                  ProbaType;
  typedef otb::MachineLearningModelFactory<ValueType, ProbaType>                               MachineLearningModelFactoryType;
 
  typedef otb::ImageList<OutputImageType>  ImageListType;
  typedef ImageListToVectorImageFilter<ImageListType,
                                       FloatVectorImageType >                   ListConcatenerFilterType;
    
private:
  void DoInit()
  {
    SetName("ClassificationProbability");
    SetDescription("Compute Classification Probability of the input image according to a model file.");

    // Documentation
    SetDocName("Classification Probability");
    SetDocLongDescription("This application Compute Classification Probability based on a model file produced by\
    the TrainImagesClassifier application.");
    
    SetDocAuthors("Boussaffa Walid");

    AddDocTag(Tags::Learning);

    AddParameter(ParameterType_InputImage, "in",  "Input Image");
    SetParameterDescription( "in", "The input image to classify.");

    AddParameter(ParameterType_InputImage,  "mask",   "Input Mask");
    SetParameterDescription( "mask", "The mask allows to restrict Probability computation of the input image to the area where mask pixel values are greater than 0.");
    MandatoryOff("mask");

    AddParameter(ParameterType_InputFilename, "model", "Model file");
    SetParameterDescription("model", "A model file (produced by TrainImagesClassifier application, maximal class label = 65535).");

    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription( "out", "Output image containing class Probabilities");
    SetParameterOutputImagePixelType( "out", ImagePixelType_uint8);

    AddRAMParameter();

   // Doc example parameter settings
    SetDocExampleParameterValue("in", "QB_1_ortho.tif");
    SetDocExampleParameterValue("model", "clsvmModelQB1.svm");
    SetDocExampleParameterValue("out", "ProbabilitiesImage.tif");
  }

  void DoUpdateParameters()
  {
    // Reinitialize the object
    m_Concatener = ListConcatenerFilterType::New();
    m_ImageList = ImageListType::New();
  }

  void DoExecute()
  {
    // Load input image
    InputImageType::Pointer inImage = GetParameterImage("in");
    inImage->UpdateOutputInformation();

    // Load svm model
    otbAppLogINFO("Loading model");
    m_Model = MachineLearningModelFactoryType::CreateMachineLearningModel(GetParameterString("model"),
                                                                          MachineLearningModelFactoryType::ReadMode);

    if (m_Model.IsNull())
      {
      otbAppLogFATAL(<< "Error when loading model " << GetParameterString("model") << " : unsupported model type");
      }

    m_Model->Load(GetParameterString("model"));
    otbAppLogINFO("Model loaded");

    
    // m_Classification Probability 
    m_ClassificationProbabilityFilter = ClassificationProbabilityFilterType::New();
    m_ClassificationProbabilityFilter->SetModel(m_Model);

    m_ClassificationProbabilityFilter->SetInput(inImage);

    if(IsParameterEnabled("mask"))
      {
      otbAppLogINFO("Using input mask");
      // Load mask image and cast into LabeledImageType
      MaskImageType::Pointer inMask = GetParameterUInt8Image("mask");

      m_ClassificationProbabilityFilter->SetInputMask(inMask);
      }

      ///////////////////////////////////////////////////////////
      
    m_ImageList->PushBack(  m_ClassificationProbabilityFilter->GetOutput() );
    m_ImageList->PushBack(  m_ClassificationProbabilityFilter->GetOutput() );

    m_Concatener->SetInput( m_ImageList );

    SetParameterOutputImage("out", m_Concatener->GetOutput());
  
   // SetParameterOutputImage<OutputImageType>("out", m_ClassificationProbabilityFilter->GetOutput() );
  }

  ClassificationProbabilityFilterType::Pointer m_ClassificationProbabilityFilter;
  ModelPointerType m_Model;
  
  ListConcatenerFilterType::Pointer  m_Concatener;
  ImageListType::Pointer        m_ImageList;
};


}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ClassificationProbability)
