/*=========================================================================
 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 Created By:Boussaffa Walid
 Email:	    Boussaffa.walid@outlook.com


 Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
 See OTBCopyright.txt for details.


 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef __otbImageClassificationProbabilityFilter_h
#define __otbImageClassificationProbabilityFilter_h

#include "itkImageToImageFilter.h"
#include "otbMachineLearningModel.h"

namespace otb
{
/** \class ImageClassificationProbabilityFilter
 *  \brief This filter performs the classification of a VectorImage using a Model.
 *
 *  This filter is streamed and threaded, allowing to classify huge images
 *  while fully using several core.
 *
 * \sa Classifier
 * \ingroup Streamed
 * \ingroup Threaded
 */
template <class TInputImage, class TOutputImage, class TMaskImage = TOutputImage>
class ITK_EXPORT ImageClassificationProbabilityFilter
  : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard typedefs */
  typedef ImageClassificationProbabilityFilter                       Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(ImageClassificationProbabilityFilter, ImageToImageFilter);

  typedef TInputImage                                InputImageType;
  typedef typename InputImageType::ConstPointer      InputImageConstPointerType;
  typedef typename InputImageType::InternalPixelType ValueType;

  typedef TMaskImage                           MaskImageType;
  typedef typename MaskImageType::ConstPointer MaskImageConstPointerType;
  typedef typename MaskImageType::Pointer      MaskImagePointerType;

  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointerType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  ProbabilityVectorType; //typedef VariableLengthVector< TPixel > PixelType;
  typedef typename OutputImageType::InternalPixelType  ProbabilityType;

  typedef MachineLearningModel<ValueType, ProbabilityType> ModelType;
  typedef typename ModelType::Pointer    ModelPointerType;

  /** Set/Get the model */
  itkSetObjectMacro(Model, ModelType);
  itkGetObjectMacro(Model, ModelType);

  /** Set/Get the default label */
  itkSetMacro(DefaultprobaVector, ProbabilityVectorType);
  itkGetMacro(DefaultprobaVector, ProbabilityVectorType);

  /** 
   * If set, only pixels within the mask will be classified.
   * All pixels with a value greater than 0 in the mask, will be classified.
   * \param mask The input mask.
   */
  void SetInputMask(const MaskImageType * mask);

  /**
   * Get the input mask.
   * \return The mask.
   */
  const MaskImageType * GetInputMask(void);

protected:
  /** Constructor */
  ImageClassificationProbabilityFilter();
  /** Destructor */
  virtual ~ImageClassificationProbabilityFilter() {}

  /** Threaded generate data */
  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId);
  /** Before threaded generate data */
  virtual void BeforeThreadedGenerateData();
  
    /** GenerateOutputInformation
   * Set the number of bands of the output.
   * Copy informations from the first image of the list if existing.
   **/
  virtual void GenerateOutputInformation(void);
  
  /**PrintSelf method */
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  ImageClassificationProbabilityFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  /** The model used for classification */
  ModelPointerType m_Model;
  /** Default label for invalid pixels (when using a mask) */
  ProbabilityVectorType m_DefaultprobaVector;
  int m_nbclass;

};
} // End namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbImageClassificationProbabilityFilter.txx"
#endif

#endif
