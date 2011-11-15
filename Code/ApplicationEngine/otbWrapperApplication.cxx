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
#include "otbWrapperApplication.h"

#include "otbWrapperChoiceParameter.h"
#include "otbWrapperListViewParameter.h"
#include "otbWrapperDirectoryParameter.h"
#include "otbWrapperEmptyParameter.h"
#include "otbWrapperFilenameParameter.h"
#include "otbWrapperInputVectorDataParameter.h"
#include "otbWrapperInputVectorDataListParameter.h"
#include "otbWrapperNumericalParameter.h"
#include "otbWrapperOutputVectorDataParameter.h"
#include "otbWrapperRadiusParameter.h"
#include "otbWrapperStringParameter.h"
#include "otbWrapperStringListParameter.h"
#include "otbWrapperInputImageListParameter.h"
#include "otbWrapperRAMParameter.h"

#include "otbWrapperParameterGroup.h"

#include "otbWrapperAddProcessToWatchEvent.h"


#include "otbWrapperTypes.h"
#include <exception>
#include "itkMacro.h"

namespace otb
{
namespace Wrapper
{

Application::Application()
 : m_Logger(itk::Logger::New())
{
  // Don't call Init from the constructor, since it calls a virtual method !

  m_Logger->SetName("Application.logger");
  m_Logger->SetPriorityLevel(itk::LoggerBase::DEBUG);
  m_Logger->SetLevelForFlushing(itk::LoggerBase::CRITICAL);

}

Application::~Application()
{
}

itk::Logger* Application::GetLogger()
{
  return m_Logger;
}

std::vector<std::string>
Application::GetParametersKeys(bool recursive)
{
  return GetParameterList()->GetParametersKeys(recursive);
}

ParameterGroup* Application::GetParameterList()
{
  if (!m_ParameterList)
    {
    Init();
    }

  return m_ParameterList;
}

Parameter* Application::GetParameterByKey(std::string name)
{
  return GetParameterList()->GetParameterByKey(name);
}

const Parameter* Application::GetParameterByKey(std::string name) const
{
  // GetParameterList is non const...
  Application* _this = const_cast<Application*>(this);
  return _this->GetParameterByKey(name);
}

void Application::Init()
{
  m_ParameterList = ParameterGroup::New();
  this->DoCreateParameters();
}

void Application::UpdateParameters()
{
  this->DoUpdateParameters();
}

int Application::Execute()
{
  int ret = 0;
  
  try
    {
    this->DoExecute();
    }
  catch(std::exception& err)
    {
    ret = 1;
    otbAppLogFATAL(<<"The following error occurred during application execution : " << err.what());
    }
  catch(...)
    {
    ret = 1;
    otbAppLogFATAL(<<"An unknown exception has been raised during application execution");
    }

  return ret;
}

int Application::ExecuteAndWriteOutput()
{
  int status = this->Execute();

  if (status == 0)
    {
    try
      {
      std::vector<std::string> paramList = GetParametersKeys(true);
      // First Get the value of the available memory to use with the
      // writer if a RAMParameter is set
      bool useRAM = false;
      unsigned int ram = 0;
      for (std::vector<std::string>::const_iterator it = paramList.begin();
           it != paramList.end();
           ++it)
        {
        std::string key = *it;

        if (GetParameterType(key) == ParameterType_RAM
          && IsParameterEnabled(key))
          {
          Parameter* param = GetParameterByKey(key);
          RAMParameter* ramParam = dynamic_cast<RAMParameter*>(param);
          ram = ramParam->GetValue();
          useRAM = true;
          }
        }

      for (std::vector<std::string>::const_iterator it = paramList.begin();
           it != paramList.end();
           ++it)
        {
        std::string key = *it;
        if (GetParameterType(key) == ParameterType_OutputImage
            && IsParameterEnabled(key) && HasValue(key) )
          {
          Parameter* param = GetParameterByKey(key);
          OutputImageParameter* outputParam = dynamic_cast<OutputImageParameter*>(param);
          outputParam->InitializeWriters();
          if (useRAM)
            {
            outputParam->SetRAMValue(ram);
            }
          std::ostringstream progressId;
          progressId << "Writing " << outputParam->GetFileName() << "...";
          AddProcess(outputParam->GetWriter(), progressId.str());
          outputParam->Write();
          }
        else if (GetParameterType(key) == ParameterType_OutputVectorData
                 && IsParameterEnabled(key) && HasValue(key) )
          {
          Parameter* param = GetParameterByKey(key);
          OutputVectorDataParameter* outputParam = dynamic_cast<OutputVectorDataParameter*>(param);
          outputParam->InitializeWriters();
          std::ostringstream progressId;
          progressId << "Writing " << outputParam->GetFileName() << "...";
          AddProcess(outputParam->GetWriter(), progressId.str());
          outputParam->Write();
          }
        else if (GetParameterType(key) == ParameterType_ComplexOutputImage
                 && IsParameterEnabled(key) && HasValue(key) )
          {
          Parameter* param = GetParameterByKey(key);
          ComplexOutputImageParameter* outputParam = dynamic_cast<ComplexOutputImageParameter*>(param);
          outputParam->InitializeWriters();
          if (useRAM)
            {
            outputParam->SetRAMValue(ram);
            }
          std::ostringstream progressId;
          progressId << "Writing " << outputParam->GetFileName() << "...";
          AddProcess(outputParam->GetWriter(), progressId.str());
          outputParam->Write();
          }
        }
      }
    catch(std::exception& err)
      {
      status = 1;
      otbAppLogFATAL(<<"The following error occurred when writing outputs : " << err.what());
      }
    catch(...)
      {
      status = 1;
      otbAppLogFATAL(<<"Unknown exception raised when writing outputs");
      }
    }

  return status;
}

/* Enable the use of an optional parameter. Returns the previous state */
void Application::EnableParameter(std::string paramKey)
{
  Parameter* param = GetParameterByKey(paramKey);
  param->SetActive(true);
}

/* Disable the use of an optional parameter. Returns the previous state  */
void Application::DisableParameter(std::string paramKey)
{
  GetParameterByKey(paramKey)->SetActive(false);
}

/* Return the enable state of an optional parameter  */
bool Application::IsParameterEnabled(std::string paramKey) const
{
  return GetParameterByKey(paramKey)->GetActive();
}

/* Return true if the specified parameter is mandatory */
bool Application::IsMandatory(std::string paramKey) const
{
  return GetParameterByKey(paramKey)->GetMandatory();
}

void Application::MandatoryOn(std::string paramKey)
{
  GetParameterByKey(paramKey)->SetMandatory(true);
}

void Application::MandatoryOff(std::string paramKey)
{
  GetParameterByKey(paramKey)->SetMandatory(false);
}

/* Return true if the specified parameter was set automatically in the
 * application
 */
bool Application::HasAutomaticValue(std::string paramKey) const
{
  return GetParameterByKey(paramKey)->GetAutomaticValue();
}

void Application::AutomaticValueOn(std::string paramKey)
{
  GetParameterByKey(paramKey)->SetAutomaticValue(true);
}

void Application::AutomaticValueOff(std::string paramKey)
{
  GetParameterByKey(paramKey)->SetAutomaticValue(false);
}

/* Returns true if the parameter has an associated value provided externally
 *  (not automatically computed by the application) */
bool Application::HasUserValue(std::string paramKey) const
{
  return GetParameterByKey(paramKey)->HasUserValue();
}

/* If a user value was provided clear it and update the other parameters */
void Application::ClearValue(std::string paramKey)
{
  GetParameterByKey(paramKey)->ClearValue();
}

/* Returns true if the parameter has an associated value.
 * This value can be an automatically computed value or default value,
 * or a value set externally by user */
bool Application::HasValue(std::string paramKey) const
{
  return GetParameterByKey(paramKey)->HasValue();
}

/* Return the user level of access to a parameter */
UserLevel Application::GetParameterUserLevel(std::string paramKey) const
{
  return GetParameterByKey(paramKey)->GetUserLevel();
}


/* Return the role (input/output) of a parameter */
Role Application::GetParameterRole(std::string paramKey) const
{
  return GetParameterByKey(paramKey)->GetRole();
}

/* Return the role (input/output) of a parameter */
void Application::SetParameterRole(std::string paramKey, Role role)
{
  GetParameterByKey(paramKey)->SetRole(role);
}

/* Get the parameter type from its name */
ParameterType Application::GetParameterType(std::string paramKey) const
{
  const Parameter* param = GetParameterByKey(paramKey);
  ParameterType type;

  if (dynamic_cast<const ChoiceParameter*>(param))
    {
    type = ParameterType_Choice;
    }
  else if (dynamic_cast<const ListViewParameter*>(param))
    {
    type = ParameterType_ListView;
    }
  else if (dynamic_cast<const RadiusParameter*>(param))
    {
    type = ParameterType_Radius;
    }
  else if (dynamic_cast<const EmptyParameter*>(param))
    {
    type = ParameterType_Empty;
    }
 else if (dynamic_cast<const IntParameter*>(param))
    {
    type = ParameterType_Int;
    }
  else if (dynamic_cast<const FloatParameter*>(param))
    {
    type = ParameterType_Float;
    }
  else if (dynamic_cast<const FilenameParameter*>(param))
    {
    type = ParameterType_Filename;
    }
  else if (dynamic_cast<const DirectoryParameter*>(param))
    {
    type = ParameterType_Directory;
    }
  else if (dynamic_cast<const InputImageParameter*>(param))
    {
    type = ParameterType_InputImage;
    }
 else if (dynamic_cast<const InputImageListParameter*>(param))
    {
    type = ParameterType_InputImageList;
    }
  else if (dynamic_cast<const ComplexInputImageParameter*>(param))
    {
    type = ParameterType_ComplexInputImage;
    }
  else if (dynamic_cast<const InputVectorDataParameter*>(param))
    {
    type = ParameterType_InputVectorData;
    }
  else if (dynamic_cast<const InputVectorDataListParameter*>(param))
    {
    type = ParameterType_InputVectorDataList;
    }
  else if (dynamic_cast<const OutputImageParameter*>(param))
    {
    type = ParameterType_OutputImage;
    }
    else if (dynamic_cast<const ComplexOutputImageParameter*>(param))
    {
    type = ParameterType_ComplexOutputImage;
    }
  else if (dynamic_cast<const OutputVectorDataParameter*>(param))
    {
    type = ParameterType_OutputVectorData;
    }
  else if (dynamic_cast<const StringParameter*>(param))
    {
    type = ParameterType_String;
    }
  else if (dynamic_cast<const StringListParameter*>(param))
    {
    type = ParameterType_StringList;
    }
  else if (dynamic_cast<const RAMParameter*>(param))
    {
    type = ParameterType_RAM;
    }
  else if (dynamic_cast<const ParameterGroup*>(param))
    {
    type = ParameterType_Group;
    }
  else
    {
    itkExceptionMacro(<< "Unknown parameter : " << paramKey);
    }

  return type;
}

std::vector<std::string> Application::GetChoiceKeys(std::string name)
{
  Parameter* param = GetParameterByKey(name);
  if (dynamic_cast<ChoiceParameter*>(param))
    {
    ChoiceParameter* paramChoice = dynamic_cast<ChoiceParameter*>(param);
    return paramChoice->GetChoiceKeys();
    }
  else if (dynamic_cast<ListViewParameter*>(param))
    {
    ListViewParameter* paramList = dynamic_cast<ListViewParameter*>(param);
    return paramList->GetChoiceKeys();
    }
  itkExceptionMacro(<< name << " is not a choice parameter");
}

std::vector<std::string> Application::GetChoiceNames(std::string name)
{
  Parameter* param = GetParameterByKey(name);
  if (dynamic_cast<ChoiceParameter*>(param))
    {
    ChoiceParameter* paramChoice = dynamic_cast<ChoiceParameter*>(param);
    return paramChoice->GetChoiceNames();
    }
  else if (dynamic_cast<ListViewParameter*>(param))
    {
    ListViewParameter* paramList = dynamic_cast<ListViewParameter*>(param);
    return paramList->GetChoiceNames();
    }
  itkExceptionMacro(<< name << " is not a choice parameter");
}

void Application::SetParameterInt(std::string parameter, int value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<IntParameter*>(param))
    {
    IntParameter* paramInt = dynamic_cast<IntParameter*>(param);
    paramInt->SetValue(value);
    }
  else if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
    paramFloat->SetValue(static_cast<float>(value));
    }
  else if (dynamic_cast<RadiusParameter*>(param))
    {
    RadiusParameter* paramRadius = dynamic_cast<RadiusParameter*>(param);
    paramRadius->SetValue(static_cast<unsigned int>(value));
    }
  else if (dynamic_cast<ChoiceParameter*>(param))
    {
    ChoiceParameter* paramChoice = dynamic_cast<ChoiceParameter*>(param);
    paramChoice->SetValue(value);
    }
}

