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
#ifndef __otbRandomForestsMachineLearningModel_h
#define __otbRandomForestsMachineLearningModel_h

#include "itkLightObject.h"
#include "itkVariableLengthVector.h"
#include "itkFixedArray.h"
#include "itkListSample.h"
#include "otbMachineLearningModel.h"

//include opencv
#include <cv.h>       // opencv general include file
#include <ml.h>		  // opencv machine learning include file

namespace otb
{
template <class TInputValue, class TTargetValue>
class ITK_EXPORT RandomForestsMachineLearningModel
  : public MachineLearningModel <TInputValue, TTargetValue>
{
public:
  /** Standard class typedefs. */
  typedef RandomForestsMachineLearningModel               Self;
  typedef MachineLearningModel<TInputValue, TTargetValue> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  // Input related typedefs
  typedef TInputValue                                   InputValueType;
  typedef itk::VariableLengthVector<InputValueType>     InputSampleType;
  typedef itk::Statistics::ListSample<InputSampleType>  InputListSampleType;

  // Target related typedefs
  typedef TTargetValue                                  TargetValueType;
  typedef itk::FixedArray<TargetValueType,1>            TargetSampleType;
  typedef itk::Statistics::ListSample<TargetSampleType> TargetListSampleType;

  //opencv typedef
  typedef CvRTrees RFType;

  /** Run-time type information (and related methods). */
  itkNewMacro(Self);
  itkTypeMacro(RandomForestsMachineLearningModel, itk::MachineLearningModel);

  /** Train the machine learning model */
  virtual void Train();

  /** Predict values using the model */
  virtual void Predict();

  /** Save the model to file */
  virtual void Save(char * filename, const char * name=0);

  /** Load the model from file */
  virtual void Load(char * filename, const char * name=0);

  /* /\** Input accessors *\/ */
  /* itkSetObjectMacro(InputListSample,InputListSampleType); */
  /* itkGetObjectMacro(InputListSample,InputListSampleType); */

  /* /\** Target accessors *\/ */
  /* itkSetObjectMacro(TargetListSample,TargetListSampleType); */
  /* itkGetObjectMacro(TargetListSample,TargetListSampleType); */

  //Setters of RT parameters (documentation get from opencv doxygen 2.4)
  /* the depth of the tree. A low value will likely underfit and conversely a
   * high value will likely overfit. The optimal value can be obtained using cross
   * validation or other suitable methods. */
  itkGetMacro(MaxDepth, int);
  /* minimum samples required at a leaf node for it to be split. A reasonable
   * value is a small percentage of the total data e.g. 1%. */
  itkGetMacro(MinSampleCount, int);

  itkGetMacro(RegressionAccuracy, double);
  itkGetMacro(ComputeSurrogateSplit, bool);
  /* Cluster possible values of a categorical variable into K \leq
   * max_categories clusters to find a suboptimal split. If a discrete variable,
   * on which the training procedure tries to make a split, takes more than
   * max_categories values, the precise best subset estimation may take a very
   * long time because the algorithm is exponential. Instead, many decision
   * trees engines (including ML) try to find sub-optimal split in this case by
   * clustering all the samples into max categories clusters that is some
   * categories are merged together. The clustering is applied only in n>2-class
   * classification problems for categorical variables with N > max_categories
   * possible values. In case of regression and 2-class classification the optimal split can be found efficiently without employing clustering, thus the parameter is not used in these cases.
  */
  itkGetMacro(MaxNumberOfCategories, int);

  std::vector<float> GetPriors() const
  {
    return m_Priors;
  }

  void SetPriors(const std::vector<float> & priors)
  {
    m_Priors = priors;
  }
  /* If true then variable importance will be calculated and then it can be
  retrieved by CvRTrees::get_var_importance(). */
  itkGetMacro(CalculateVariableImportance, int);
  /* The size of the randomly selected subset of features at each tree node and
  * that are used to find the best split(s). If you set it to 0 then the size will
  be set to the square root of the total number of features. */
  itkGetMacro(MaxNumberOfVariables, int);
  /* The maximum number of trees in the forest (surprise, surprise). Typically
  * the more trees you have the better the accuracy. However, the improvement in
  * accuracy generally diminishes and asymptotes pass a certain number of
  * trees. Also to keep in mind, the number of tree increases the prediction time
  linearly. */
  itkGetMacro(MaxNumberOfTrees, int);
  /* Sufficient accuracy (OOB error) */
  itkGetMacro(ForestAccuracy, float);
  /* The type of the termination criteria */
  itkGetMacro(TerminationCriteria, int);

  cv::Mat GetVariableImportance()
  {
    return m_RFModel->getVarImportance();
  }

  float GetTrainError()
  {
    return m_RFModel->get_train_error();
  }
protected:
  /** Constructor */
  RandomForestsMachineLearningModel();

  /** Destructor */
  virtual ~RandomForestsMachineLearningModel();

  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /* /\** Input list sample *\/ */
  /* typename InputListSampleType::Pointer m_InputListSample; */

  /* /\** Target list sample *\/ */
  /* typename TargetListSampleType::Pointer m_TargetListSample; */

private:
  RandomForestsMachineLearningModel(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  CvRTrees * m_RFModel;
  int m_MaxDepth;
  int m_MinSampleCount;
  float m_RegressionAccuracy;
  bool m_ComputeSurrogateSplit;
  int m_MaxNumberOfCategories;
  std::vector<float> m_Priors;
  bool m_CalculateVariableImportance;
  int m_MaxNumberOfVariables;
  int m_MaxNumberOfTrees;
  float m_ForestAccuracy;
  int m_TerminationCriteria;
};
} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbRandomForestsMachineLearningModel.txx"
#endif

#endif
