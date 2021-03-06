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

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "otbTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(otbWrapperCommandLineParserNew);
  REGISTER_TEST(otbWrapperCommandLineParserTest1);
  REGISTER_TEST(otbWrapperCommandLineParserTest2);
  REGISTER_TEST(otbWrapperCommandLineParserTest3);
  REGISTER_TEST(otbWrapperCommandLineParserTest4);
  REGISTER_TEST(otbWrapperCommandLineLauncherNew);
  REGISTER_TEST(otbWrapperCommandLineLauncherTest);
}