void Application::SetParameterFloat(std::string parameter, float value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
    paramFloat->SetValue(value);
    }
}

void Application::SetDefaultParameterInt(std::string parameter, int value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<RadiusParameter*>(param))
    {
    RadiusParameter* paramRadius = dynamic_cast<RadiusParameter*>(param);
    paramRadius->SetDefaultValue(value);
    paramRadius->SetValue(value);
    }
   else if (dynamic_cast<IntParameter*>(param))
    {
    IntParameter* paramInt = dynamic_cast<IntParameter*>(param);
    paramInt->SetDefaultValue(value);
    paramInt->SetValue(value);
    }
  else if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
    paramFloat->SetDefaultValue(static_cast<float>(value));
    paramFloat->SetValue(static_cast<float>(value));
    }
  else if (dynamic_cast<RAMParameter*>(param))
    {
    RAMParameter* paramRAM = dynamic_cast<RAMParameter*>(param);
    paramRAM->SetDefaultValue(static_cast<unsigned int>(value));
    paramRAM->SetValue(static_cast<unsigned int>(value));
    }
}

void Application::SetDefaultParameterFloat(std::string parameter, float value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
    paramFloat->SetDefaultValue(value);
    paramFloat->SetValue(value);
    }
}

void Application::SetMinimumParameterIntValue(std::string parameter, int value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<IntParameter*>(param))
    {
    IntParameter* paramInt = dynamic_cast<IntParameter*>(param);
    paramInt->SetMinimumValue(value);
    }
 else
    itkExceptionMacro(<<parameter << "parameter can't be casted to int");
}

