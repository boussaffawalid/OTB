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
#ifndef __otbMachineLearningModel_txx
#define __otbMachineLearningModel_txx

#include "otbMachineLearningModel.h"

namespace otb
{

template <class TInputValue, class TOutputValue>
MachineLearningModel<TInputValue,TOutputValue>
::MachineLearningModel()
{}


template <class TInputValue, class TOutputValue>
MachineLearningModel<TInputValue,TOutputValue>
::~MachineLearningModel()
{}

template <class TInputValue, class TOutputValue>
void
MachineLearningModel<TInputValue,TOutputValue>
::PredictAll()
{
  TargetListSampleType * targets = this->GetTargetListSample();
  targets->Clear();

  for(typename InputListSampleType::ConstIterator sIt = this->GetInputListSample()->Begin();
      sIt!=this->GetInputListSample()->End(); ++sIt)
    {
    targets->PushBack(this->Predict(sIt.GetMeasurementVector()));
    }
}


template <class TInputValue, class TOutputValue>
void
MachineLearningModel<TInputValue,TOutputValue>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  // Call superclass implementation
  Superclass::PrintSelf(os,indent);
}

template <class TInputValue, class TOutputValue>
void
MachineLearningModel<TInputValue,TOutputValue>
::SetNumberOfClasses(const int number) 
{
  m_nbclass = number ;
}

template <class TInputValue, class TOutputValue>
int
MachineLearningModel<TInputValue,TOutputValue>
::GetNumberOfClasses() const 
{
  return m_nbclass;
}

  
template <class TInputValue, class TOutputValue>
typename MachineLearningModel<TInputValue,TOutputValue>
::ProbabilitiesVectorType
MachineLearningModel<TInputValue,TOutputValue>
::GetProbability(const InputSampleType& input) const
{
  ProbabilitiesVectorType proba(m_nbclass);
  //proba.Fill(0);
  //TODO set proba element to 0
  return proba;
}

}

#endif
