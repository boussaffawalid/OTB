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
#ifndef __otbClassificationProbabilityFilter_h
#define __otbClassificationProbabilityFilter_h

#include "itkImageToImageFilter.h"
#include "otbMachineLearningModel.h"

namespace otb
{
/** \class ClassificationProbabilityFilter
 *  \brief This filter compute classification probability using a Model.
 *
 *  This filter is streamed and threaded, allowing to process huge images
 *  while fully using several core.
 *
 * \sa Classifier
 * \ingroup Streamed
 * \ingroup Threaded
 */
template <class TInputImage, class TOutputImage, class TMaskImage = TOutputImage>
class ITK_EXPORT ClassificationProbabilityFilter
  : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard typedefs */
  typedef ClassificationProbabilityFilter                       Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(ClassificationProbabilityFilter, ImageToImageFilter);

  typedef TInputImage                                InputImageType;
  typedef typename InputImageType::ConstPointer      InputImageConstPointerType;
  typedef typename InputImageType::InternalPixelType ValueType;

  typedef TMaskImage                           MaskImageType;
  typedef typename MaskImageType::ConstPointer MaskImageConstPointerType;
  typedef typename MaskImageType::Pointer      MaskImagePointerType;

  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointerType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  ProbaType;

  typedef MachineLearningModel<ValueType, ProbaType> ModelType;
  typedef typename ModelType::Pointer    ModelPointerType;

  /** Set/Get the model */
  itkSetObjectMacro(Model, ModelType);
  itkGetObjectMacro(Model, ModelType);

  /** Set/Get the default label */
  itkSetMacro(DefaultProba, ProbaType);
  itkGetMacro(DefaultProba, ProbaType);

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
  ClassificationProbabilityFilter();
  /** Destructor */
  virtual ~ClassificationProbabilityFilter() {}

  /** Threaded generate data */
  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId);
  /** Before threaded generate data */
  virtual void BeforeThreadedGenerateData();
  /**PrintSelf method */
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  void SetClassIndex(const int idx);
  
private:
  ClassificationProbabilityFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  /** The model used for classification */
  ModelPointerType m_Model;
  /** Default probabilities for invalid pixels (when using a mask) */
  ProbaType m_DefaultProba;
  int classIndex;

};
} // End namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbClassificationProbabilityFilter.txx"
#endif

#endif