void Application::SetMaximumParameterIntValue(std::string parameter, int value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<IntParameter*>(param))
    {
    IntParameter* paramInt = dynamic_cast<IntParameter*>(param);
    paramInt->SetMaximumValue(value);
    }
  else
    itkExceptionMacro(<<parameter << "parameter can't be casted to int");
  
}

void Application::SetMinimumParameterFloatValue(std::string parameter, float value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
    paramFloat->SetMinimumValue(value);
    }
 else
    itkExceptionMacro(<<parameter << "parameter can't be casted to float");
}

void Application::SetMaximumParameterFloatValue(std::string parameter, float value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
    paramFloat->SetMaximumValue(value);
    }
  else
    itkExceptionMacro(<<parameter << "parameter can't be casted to float");
  
}


void Application::SetParameterString(std::string parameter, std::string value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<ChoiceParameter*>(param))
    {
    ChoiceParameter* paramDown = dynamic_cast<ChoiceParameter*>(param);
    paramDown->SetValue(value);
    }
  else if (dynamic_cast<ListViewParameter*>(param))
    {
    ListViewParameter* paramDown = dynamic_cast<ListViewParameter*>(param);
    paramDown->SetValue(value);
    }
  else if (dynamic_cast<StringParameter*>(param))
    {
    StringParameter* paramDown = dynamic_cast<StringParameter*>(param);
    paramDown->SetValue(value);
    }
  else if (dynamic_cast<FilenameParameter*>(param))
    {
    FilenameParameter* paramDown = dynamic_cast<FilenameParameter*>(param);
    paramDown->SetValue(value);
    }
  else if (dynamic_cast<DirectoryParameter*>(param))
    {
    DirectoryParameter* paramDown = dynamic_cast<DirectoryParameter*>(param);
    paramDown->SetValue(value);
    }
 else if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramDown = dynamic_cast<FloatParameter*>(param);
    paramDown->SetValue(value);
    }
 else if (dynamic_cast<RadiusParameter*>(param))
    {
    RadiusParameter* paramDown = dynamic_cast<RadiusParameter*>(param);
    paramDown->SetValue(value);
    }
 else if (dynamic_cast<IntParameter*>(param))
    {
    IntParameter* paramDown = dynamic_cast<IntParameter*>(param);
    paramDown->SetValue(value);
    }
  else if (dynamic_cast<InputImageParameter*>(param))
    {
    InputImageParameter* paramDown = dynamic_cast<InputImageParameter*>(param);
    if ( !paramDown->SetFromFileName(value) )
      otbAppLogCRITICAL( <<"Invalid image filename " << value <<".");
      
    }
  else if (dynamic_cast<ComplexInputImageParameter*>(param))
    {
    ComplexInputImageParameter* paramDown = dynamic_cast<ComplexInputImageParameter*>(param);
    paramDown->SetFromFileName(value);
    }
  else if (dynamic_cast<InputVectorDataParameter*>(param))
    {
    InputVectorDataParameter* paramDown = dynamic_cast<InputVectorDataParameter*>(param);
    if ( !paramDown->SetFromFileName(value) )
      otbAppLogCRITICAL( <<"Invalid vector data filename " << value <<".");
    }
  else if (dynamic_cast<OutputImageParameter*>(param))
    {
    OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
    paramDown->SetFileName(value);
    }
  else if (dynamic_cast<ComplexOutputImageParameter*>(param))
    {
    ComplexOutputImageParameter* paramDown = dynamic_cast<ComplexOutputImageParameter*>(param);
    paramDown->SetFileName(value);
    }
  else if (dynamic_cast<OutputVectorDataParameter*>(param))
    {
    OutputVectorDataParameter* paramDown = dynamic_cast<OutputVectorDataParameter*>(param);
    paramDown->SetFileName(value);
    }
  else if (dynamic_cast<RAMParameter*>(param))
    {
    RAMParameter* paramDown = dynamic_cast<RAMParameter*>(param);
    paramDown->SetValue(value);
    }
}

