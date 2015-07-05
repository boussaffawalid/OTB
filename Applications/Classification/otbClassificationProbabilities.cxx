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

#include "otbListSampleGenerator.h"


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

    typedef otb::ListSampleGenerator<FloatVectorImageType, VectorDataType> ListSampleGeneratorType;

private:
  void DoInit()
  {
	SetName("ClassificationProbabilities");
	SetDescription("Compute Classification Probabilities of the input image according to a model file.");

        // Documentation
        SetDocName ( "Classification Probabilities" );
	SetDocLongDescription("This application Compute Classification Probability based on a model file produced by\
			      the TrainImagesClassifier application.");
    

	SetDocAuthors("Boussaffa Walid");
	AddDocTag(Tags::Learning);

        AddParameter ( ParameterType_InputImage, "in",  "Input Image" );
        SetParameterDescription ( "in", "The input image to classify." );

	AddParameter(ParameterType_InputVectorDataList,  "vd",   "Input vector dataset");
	SetParameterDescription( "vd", "The input vector" );

	AddParameter(ParameterType_String, "field", "Name of the discrimination field");
	SetParameterDescription("field", "Name of the field used to discriminate class labels in the input vector data files.");
	SetParameterString("field", "Class");
    
	AddParameter(ParameterType_InputImage,  "mask",   "Input Mask");
	SetParameterDescription( "mask", "The mask allows to restrict Probabilities computation of the input image\
				to the area where mask pixel values are greater than 0.");
	MandatoryOff("mask");

        AddParameter ( ParameterType_InputFilename, "model", "Model file" );
        SetParameterDescription ( "model", "A model file (produced by TrainImagesClassifier application" );

        AddParameter ( ParameterType_OutputImage, "out",  "Output Image" );
        SetParameterDescription ( "out", "Output image containing class probabilities" );
        //SetParameterOutputImagePixelType ( "out", ImagePixelType_uint8 );

        AddRAMParameter();

        // Doc example parameter settings
	SetDocExampleParameterValue("in", "image.tif");
	SetDocExampleParameterValue("model", "model.svm");
	SetDocExampleParameterValue("vd", "shapefile.shp");
	SetDocExampleParameterValue("out", "ProbabilitiesImage.tif");
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

	
	VectorDataListType* vectorDataList = GetParameterVectorDataList("vd");
	
	// read the Vectordata
	VectorDataType::Pointer vectorData = vectorDataList->GetNthElement(0);
	vectorData->Update();
	
	//Sample list generator
	ListSampleGeneratorType::Pointer sampleGenerator = ListSampleGeneratorType::New();
	
	sampleGenerator->SetInput(inImage);
	sampleGenerator->SetInputVectorData(vectorData);
	sampleGenerator->SetClassKey(GetParameterString("field"));
	sampleGenerator->Update();

	int classSize = sampleGenerator->GetClassesSize().size() ;
	otbAppLogINFO("number of class : " << classSize);

    
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

	
	m_Model->SetNumberOfClasses( classSize );
        // Compute probabilities
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
};


}
}

OTB_APPLICATION_EXPORT ( otb::Wrapper::ClassificationProbabilities )
