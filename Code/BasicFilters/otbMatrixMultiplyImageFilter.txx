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
#ifndef __otbMatrixMultiplyImageFilter_txx
#define __otbMatrixMultiplyImageFilter_txx

#include "otbMatrixMultiplyImageFilter.h"

namespace otb
{

/**
 *
 */
template <class TInputImage, class TOutputImage, class TPrecision>
MatrixMultiplyImageFilter<TInputImage, TOutputImage, TPrecision>
::MatrixMultiplyImageFilter()
{
}

template <class TInputImage, class TOutputImage, class TPrecision>
void
MatrixMultiplyImageFilter<TInputImage, TOutputImage, TPrecision>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

} // end namespace

#endif