void Application::SetParameterStringList(std::string parameter, std::vector<std::string> value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<InputImageListParameter*>(param))
    {
    InputImageListParameter* paramDown = dynamic_cast<InputImageListParameter*>(param);
    if( !paramDown->SetListFromFileName(value) )
    otbAppLogCRITICAL( <<"At least one image filename is invalid.");
    }
  else if (dynamic_cast<InputVectorDataListParameter*>(param))
     {
     InputVectorDataListParameter* paramDown = dynamic_cast<InputVectorDataListParameter*>(param);
     if( !paramDown->SetListFromFileName(value)  )
       otbAppLogCRITICAL( <<"At least one vector data filename is invalid..");
     }
  else if (dynamic_cast<StringListParameter*>(param))
    {
    StringListParameter* paramDown = dynamic_cast<StringListParameter*>(param);
    paramDown->SetValue(value);
    }
}

void Application::SetParameterOutputImage(std::string parameter, FloatVectorImageType* value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<OutputImageParameter*>(param))
    {
    OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
    paramDown->SetValue(value);
    }
}

void Application::SetParameterComplexOutputImage(std::string parameter, ComplexFloatVectorImageType* value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<ComplexOutputImageParameter*>(param))
    {
    ComplexOutputImageParameter* paramDown = dynamic_cast<ComplexOutputImageParameter*>(param);
    paramDown->SetValue(value);
    }
}

