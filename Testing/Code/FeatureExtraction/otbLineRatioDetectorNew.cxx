/*=========================================================================

  Programme :   OTB (ORFEO ToolBox)
  Auteurs   :   CS - C.Ruffel
  Language  :   C++
  Date      :   14 mars 2006
  Version   :   
  Role      :   Test du filtre de detection de lignes (instanciation)
  $Id$

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#define MAIN


#include "itkExceptionObject.h"
#include "itkImage.h"
#include <iostream>

#include "otbLineRatioDetectorImageFilter.h"


int otbLineRatioDetectorNew( int argc, char* argv[] )
{
  try 
    {        
        typedef unsigned char                                   InputPixelType;
        typedef double		   	                        OutputPixelType;
        const   unsigned int        	                        Dimension = 2;

        typedef itk::Image< InputPixelType,  Dimension >        InputImageType;
        typedef itk::Image< OutputPixelType, Dimension >        OutputImageType;

        typedef otb::LineRatioDetectorImageFilter< InputImageType, OutputImageType>   FilterType;
	
        FilterType::Pointer FilterLineRatio = FilterType::New();
        

    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cout << "Exception itk::ExceptionObject levee !" << std::endl; 
    std::cout << err << std::endl; 
    return EXIT_FAILURE;
    } 
  catch( ... ) 
    { 
    std::cout << "Exception levee inconnue !" << std::endl; 
    return EXIT_FAILURE;
    } 
  // Software Guide : EndCodeSnippet

//#endif
  return EXIT_SUCCESS;
}


