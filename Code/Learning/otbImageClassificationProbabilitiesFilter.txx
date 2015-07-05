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
#ifndef __otbImageClassificationProbabilitiesFilter_txx
#define __otbImageClassificationProbabilitiesFilter_txx

#include "otbImageClassificationProbabilitiesFilter.h"
#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"

namespace otb
{
/**
 * Constructor
 */
template <class TInputImage, class TOutputImage, class TMaskImage>
ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::ImageClassificationProbabilitiesFilter()
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfRequiredInputs(1);  
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::SetInputMask(const MaskImageType * mask)
{
  this->itk::ProcessObject::SetNthInput(1, const_cast<MaskImageType *>(mask));
}

template <class TInputImage, class TOutputImage, class TMaskImage>
const typename ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::MaskImageType *
ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::GetInputMask()
{
  if (this->GetNumberOfInputs() < 2)
    {
    return 0;
    }
  return static_cast<const MaskImageType *>(this->itk::ProcessObject::GetInput(1));
}


template <class TInputImage, class TOutputImage, class TMaskImage>
void
ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::GenerateOutputInformation()
{
    Superclass::GenerateOutputInformation();
    
    m_nbclass = this->GetModel()->GetNumberOfClasses()  ;
    
    m_DefaultprobaVector =  itk::VariableLengthVector<ProbabilityType>( m_nbclass );
    for(int i(0); i<m_nbclass; i++ )
      m_DefaultprobaVector[i] = itk::NumericTraits<ProbabilityType>::ZeroValue();

    this->GetOutput()->SetNumberOfComponentsPerPixel( m_nbclass );
}


template <class TInputImage, class TOutputImage, class TMaskImage>
void
ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::BeforeThreadedGenerateData()
{
  if (!m_Model)
    {
    itkGenericExceptionMacro(<< "No model for classification");
    }
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId)
{
  // Get the input pointers
  InputImageConstPointerType inputPtr     = this->GetInput();
  MaskImageConstPointerType  inputMaskPtr  = this->GetInputMask();
  OutputImagePointerType     outputPtr    = this->GetOutput();

  // Progress reporting
  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Define iterators
  typedef itk::ImageRegionConstIterator<InputImageType> InputIteratorType;
  typedef itk::ImageRegionConstIterator<MaskImageType>  MaskIteratorType;
  typedef itk::ImageRegionIterator<OutputImageType>     OutputIteratorType;

  InputIteratorType inIt(inputPtr, outputRegionForThread);
  OutputIteratorType outIt(outputPtr, outputRegionForThread);

  // Eventually iterate on masks
  MaskIteratorType maskIt;
  if (inputMaskPtr)
    {
    maskIt = MaskIteratorType(inputMaskPtr, outputRegionForThread);
    maskIt.GoToBegin();
    }

  bool validPoint = true;

  // Walk the part of the image
  for (inIt.GoToBegin(), outIt.GoToBegin(); !inIt.IsAtEnd() && !outIt.IsAtEnd(); ++inIt, ++outIt)
    {
    // Check pixel validity
    if (inputMaskPtr)
      {
      validPoint = maskIt.Get() > 0;
      ++maskIt;
      }
    // If point is valid
    if (validPoint)
      {
	  // proba
	  ProbabilityVectorType probaVector = itk::VariableLengthVector<ProbabilityType>( m_nbclass ) ;
	  probaVector = m_Model->GetProbability( inIt.Get() );	  
	  outIt.Set( probaVector );
      }
    else
      {
      // else, set default value
      outIt.Set(m_DefaultprobaVector);
      }
    progress.CompletedPixel();
    }

}
/**
 * PrintSelf Method
 */
template <class TInputImage, class TOutputImage, class TMaskImage>
void
ImageClassificationProbabilitiesFilter<TInputImage, TOutputImage, TMaskImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // End namespace otb
#endif