void Application::SetParameterOutputImagePixelType(std::string parameter, ImagePixelType pixelType)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<OutputImageParameter*>(param))
    {
    OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
    paramDown->SetPixelType(pixelType);
    }
}

void Application::SetParameterComplexOutputImagePixelType(std::string parameter,
                                                          ComplexImagePixelType cpixelType)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<ComplexOutputImageParameter*>(param))
    {
    ComplexOutputImageParameter* paramDown = dynamic_cast<ComplexOutputImageParameter*>(param);
    paramDown->SetComplexPixelType(cpixelType);
    }
}
void Application::SetParameterOutputVectorData(std::string parameter, VectorDataType* value)
{
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<OutputVectorDataParameter*>(param))
    {
    OutputVectorDataParameter* paramDown = dynamic_cast<OutputVectorDataParameter*>(param);
    paramDown->SetValue(value);
    }
}

std::string Application::GetParameterName(std::string parameter)
{
  Parameter* param = GetParameterByKey(parameter);
  return param->GetName();
}

std::string Application::GetParameterDescription(std::string parameter)
{
  Parameter* param = GetParameterByKey(parameter);
  return param->GetDescription();
}

void Application::SetParameterDescription(std::string parameter, std::string desc)
{
  Parameter* param = GetParameterByKey(parameter);
  param->SetDescription(desc);
}

int Application::GetParameterInt(std::string parameter)
{
  int ret = 0;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<IntParameter*>(param))
    {
    IntParameter* paramInt = dynamic_cast<IntParameter*>(param);
    ret = paramInt->GetValue();
    }
  else if (dynamic_cast<FloatParameter*>(param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
    ret = static_cast<int>(paramFloat->GetValue());
    }
  else if (dynamic_cast<RadiusParameter*>(param))
    {
    RadiusParameter* paramRadius = dynamic_cast<RadiusParameter*>(param);
    ret = paramRadius->GetValue();
    }
  else if (dynamic_cast<RAMParameter*>(param))
    {
    RAMParameter* paramRAM = dynamic_cast<RAMParameter*>(param);
    ret = paramRAM->GetValue();
    }
  else if (dynamic_cast<ChoiceParameter*>(param))
    {
    ChoiceParameter* paramChoice = dynamic_cast<ChoiceParameter*>(param);
    ret = paramChoice->GetValue();
    }
  else
    {
     itkExceptionMacro(<<parameter << "parameter can't be casted to int");
    }

  return ret;
}

float Application::GetParameterFloat(std::string parameter)
{
  float ret = 0.0;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<FloatParameter*> (param))
    {
    FloatParameter* paramFloat = dynamic_cast<FloatParameter*> (param);
    ret = paramFloat->GetValue();
    }
  else
    {
    itkExceptionMacro(<<parameter << "parameter can't be casted to float");
    }

  return ret;
}

