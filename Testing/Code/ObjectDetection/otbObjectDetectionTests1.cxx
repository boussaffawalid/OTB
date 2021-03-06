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


//
#include "otbTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(otbLabeledSampleLocalizationGeneratorNew);
  REGISTER_TEST(otbLabeledSampleLocalizationGenerator);
  REGISTER_TEST(otbDescriptorsListSampleGeneratorNew);
  REGISTER_TEST(otbDescriptorsListSampleGenerator);
  REGISTER_TEST(otbDescriptorsSVMModelCreation);
  REGISTER_TEST(otbObjectDetectionClassifier);
  REGISTER_TEST(otbObjectDetectionClassifierNew);
  REGISTER_TEST(otbObjectDetectionClassifier);
  REGISTER_TEST(otbStandardMetaImageFunctionBuilderNew);
  REGISTER_TEST(otbStandardMetaImageFunctionBuilder);
}
