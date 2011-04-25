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
#include "otbWrapperQtWidgetInputImageParameter.h"

namespace otb
{
namespace Wrapper
{

QtWidgetInputImageParameter::QtWidgetInputImageParameter(InputImageParameter* param, QtWidgetModel* m)
: QtWidgetParameterBase(m),
  m_InputImageParam(param)
{
}

QtWidgetInputImageParameter::~QtWidgetInputImageParameter()
{
}

void QtWidgetInputImageParameter::DoUpdateGUI()
{

}

void QtWidgetInputImageParameter::DoCreateWidget()
{
  // Set up input text edit
  m_HLayout = new QHBoxLayout;
  m_HLayout->setSpacing(0);
  m_HLayout->setContentsMargins(0,0,0,0);
  m_Input = new QLineEdit;
  m_Input->setToolTip( m_InputImageParam->GetDescription() );
  connect( m_Input, SIGNAL(textChanged(const QString&)), this, SLOT(SetFileName(const QString&)) );
  m_HLayout->addWidget(m_Input);

  // Set up input text edit
  m_Button = new QPushButton;
  m_Button->setText("...");
  m_Button->setToolTip("Select file...");
  m_Button->setMaximumWidth(m_Button->width());
  connect( m_Button, SIGNAL(clicked()), this, SLOT(SelectFile()) );
  m_HLayout->addWidget(m_Button);

  this->setLayout(m_HLayout);
}

void QtWidgetInputImageParameter::SelectFile()
{
  QFileDialog fileDialog;
  fileDialog.setConfirmOverwrite(true);
  fileDialog.setFileMode(QFileDialog::ExistingFile);
  fileDialog.setNameFilter("Raster files (*)");

  if (fileDialog.exec())
    {
    //this->SetFileName(fileDialog.selectedFiles().at(0));
    m_Input->setText(fileDialog.selectedFiles().at(0));
    }
}

void QtWidgetInputImageParameter::SetFileName(const QString& value)
{
  // save value
  m_InputImageParam->SetFromFileName(value.toStdString());

  // notify of value change
  QString key( QString::fromStdString(m_InputImageParam->GetKey()) );
  emit ParameterChanged(key);
}

}
}