std::string Application::GetParameterString(std::string parameter)
{
  std::string ret="";
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<ChoiceParameter*>(param))
    {
    ChoiceParameter* paramDown = dynamic_cast<ChoiceParameter*>(param);
    ret = paramDown->GetChoiceName( paramDown->GetValue() );
    }
  else if (dynamic_cast<ListViewParameter*>(param))
    {
    ListViewParameter* paramDown = dynamic_cast<ListViewParameter*>(param);
    ret = paramDown->GetChoiceName( paramDown->GetValue() );
    }
  else if (dynamic_cast<StringParameter*>(param))
    {
    StringParameter* paramDown = dynamic_cast<StringParameter*>(param);
    ret = paramDown->GetValue();
    }
  else if (dynamic_cast<FilenameParameter*>(param))
    {
    FilenameParameter* paramDown = dynamic_cast<FilenameParameter*>(param);
    ret = paramDown->GetValue();
    }
  else if (dynamic_cast<DirectoryParameter*>(param))
    {
    DirectoryParameter* paramDown = dynamic_cast<DirectoryParameter*>(param);
    ret = paramDown->GetValue();
    }
  else if (dynamic_cast<InputImageParameter*>(param))
    {
    InputImageParameter* paramDown = dynamic_cast<InputImageParameter*>(param);
    ret = paramDown->GetFileName();
    }
  else if (dynamic_cast<ComplexInputImageParameter*>(param))
    {
    ComplexInputImageParameter* paramDown = dynamic_cast<ComplexInputImageParameter*>(param);
    ret = paramDown->GetFileName();
    }
  else if (dynamic_cast<InputVectorDataParameter*>(param))
    {
    InputVectorDataParameter* paramDown = dynamic_cast<InputVectorDataParameter*>(param);
    ret = paramDown->GetFileName();
    }
  else if (dynamic_cast<OutputImageParameter*>(param))
    {
    OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
    ret = paramDown->GetFileName();
    }
  else if (dynamic_cast<OutputVectorDataParameter*>(param))
    {
    OutputVectorDataParameter* paramDown = dynamic_cast<OutputVectorDataParameter*>(param);
    ret = paramDown->GetFileName();
    }
  else
   {
    itkExceptionMacro(<<parameter << "parameter can't be casted to string");
    }

  return ret;
}

std::vector<std::string> Application::GetParameterStringList(std::string parameter)
{
  std::vector<std::string> ret;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<InputImageListParameter*> (param))
    {
    InputImageListParameter* paramDown = dynamic_cast<InputImageListParameter*> (param);
    ret = paramDown->GetFileNameList();
    }
  else
    if (dynamic_cast<InputVectorDataListParameter*> (param))
      {
      InputVectorDataListParameter* paramDown = dynamic_cast<InputVectorDataListParameter*> (param);
      ret = paramDown->GetFileNameList();
      }
    else
      if (dynamic_cast<StringListParameter*> (param))
        {
        StringListParameter* paramDown = dynamic_cast<StringListParameter*> (param);
        ret = paramDown->GetValue();
        }
      else
       {
            itkExceptionMacro(<<parameter << "parameter can't be casted to StringList");
       }

  return ret;
}


FloatVectorImageType* Application::GetParameterImage(std::string parameter)
{
  FloatVectorImageType::Pointer ret = NULL;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<InputImageParameter*> (param))
    {
    InputImageParameter* paramDown = dynamic_cast<InputImageParameter*> (param);
    ret = paramDown->GetImage();
    }
  else
    {
    itkExceptionMacro(<<parameter << "parameter can't be casted to ImageType");
    }

  return ret;
}

FloatVectorImageListType* Application::GetParameterImageList(std::string parameter)
{
  FloatVectorImageListType::Pointer ret=NULL;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<InputImageListParameter*>(param))
    {
    InputImageListParameter* paramDown = dynamic_cast<InputImageListParameter*>(param);
    ret = paramDown->GetImageList();
    }
  else
    {
    itkExceptionMacro(<<parameter << "parameter can't be casted to ImageListType");
    }

  return ret;
}

ComplexFloatVectorImageType* Application::GetParameterComplexImage(std::string parameter)
{
  ComplexFloatVectorImageType::Pointer ret=NULL;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<ComplexInputImageParameter*>(param))
    {
    ComplexInputImageParameter* paramDown = dynamic_cast<ComplexInputImageParameter*>(param);
    ret = paramDown->GetImage();
    }
  else
    {
    itkExceptionMacro(<<parameter << "parameter can't be casted to ComplexImageType");
    }

  return ret;
}

VectorDataType* Application::GetParameterVectorData(std::string parameter)
{
  VectorDataType::Pointer ret=NULL;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<InputVectorDataParameter*>(param))
    {
    InputVectorDataParameter* paramDown = dynamic_cast<InputVectorDataParameter*>(param);
    ret = paramDown->GetVectorData();
    }
  else
    {
    itkExceptionMacro(<<parameter << "parameter can't be casted to Vector Data");
    }
  return ret;
}

