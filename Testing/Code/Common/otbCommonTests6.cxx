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
  REGISTER_TEST(otbDrawPathListFilterNew);
  REGISTER_TEST(otbDrawPathListFilter);
  REGISTER_TEST(otbDrawPathListFilterWithValue);
  REGISTER_TEST(otbPolyLineParametricPathWithValueNew);
  REGISTER_TEST(otbPolyLineImageConstIterator);
  REGISTER_TEST(otbPolyLineImageIterator);
  REGISTER_TEST(otbDrawPathFilterNew);
  REGISTER_TEST(otbDrawPathFilter);
  REGISTER_TEST(otbPolygonNew);
  REGISTER_TEST(otbPolygon);
  REGISTER_TEST(otbUnaryFunctorNeighborhoodImageFilterNew);
  REGISTER_TEST(otbUnaryFunctorNeighborhoodImageFilter);
  REGISTER_TEST(otbRectangleNew);
  REGISTER_TEST(otbRectangle);
}
