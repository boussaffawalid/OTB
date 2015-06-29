
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
#ifndef __otbClassificationProbabilityFilter_txx
#define __otbClassificationProbabilityFilter_txx

//#include "otbClassificationProbabilityFilter.h"
#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"

namespace otb
{
/**
 * Constructor
 */
template <class TInputImage, class TOutputImage, class TMaskImage>
ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
::ClassificationProbabilityFilter()
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfRequiredInputs(1);
  m_DefaultProba = itk::NumericTraits<ProbaType>::ZeroValue();
  classIndex = 0 ;
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
::SetInputMask(const MaskImageType * mask)
{
  this->itk::ProcessObject::SetNthInput(1, const_cast<MaskImageType *>(mask));
}

template <class TInputImage, class TOutputImage, class TMaskImage>
const typename ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
::MaskImageType *
ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
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
ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
::BeforeThreadedGenerateData()
{
  if (!m_Model)
    {
    itkGenericExceptionMacro(<< "No model for classification");
    }
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
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
	 
	 // suppose we have two class 1 and 2 
	 // m_Model->Predict(inIt.Get())[0]  will return predicted class number (1 or 2)
	 // m_Model->GetProbability( inIt.Get() )  will return a vector contains classes probabilities
	 // example [0.25, 0.75] 
	 // 0.25 : probability of 1
	 // 0.75 : probability of 2
	  
 	 outIt.Set( 100.0*m_Model->GetProbability( inIt.Get() )[classIndex] );
	}
      else
	{
	  // else, set default value
	  outIt.Set(m_DefaultProba);
	}
      progress.CompletedPixel();
    }

}
/**
 * PrintSelf Method
 */
template <class TInputImage, class TOutputImage, class TMaskImage>
void
ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
ClassificationProbabilityFilter<TInputImage, TOutputImage, TMaskImage>
::SetClassIndex(const int idx)
{
   classIndex = idx;
}

} // End namespace otb
#endif
