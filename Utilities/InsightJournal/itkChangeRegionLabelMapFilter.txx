/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkChangeRegionLabelMapFilter.txx,v $
  Language:  C++
  Date:      $Date: 2004/07/11 14:56:39 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkChangeRegionLabelMapFilter_txx
#define _itkChangeRegionLabelMapFilter_txx
#include "itkChangeRegionLabelMapFilter.h"


namespace itk
{


template <class TInputImage>
void 
ChangeRegionLabelMapFilter<TInputImage>
::GenerateInputRequestedRegion()
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // We need all the input.
  InputImagePointer input = const_cast<InputImageType *>(this->GetInput());
  if ( !input )
    { return; }
  input->SetRequestedRegion( input->GetLargestPossibleRegion() );
}

template <class TInputImage>
void 
ChangeRegionLabelMapFilter<TInputImage>
::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
  this->GetOutput()->SetLargestPossibleRegion( m_Region );
//  std::cout << "ChangeRegionLabelMapFilter::GenerateOutputInformation(): " << this->GetOutput()->GetLargestPossibleRegion() << std::endl;
}

template <class TInputImage>
void 
ChangeRegionLabelMapFilter<TInputImage>
::EnlargeOutputRequestedRegion(DataObject *)
{
  this->GetOutput()
    ->SetRequestedRegion( this->GetOutput()->GetLargestPossibleRegion() );
}

template <class TInputImage>
void 
ChangeRegionLabelMapFilter<TInputImage>
::GenerateData()
{
//  std::cout << "ChangeRegionLabelMapFilter::GenerateData(): " << this->GetOutput()->GetLargestPossibleRegion() << std::endl;

  if( m_Region.IsInside( this->GetInput()->GetLargestPossibleRegion() ) )
    {
    // only copy the image, and do nothing much
    // oh, we have to pretend we have a progress !
    ProgressReporter progress( this, 0, 1 );
    this->AllocateOutputs();
//     std::cout << "do nothing" << std::endl;
    }
  else
    {
    // call the superclass implementation so it will take care to create the threads
    Superclass::GenerateData();
    }
}


template<class TInputImage>
void
ChangeRegionLabelMapFilter<TInputImage>
::ThreadedGenerateData( LabelObjectType * labelObject )
{
  typename InputImageType::LabelObjectType::LineContainerType::const_iterator lit;
  typename InputImageType::LabelObjectType::LineContainerType lineContainer = labelObject->GetLineContainer();
  labelObject->GetLineContainer().clear();
//   std::cout << "lineContainer.size(): " << lineContainer.size() << std::endl;
  
  IndexType idxMin = m_Region.GetIndex();
  IndexType idxMax;
  for( int i=0; i<ImageDimension; i++ )
    {
    idxMax[i] = idxMin[i] + m_Region.GetSize()[i] - 1;
    }
//   std::cout << "idxMin: " << idxMin << std::endl;
//   std::cout << "idxMax: " << idxMax << std::endl;

  for( lit = lineContainer.begin(); lit != lineContainer.end(); lit++ )
    {
    IndexType idx = lit->GetIndex();
    unsigned long length = lit->GetLength();
    
    bool outside = false;
    for( int i=1; i<ImageDimension; i++ )
      {
      if( idx[i] < idxMin[i] || idx[i] > idxMax[i] )
        {
        outside = true;
        }
      }
    // check the axis 0
    if( !outside )
      {
      long lastIdx0 = idx[0] + length - 1;
      if( !( ( idx[0] < idxMin[0] && lastIdx0 < idxMin[0] )
               || ( idx[0] > idxMax[0] && lastIdx0 > idxMax[0] ) ) )
        {
//         std::cout << "!outside" << std::endl;
        IndexType newIdx = idx;
        long newLength = length;
        if( idx[0] < idxMin[0] )
          {
          newLength -= idxMin[0] - idx[0];
          newIdx[0] = idxMin[0];
          }
        if( lastIdx0 > idxMax[0] )
          {
          newLength -= lastIdx0 - idxMax[0];
          }
        
        labelObject->AddLine( newIdx, newLength );
        }
      }
  
    }

  // remove the object if it is empty
  if( labelObject->GetLineContainer().empty() )
    {
//     std::cout << "remove: " << labelObject << std::endl;
    this->m_LabelObjectContainerLock->Lock();
    this->GetOutput()->RemoveLabelObject( labelObject );
    this->m_LabelObjectContainerLock->Unlock();
    }

}

template<class TInputImage>
void 
ChangeRegionLabelMapFilter<TInputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Region: " << m_Region << std::endl;
}


} // end namespace itk

#endif
