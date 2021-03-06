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

// this file defines the otbCommonTest for the test driver
// and all it expects is that you have a function called RegisterTests



#include "otbTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(otbGaborFilterGeneratorNew);
  REGISTER_TEST(otbGaborFilterGenerator);
#if defined(ITK_USE_FFTWD)
  REGISTER_TEST(otbOverlapSaveConvolutionImageFilterNew);
  REGISTER_TEST(otbOverlapSaveConvolutionImageFilter);
  REGISTER_TEST(otbCompareOverlapSaveAndClassicalConvolutionWithGaborFilter);
#endif
  REGISTER_TEST(otbPolygonCompacityFunctor);
  REGISTER_TEST(otbPathLengthFunctor);
  REGISTER_TEST(otbClosePathFunctor);
  REGISTER_TEST(otbPersistentVectorizationFilterNew);
  REGISTER_TEST(otbPersistentVectorizationFilter);
}
