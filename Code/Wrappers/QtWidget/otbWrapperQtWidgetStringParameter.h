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
#ifndef __otbWrapperQtWidgetStringParameter_h
#define __otbWrapperQtWidgetStringParameter_h

#include <QtGui>
#include "otbWrapperStringParameter.h"
#include "otbWrapperQtWidgetParameterBase.h"

namespace otb
{
namespace Wrapper
{

/** \class
 * \brief
 */
class QtWidgetStringParameter : public QtWidgetParameterBase
{
  Q_OBJECT
public:
  QtWidgetStringParameter(StringParameter*, QtWidgetModel*);
  virtual ~QtWidgetStringParameter();

protected slots:
  void SetValue( const QString& value );

private:
  QtWidgetStringParameter(const QtWidgetStringParameter&); //purposely not implemented
  void operator=(const QtWidgetStringParameter&); //purposely not implemented

  void CreateWidget();

  StringParameter::Pointer m_StringParam;
};


}
}

#endif
