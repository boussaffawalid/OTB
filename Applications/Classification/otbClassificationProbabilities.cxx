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
#include "otbImageClassificationProbabilitiesFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbMachineLearningModelFactory.h"

namespace otb
{
namespace Wrapper
{

class ClassificationProbabilities : public Application
{
public:
    /** Standard class typedefs. */
    typedef ClassificationProbabilities            Self;
    typedef Application                   Superclass;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Standard macro */
    itkNewMacro ( Self );

    itkTypeMacro ( ClassificationProbabilities, otb::Application );

    typedef float InternalPixelType ;
    typedef otb::Image<InternalPixelType>                ImageType;
    typedef otb::VectorImage<InternalPixelType>          VectorImageType;

    /** Filters typedef */
    
    typedef VectorImageType                                                                      OutputImageType;
    typedef UInt8ImageType                                                                       MaskImageType;

    typedef otb::ImageClassificationProbabilitiesFilter<VectorImageType, OutputImageType, MaskImageType>      ClassificationFilterType;
    typedef ClassificationFilterType::Pointer                                                    ClassificationFilterPointerType;
    typedef ClassificationFilterType::ModelType                                                  ModelType;
    typedef ModelType::Pointer                                                                   ModelPointerType;
    typedef ClassificationFilterType::ValueType                                                  ValueType;
    typedef ClassificationFilterType::ProbabilityType                                                  ProbabilityType;
    typedef otb::MachineLearningModelFactory<ValueType, ProbabilityType>                               MachineLearningModelFactoryType;

private:
    void DoInit()
    {
        SetName ( "ClassificationProbabilities" );
        SetDescription ( "Performs a classification of the input image according to a model file." );

        // Documentation
        SetDocName ( "Image Classification" );
        SetDocLongDescription ( "This application performs an image classification based on a model file produced by the TrainImagesClassifier application. Pixels of the output image will contain the class labels decided by the classifier (maximal class label = 65535). The input pixels can be optionally centered and reduced according to the statistics file produced by the ComputeImagesStatistics application. An optional input mask can be provided, in which case only input image pixels whose corresponding mask value is greater than 0 will be classified. The remaining of pixels will be given the label 0 in the output image." );

        SetDocLimitations ( "The input image must have the same type, order and number of bands than the images used to produce the statistics file and the SVM model file. If a statistics file was used during training by the TrainImagesClassifier, it is mandatory to use the same statistics file for classification. If an input mask is used, its size must match the input image size." );
        SetDocAuthors ( "OTB-Team" );
        SetDocSeeAlso ( "TrainImagesClassifier, ValidateImagesClassifier, ComputeImagesStatistics" );

        AddDocTag ( Tags::Learning );

        AddParameter ( ParameterType_InputImage, "in",  "Input Image" );
        SetParameterDescription ( "in", "The input image to classify." );

	AddParameter ( ParameterType_Int, "nb",  "number of class" );
        SetParameterDescription ( "nb", "number of class." );
	
        AddParameter ( ParameterType_InputImage,  "mask",   "Input Mask" );
        SetParameterDescription ( "mask", "The mask allows to restrict classification of the input image to the area where mask pixel values are greater than 0." );
        MandatoryOff ( "mask" );

        AddParameter ( ParameterType_InputFilename, "model", "Model file" );
        SetParameterDescription ( "model", "A model file (produced by TrainImagesClassifier application, maximal class label = 65535)." );

        AddParameter ( ParameterType_OutputImage, "out",  "Output Image" );
        SetParameterDescription ( "out", "Output image containing class labels" );
        SetParameterOutputImagePixelType ( "out", ImagePixelType_uint8 );

        AddRAMParameter();

        // Doc example parameter settings
        SetDocExampleParameterValue ( "in", "QB_1_ortho.tif" );
        SetDocExampleParameterValue ( "imstat", "EstimateImageStatisticsQB1.xml" );
        SetDocExampleParameterValue ( "model", "clsvmModelQB1.svm" );
        SetDocExampleParameterValue ( "out", "clLabeledImageQB1.tif" );
    }

    void DoUpdateParameters()
    {
        // Nothing to do here : all parameters are independent
    }

    void DoExecute()
    {
        // Load input image
        FloatVectorImageType::Pointer inImage = GetParameterImage ( "in" );
        inImage->UpdateOutputInformation();

        // Load svm model
        otbAppLogINFO ( "Loading model" );
        m_Model = MachineLearningModelFactoryType::CreateMachineLearningModel ( GetParameterString ( "model" ),
                  MachineLearningModelFactoryType::ReadMode );

        if ( m_Model.IsNull() )
        {
            otbAppLogFATAL ( << "Error when loading model " << GetParameterString ( "model" ) << " : unsupported model type" );
        }

        m_Model->Load ( GetParameterString ( "model" ) );
        otbAppLogINFO ( "Model loaded" );

	
	m_Model->SetNumberOfClasses(GetParameterInt("nb"));
        // Classify
        m_ClassificationProbabilities = ClassificationFilterType::New();
        m_ClassificationProbabilities->SetModel ( m_Model );

            otbAppLogINFO ( "Input image normalization deactivated." );
            m_ClassificationProbabilities->SetInput ( inImage );

	    
        if ( IsParameterEnabled ( "mask" ) )
        {
            otbAppLogINFO ( "Using input mask" );
            // Load mask image and cast into LabeledImageType
            MaskImageType::Pointer inMask = GetParameterUInt8Image ( "mask" );

            m_ClassificationProbabilities->SetInputMask ( inMask );
        }

        SetParameterOutputImage<OutputImageType> ( "out", m_ClassificationProbabilities->GetOutput() );
    }

    ClassificationFilterType::Pointer m_ClassificationProbabilities;
    ModelPointerType m_Model;
    //RescalerType::Pointer m_Rescaler;
};


}
}

OTB_APPLICATION_EXPORT ( otb::Wrapper::ClassificationProbabilities )
