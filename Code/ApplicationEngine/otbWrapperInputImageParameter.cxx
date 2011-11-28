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
#include "otbWrapperInputImageParameter.h"
#include "itksys/SystemTools.hxx"
#include "otbImageFileReader.h"
#include "itkCastImageFilter.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbWrapperTypes.h"

namespace otb
{
namespace Wrapper
{

InputImageParameter::InputImageParameter()
{
  this->SetName("Input Image");
  this->SetKey("in");
  m_FileName="";
  m_PreviousFileName="";
  this->ClearValue();
}

InputImageParameter::~InputImageParameter()
{
}

bool
InputImageParameter::SetFromFileName(const std::string& filename)
{
  // First clear previous file choosen
  this->ClearValue();

  // TODO : when the logger will be available, redirect the exception
  // in the logger (like what is done in MsgReporter)
  if (!filename.empty()
      && itksys::SystemTools::FileExists(filename.c_str()))
    {
    FloatVectorReaderType::Pointer reader = FloatVectorReaderType::New();
    reader->SetFileName(filename);
    try
      {
      reader->UpdateOutputInformation();
      }
    catch(itk::ExceptionObject & /*err*/)
      {
      return false;
      }

    // the specified filename is valid => store the value
    m_FileName = filename;
    SetActive(true);
    return true;
    }
  return false;
}


FloatVectorImageType*
InputImageParameter::GetImage()
{
  return this->GetImage<FloatVectorImageType>();
}


#define otbGetImageMacro(image)                       \
  image##Type *                                       \
  InputImageParameter::Get##image ()                  \
  {                                                   \
    return this->GetImage< image##Type > ();          \
  }

otbGetImageMacro(UInt8Image)
otbGetImageMacro(Int8Image);
otbGetImageMacro(UInt16Image);
otbGetImageMacro(Int16Image);
otbGetImageMacro(UInt32Image);
otbGetImageMacro(Int32Image);
otbGetImageMacro(FloatImage);
otbGetImageMacro(DoubleImage);

otbGetImageMacro(UInt8VectorImage);
otbGetImageMacro(Int8VectorImage);
otbGetImageMacro(UInt16VectorImage);
otbGetImageMacro(Int16VectorImage);
otbGetImageMacro(UInt32VectorImage);
otbGetImageMacro(Int32VectorImage);
otbGetImageMacro(FloatVectorImage);
otbGetImageMacro(DoubleVectorImage);

otbGetImageMacro(UInt8RGBImage);
otbGetImageMacro(UInt8RGBAImage);


template <class TOutputImage>
TOutputImage *
InputImageParameter::GetImage()
{
  // Used m_PreviousFileName because if not, when the user call twice GetImage,
  // it without changing the filename, it returns 2 different
  // image pointers
  // TODO : how do we deal with 2 GetImage with 2 defferent template???
  // Should be handled by the CastImage< > macro
  
  // 2 cases : the user set a filename vs. the user set an image
  if( m_PreviousFileName!=m_FileName && !m_FileName.empty() )
    {
    //////////////////////// Filename case:
    m_PreviousFileName = m_FileName;
    typedef otb::ImageFileReader<TOutputImage> ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(m_FileName);
    try
      {
      reader->UpdateOutputInformation();
      }
    catch (itk::ExceptionObject &)
      {
      this->ClearValue();
      }
    
    m_Image = reader->GetOutput();
    m_Reader = reader;
    
    // Pay attention, don't return m_Image because it is a ImageBase...
    return reader->GetOutput();
    }
  else
    {
    //////////////////////// Image case:
    if (m_Image.IsNull())
      {
      itkExceptionMacro("No input image or filename detected...");
      }
    else
      {
      if (dynamic_cast<Int8ImageType*> (m_Image.GetPointer()))
        {
        return CastImage<Int8ImageType, TOutputImage> ();
        }
      else if (dynamic_cast<UInt8ImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt8ImageType, TOutputImage> ();
        }
      else if (dynamic_cast<Int16ImageType*> (m_Image.GetPointer()))
        {
        return CastImage<Int16ImageType, TOutputImage> ();
        }
      else if (dynamic_cast<UInt16ImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt16ImageType, TOutputImage> ();
        }
      else if (dynamic_cast<Int32ImageType*> (m_Image.GetPointer()))
        {
        return CastImage<Int32ImageType, TOutputImage> ();
        }
      else if (dynamic_cast<UInt32ImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt32ImageType, TOutputImage> ();
        }
      else if (dynamic_cast<FloatImageType*> (m_Image.GetPointer()))
        {
        return CastImage<FloatImageType, TOutputImage> ();
        }
      else if (dynamic_cast<DoubleImageType*> (m_Image.GetPointer()))
        {
        return CastImage<DoubleImageType, TOutputImage> ();
        }
      else if (dynamic_cast<Int8VectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<Int8VectorImageType, TOutputImage> ();
        }
      else if (dynamic_cast<UInt8VectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt8VectorImageType, TOutputImage> ();
        }
      else if (dynamic_cast<Int16VectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<Int16VectorImageType, TOutputImage> ();
        }
      else if (dynamic_cast<UInt16VectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt16VectorImageType, TOutputImage> ();
        }
      else if (dynamic_cast<Int32VectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<Int32VectorImageType, TOutputImage> ();
        }
      else if (dynamic_cast<UInt32VectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt32VectorImageType, TOutputImage> ();
        }
      else if (dynamic_cast<FloatVectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<FloatVectorImageType, TOutputImage> ();
        }
      else if (dynamic_cast<DoubleVectorImageType*> (m_Image.GetPointer()))
        {
        return CastImage<DoubleVectorImageType, TOutputImage> ();
        }
    
      else if (dynamic_cast<UInt8RGBAImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt8RGBAImageType, TOutputImage> ();
        }
      else if (dynamic_cast<UInt8RGBImageType*> (m_Image.GetPointer()))
        {
        return CastImage<UInt8RGBImageType, TOutputImage> ();
        }
      else
        {
        itkExceptionMacro("Unknown image type");
        }
      }
    }
    
}


template <class TInputImage, class TOutputImage>
TOutputImage*
InputImageParameter::SimpleCastImage()
{
  TInputImage* realInputImage = dynamic_cast<TInputImage*>(m_Image.GetPointer());
  
  typedef itk::CastImageFilter<TInputImage, TOutputImage> CasterType;
  typename CasterType::Pointer caster = CasterType::New();
  
  caster->SetInput(realInputImage);
  caster->UpdateOutputInformation();
  
  m_Image = caster->GetOutput();
  m_Caster = caster;
  
  return caster->GetOutput();
}
  
                       
template <class TInputImage, class TOutputImage>
TOutputImage*
InputImageParameter::CastVectorImageFromImage()
{
  TInputImage* realInputImage = dynamic_cast<TInputImage*>(m_Image.GetPointer());
  
  typedef ImageToVectorImageCastFilter<TInputImage, TOutputImage> CasterType;
  typename CasterType::Pointer caster = CasterType::New();
  
  caster->SetInput(realInputImage);
  caster->UpdateOutputInformation();
  
  m_Image = caster->GetOutput();
  m_Caster = caster;
  
  return caster->GetOutput();
}


#define otbCastImageMacro(InputImageType, OutputImageType, theMethod)   \
  template<> OutputImageType *                                          \
  InputImageParameter::CastImage<InputImageType , OutputImageType>()    \
  {                                                                     \
    return this->theMethod<InputImageType , OutputImageType>();         \
  }

#define otbGenericCastImageMacro(InputImageType, theMethod, prefix)     \
  otbCastImageMacro(InputImageType, UInt8##prefix##ImageType, theMethod) \
  otbCastImageMacro(InputImageType, Int8##prefix##ImageType, theMethod) \
  otbCastImageMacro(InputImageType, UInt16##prefix##ImageType, theMethod) \
  otbCastImageMacro(InputImageType, Int16##prefix##ImageType, theMethod) \
  otbCastImageMacro(InputImageType, UInt32##prefix##ImageType, theMethod) \
  otbCastImageMacro(InputImageType, Int32##prefix##ImageType, theMethod) \
  otbCastImageMacro(InputImageType, Float##prefix##ImageType, theMethod) \
  otbCastImageMacro(InputImageType, Double##prefix##ImageType, theMethod)


/*********************************************************************
********************** Image -> Image
**********************************************************************/
otbCastImageMacro(Int8ImageType, Int8ImageType, SimpleCastImage )

otbGenericCastImageMacro(UInt8ImageType, SimpleCastImage, )
otbGenericCastImageMacro(Int16ImageType, SimpleCastImage, )
otbGenericCastImageMacro(UInt16ImageType, SimpleCastImage, )
otbGenericCastImageMacro(Int32ImageType, SimpleCastImage, )
otbGenericCastImageMacro(UInt32ImageType, SimpleCastImage, )
otbGenericCastImageMacro(FloatImageType, SimpleCastImage, )
otbGenericCastImageMacro(DoubleImageType, SimpleCastImage, )


/*********************************************************************
********************** VectorImage -> VectorImage
**********************************************************************/
otbGenericCastImageMacro(Int8VectorImageType, SimpleCastImage, Vector)
otbGenericCastImageMacro(UInt8VectorImageType, SimpleCastImage, Vector)
otbGenericCastImageMacro(Int16VectorImageType, SimpleCastImage, Vector)
otbGenericCastImageMacro(UInt16VectorImageType, SimpleCastImage, Vector)
otbGenericCastImageMacro(Int32VectorImageType, SimpleCastImage, Vector)
otbGenericCastImageMacro(UInt32VectorImageType, SimpleCastImage, Vector)
otbGenericCastImageMacro(FloatVectorImageType, SimpleCastImage, Vector)
otbGenericCastImageMacro(DoubleVectorImageType, SimpleCastImage, Vector)


/*********************************************************************
********************** Image -> VectorImage
**********************************************************************/
otbGenericCastImageMacro(Int8ImageType, CastVectorImageFromImage, Vector)
otbGenericCastImageMacro(UInt8ImageType, CastVectorImageFromImage, Vector)
otbGenericCastImageMacro(Int16ImageType, CastVectorImageFromImage, Vector)
otbGenericCastImageMacro(UInt16ImageType, CastVectorImageFromImage, Vector)
otbGenericCastImageMacro(Int32ImageType, CastVectorImageFromImage, Vector)
otbGenericCastImageMacro(UInt32ImageType, CastVectorImageFromImage, Vector)
otbGenericCastImageMacro(FloatImageType, CastVectorImageFromImage, Vector)
otbGenericCastImageMacro(DoubleImageType, CastVectorImageFromImage, Vector)


void
InputImageParameter::SetImage(FloatVectorImageType* image)
{
  this->SetImage<FloatVectorImageType>( image );
}


template <class TInputImage>
void
InputImageParameter::SetImage(TInputImage* image)
{
  m_Image = image;
}


bool
InputImageParameter::HasValue() const
{
  if( m_FileName.empty() && m_Image.IsNull() )
    return false;
  else
    return true;
}

void
InputImageParameter::ClearValue()
{
 m_Image  = NULL;
 m_Reader = NULL;
 m_Caster = NULL;
 m_FileName = "";
 m_PreviousFileName="";
}


}
}

