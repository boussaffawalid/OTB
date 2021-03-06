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
  REGISTER_TEST(otbImageToSIFTKeyPointSetFilterNew);
  REGISTER_TEST(otbImageToSIFTKeyPointSetFilterDistanceMap);
  REGISTER_TEST(otbImageToSIFTKeyPointSetFilterOutputAscii);
  REGISTER_TEST(otbImageToSIFTKeyPointSetFilterOutputImage);
  REGISTER_TEST(otbImageToSIFTKeyPointSetFilterOutputInterestPointAscii);
  REGISTER_TEST(otbImageToSIFTKeyPointSetFilterOutputDescriptorAscii);
  REGISTER_TEST(otbImageToHessianDeterminantImageFilterNew);
  REGISTER_TEST(otbImageToHessianDeterminantImageFilter);
  REGISTER_TEST(otbImageFittingPolygonListFilter);
  REGISTER_TEST(otbImageFittingPolygonListFilterNew);
  REGISTER_TEST(otbSimplePointCountStrategyTest);
}