VectorDataListType* Application::GetParameterVectorDataList(std::string parameter)
{
  VectorDataListType::Pointer ret=NULL;
  Parameter* param = GetParameterByKey(parameter);

  if (dynamic_cast<InputVectorDataListParameter*>(param))
    {
    InputVectorDataListParameter* paramDown = dynamic_cast<InputVectorDataListParameter*>(param);
    ret = paramDown->GetVectorDataList();
    }
  else
   {
    itkExceptionMacro(<<parameter << "parameter can't be casted to Vector Data List");
   }
  return ret;
}


std::string Application::GetParameterAsString(std::string paramKey)
{
  std::string ret="";
  ParameterType type = this->GetParameterType( paramKey );

  if( type == ParameterType_String || type == ParameterType_Filename
      || type == ParameterType_Directory || type == ParameterType_InputImage
      || type == ParameterType_ComplexInputImage || type == ParameterType_InputVectorData
      || type == ParameterType_OutputImage || type == ParameterType_OutputVectorData
      || type == ParameterType_ListView)
    {
      ret = this->GetParameterString( paramKey );
    }
  else if ( type == ParameterType_Int || type == ParameterType_Radius || type == ParameterType_Choice
            || type == ParameterType_RAM)
    {
      std::ostringstream oss;
      oss << this->GetParameterInt( paramKey );
      ret = oss.str();
    }
  else if( type == ParameterType_Float )
    {
      std::ostringstream oss;
      oss << this->GetParameterFloat( paramKey );
      ret = oss.str();
    }
  else
    {
      itkExceptionMacro(<<paramKey << "parameter can't be casted to string");
    }
   return ret;
}

ImagePixelType Application::GetParameterOutputImagePixelType(std::string parameter)
{
  Parameter* param = GetParameterByKey(parameter);
  ImagePixelType ret=ImagePixelType_int8; //by default to avoid warning

  if (dynamic_cast<OutputImageParameter*>(param))
    {
    OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
    ret = paramDown->GetPixelType();
    }
  else
    {
    itkExceptionMacro("Unable to find PixelType in parameter "<<parameter<<".");
    }

  return ret;
}

ComplexImagePixelType Application::GetParameterComplexOutputImagePixelType(std::string parameter)
{
  Parameter* param = GetParameterByKey(parameter);
  ComplexImagePixelType ret=ComplexImagePixelType_float;  //by default to avoid warning

  if (dynamic_cast<ComplexOutputImageParameter*>(param))
    {
    ComplexOutputImageParameter* paramDown = dynamic_cast<ComplexOutputImageParameter*>(param);
    ret = paramDown->GetComplexPixelType();
    }
  else
    {
    itkExceptionMacro("Unable to find PixelType in parameter "<<parameter<<".");
    }

  return ret;
}


void
Application::AddChoice(std::string paramKey, std::string paramName)
{
  GetParameterList()->AddChoice(paramKey, paramName);
}

void
Application::ClearChoices(std::string paramKey)
{
  GetParameterList()->ClearChoices(paramKey);
}

std::vector<int>
Application::GetSelectedItems(std::string param)
{
  return GetParameterList()->GetSelectedItems(param);
}

void
Application::AddParameter(ParameterType type, std::string paramKey, std::string paramName)
{
  GetParameterList()->AddParameter(type, paramKey, paramName);
}

void Application::AddRAMParameter(std::string paramKey, std::string paramName, unsigned int defaultValue)
{
  GetParameterList()->AddParameter(ParameterType_RAM, paramKey, paramName);
  SetDefaultParameterInt(paramKey, defaultValue);
}

void Application::AddRAMParameter()
{
  // Get the  RAM Parameter from the configuration file
  if (otb::ConfigurationFile::GetInstance()->IsValid() )
    {
    AddRAMParameter("ram",
                    "Available RAM",
                    otb::ConfigurationFile::GetInstance()->GetAvailableRAMInMBytes());
    }
  else
    {
    // TODO check this
    AddRAMParameter("ram",
                    "Available RAM",
                    128);
    }
}

