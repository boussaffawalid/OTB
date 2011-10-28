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
#include "otbWrapperQtWidgetFloatParameter.h"

namespace otb
{
namespace Wrapper
{

QtWidgetFloatParameter::QtWidgetFloatParameter(FloatParameter* floatParam, QtWidgetModel* m)
: QtWidgetParameterBase(floatParam, m),
  m_FloatParam(floatParam)
{
}

QtWidgetFloatParameter::~QtWidgetFloatParameter()
{
}

void QtWidgetFloatParameter::DoUpdateGUI()
{
  // Update the valid range if updated
  m_QDoubleSpinBox->setRange(m_FloatParam->GetMinimumValue(),
                             m_FloatParam->GetMaximumValue());

  bool signalsBlocked = m_QDoubleSpinBox->blockSignals( true );

  if (m_FloatParam->HasValue())
    {
    m_QDoubleSpinBox->setValue(m_FloatParam->GetValue());
    }
  m_QDoubleSpinBox->blockSignals( signalsBlocked );

  QFont font = m_QDoubleSpinBox->font();
  if (m_FloatParam->HasUserValue())
    {
    font.setBold(true);
    }
  else
    {
    font.setBold(false);
    }
  m_QDoubleSpinBox->setFont(font);
}

void QtWidgetFloatParameter::DoCreateWidget()
{
  m_QHBoxLayout = new QHBoxLayout;
  m_QHBoxLayout->setSpacing(0);
  m_QHBoxLayout->setContentsMargins(0, 0, 0, 0);

  m_QDoubleSpinBox = new QDoubleSpinBox;
  m_QDoubleSpinBox->setDecimals(5);
  m_QDoubleSpinBox->setSingleStep(0.1);
  m_QDoubleSpinBox->setRange(m_FloatParam->GetMinimumValue(), m_FloatParam->GetMaximumValue());
  m_QDoubleSpinBox->setToolTip(m_FloatParam->GetDescription());

  connect( m_QDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SetValue(double)) );
  connect( m_QDoubleSpinBox, SIGNAL(valueChanged(double)), GetModel(), SLOT(NotifyUpdate()) );
  connect( GetModel(), SIGNAL(UpdateGui()), this, SLOT(UpdateGUI() ) );

  m_QHBoxLayout->addWidget(m_QDoubleSpinBox);
  m_QHBoxLayout->addStretch();

  this->setLayout(m_QHBoxLayout);
}

void QtWidgetFloatParameter::SetValue(double value)
{
  m_FloatParam->SetValue( static_cast<float>(value) );
  m_FloatParam->SetUserValue(true);
  m_FloatParam->SetAutomaticValue(false);
}

}
}