std::vector< std::pair<std::string, std::string> >
Application::GetOutputParametersSumUp()
{
  std::vector< std::pair<std::string, std::string> > res;
  std::vector<std::string> paramList = GetParametersKeys(true);
  for (std::vector<std::string>::const_iterator it = paramList.begin();
       it != paramList.end();
       ++it)
    {
    Parameter* param = GetParameterByKey(*it);
    ParameterType type = GetParameterType(*it);
    
    if ( type != ParameterType_Group )
      {
      if ( param->GetRole() == Role_Output )
        {
        std::pair<std::string, std::string> keyVal;
        keyVal.first = (*it);
        itk::OStringStream oss;
        if (dynamic_cast<ChoiceParameter*>(param))
          {
          ChoiceParameter* paramDown = dynamic_cast<ChoiceParameter*>(param);
          oss << paramDown->GetChoiceName( paramDown->GetValue() );
          }
        else if (dynamic_cast<ListViewParameter*>(param))
          {
          ListViewParameter* paramDown = dynamic_cast<ListViewParameter*>(param);
          oss << paramDown->GetChoiceName( paramDown->GetValue() );
          }
        else if (dynamic_cast<StringParameter*>(param))
          {
          StringParameter* paramDown = dynamic_cast<StringParameter*>(param);
          oss << paramDown->GetValue();
          }
        else if (dynamic_cast<FilenameParameter*>(param))
          {
          FilenameParameter* paramDown = dynamic_cast<FilenameParameter*>(param);
          oss << paramDown->GetValue();
          }
        else if (dynamic_cast<DirectoryParameter*>(param))
          {
          DirectoryParameter* paramDown = dynamic_cast<DirectoryParameter*>(param);
          oss << paramDown->GetValue();
          }
        else if (dynamic_cast<InputImageParameter*>(param))
          {
          InputImageParameter* paramDown = dynamic_cast<InputImageParameter*>(param);
          oss << paramDown->GetFileName();
          }
        else if (dynamic_cast<ComplexInputImageParameter*>(param))
          {
          ComplexInputImageParameter* paramDown = dynamic_cast<ComplexInputImageParameter*>(param);
          oss << paramDown->GetFileName();
          }
        else if (dynamic_cast<InputVectorDataParameter*>(param))
          {
          InputVectorDataParameter* paramDown = dynamic_cast<InputVectorDataParameter*>(param);
          oss << paramDown->GetFileName();
          }
        else if (dynamic_cast<OutputImageParameter*>(param))
          {
          OutputImageParameter* paramDown = dynamic_cast<OutputImageParameter*>(param);
          oss << paramDown->GetFileName();
          }
        else if (dynamic_cast<OutputVectorDataParameter*>(param))
          {
          OutputVectorDataParameter* paramDown = dynamic_cast<OutputVectorDataParameter*>(param);
          oss << paramDown->GetFileName();
          }
        if (dynamic_cast<IntParameter*>(param))
          {
          IntParameter* paramInt = dynamic_cast<IntParameter*>(param);
          oss << paramInt->GetValue();
          }
        else if (dynamic_cast<FloatParameter*>(param))
          {
          FloatParameter* paramFloat = dynamic_cast<FloatParameter*>(param);
          oss << static_cast<int>(paramFloat->GetValue());
          }
        else if (dynamic_cast<RadiusParameter*>(param))
          {
          RadiusParameter* paramRadius = dynamic_cast<RadiusParameter*>(param);
          oss << paramRadius->GetValue();
          }
        else
          {
          itkExceptionMacro(<<param << "parameter type unknown");
          }
        
        keyVal.second = oss.str();
        res.push_back( keyVal );
        }
      }
    }
  return res;
}

bool
Application::IsApplicationReady()
{
  // Check if all the mandatory parameters are set
  bool ready = true;

  std::vector<std::string> paramList = GetParametersKeys(true);
  for (std::vector<std::string>::const_iterator it = paramList.begin();
       it != paramList.end();
       ++it)
    {
    // Check all Input Parameters with Input Role
    if (GetParameterByKey(*it)->GetRole() == Role_Input)
      {
      // When a parameter is mandatory :
      // return false when does not have value and:
      //  - The param is root
      //  - The param is not root and belonging to a Mandatory Group
      //    wich is activated
      if ( !this->HasValue(*it)  && IsMandatory(*it) )
        if( GetParameterByKey(*it)->IsRoot() || GetParameterByKey(*it)->GetRoot()->GetActive() )
          {
          return false;
          }
      }
    }

  return ready;
}

void
Application::AddProcess(itk::ProcessObject* object, std::string description)
{
  m_ProgressSource = object;
  m_ProgressSourceDescription = description;

  AddProcessToWatchEvent event;
  event.SetProcess(object);
  event.SetProcessDescription(description);
  this->InvokeEvent(event);
}

itk::ProcessObject* Application::GetProgressSource() const
{
  return m_ProgressSource;
}

std::string Application::GetProgressDescription() const
{
  return m_ProgressSourceDescription;
}


}
}
