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
#include "otbImageViewerManagerModel.h"
#include "otbFltkFilterWatcher.h"
#include <FL/fl_ask.H>
#include "itkExceptionObject.h"
#include "otbMacro.h"

#include "otbImageFileWriter.h"
#include "otbFltkFilterWatcher.h"


namespace otb
{

/** Initialize the singleton */
ImageViewerManagerModel::Pointer ImageViewerManagerModel::Instance = NULL;

ImageViewerManagerModel::ImageViewerManagerModel()
{
  //Set all the boolean to false
  m_HasChangedChannelOrder = false;
  m_HasImageOpened = false;
}

ImageViewerManagerModel
::~ImageViewerManagerModel(){}



/** Manage the singleton */
ImageViewerManagerModel::Pointer
ImageViewerManagerModel::GetInstance()
{
  if (!Instance)
  {
    Instance = ImageViewerManagerModel::New();
  }
  return Instance;
}

void
ImageViewerManagerModel
::Notify(ImageViewerManagerEventsListener * listener)
{
  listener->ImageViewerManagerNotify();
}

void
ImageViewerManagerModel
::OpenImage(std::string filename)
{
  /** Reader*/
  ReaderPointerType  reader = ReaderType::New();
  reader->SetFileName(filename);
  reader->GenerateOutputInformation();

  /** Generate the layer*/
  LayerGeneratorPointerType visuGenerator = LayerGeneratorType::New();
  visuGenerator->SetImage(reader->GetOutput());
  FltkFilterWatcher qlwatcher(visuGenerator->GetResampler(),0,0,200,20,"Generating QuickLook ...");
  visuGenerator->GenerateLayer();
  StandardRenderingFunctionType::Pointer  rendrerFuntion  = visuGenerator->GetDefaultRenderingFunction();

  /** Rendering image*/
  VisuModelPointerType rendering = VisuModelType::New();
  rendering->AddLayer(visuGenerator->GetLayer());

  rendering->Update();

  /** View*/
  VisuViewPointerType visuView = this->BuiltVisu(rendering);

  /** Build the pixelDescription View*/
  PixelDescriptionViewType::Pointer pixelView = PixelDescriptionViewType::New();
  PixelDescriptionModelPointerType pixelModel = PixelDescriptionModelType::New();
  pixelModel->SetLayers(rendering->GetLayers());
  pixelView->SetModel(pixelModel);

  /** Controller*/
  WidgetControllerPointerType controller = this->BuiltController(rendering, visuView ,pixelModel );

  /** Finish Builting the visu*/
  visuView->SetController(controller);

  /** Build the curve Widget */
  CurvesWidgetType::Pointer   curveWidget = CurvesWidgetType::New();

  /** Store all the information in the structure*/
  ObjectsTracked currentComponent;

  currentComponent.fileName   = filename;
  currentComponent.pLayer     = visuGenerator->GetLayer();
  currentComponent.pReader    = reader;
  currentComponent.pRendering = rendering;
  currentComponent.pVisuView  = visuView;
  currentComponent.pWidgetController = controller;
  currentComponent.pRenderFuntion  = rendrerFuntion;
  currentComponent.pPixelView   = pixelView;
  currentComponent.pPixelModel  = pixelModel;
  currentComponent.pCurveWidget = curveWidget;

  /** Add the the struct in the list*/
  m_ObjectTrackedList.push_back(currentComponent);

  m_HasImageOpened = true;
  this->NotifyAll();
  m_HasImageOpened = false;

}

/**
 * Built a part of the visu, create a pointer and add a model to the visu
 */
ImageViewerManagerModel
::VisuViewPointerType
ImageViewerManagerModel
::BuiltVisu(VisuModelPointerType pRendering)
{
  VisuViewPointerType visuView = VisuViewType::New();
  visuView->SetModel(pRendering);

  return visuView;
}

/**
 * Add Controller
 */
ImageViewerManagerModel
::WidgetControllerPointerType
ImageViewerManagerModel
::BuiltController(VisuModelPointerType modelRenderingLayer, VisuViewPointerType visuView, PixelDescriptionModelPointerType pixelModel)
{
  WidgetControllerPointerType controller = WidgetControllerType::New();

  // Add the resizing handler
  ResizingHandlerType::Pointer resizingHandler = ResizingHandlerType::New();
  resizingHandler->SetModel(modelRenderingLayer);
  resizingHandler->SetView(visuView);
  controller->AddActionHandler(resizingHandler);

    // Add the change scaled region handler
  ChangeScaledRegionHandlerType::Pointer changeScaledHandler =ChangeScaledRegionHandlerType::New();
  changeScaledHandler->SetModel(modelRenderingLayer);
  changeScaledHandler->SetView(visuView);
  controller->AddActionHandler(changeScaledHandler);

  // Add the change extract region handler
  ChangeRegionHandlerType::Pointer changeHandler =ChangeRegionHandlerType::New();
  changeHandler->SetModel(modelRenderingLayer);
  changeHandler->SetView(visuView);
  controller->AddActionHandler(changeHandler);

  // Add the change scaled handler
  ChangeScaleHandlerType::Pointer changeScaleHandler =ChangeScaleHandlerType::New();
  changeScaleHandler->SetModel(modelRenderingLayer );
  changeScaleHandler->SetView(visuView);
  controller->AddActionHandler(changeScaleHandler);

  //Pixel Description Handling
  PixelDescriptionActionHandlerType::Pointer pixelActionHandler = PixelDescriptionActionHandlerType::New();
  pixelActionHandler->SetView(visuView);
  pixelActionHandler->SetModel(pixelModel);
  controller->AddActionHandler(pixelActionHandler);

  // Add the action handler for the arrow key
  ArrowKeyMoveActionHandlerType::Pointer arrowKeyMoveHandler = ArrowKeyMoveActionHandlerType::New();
  arrowKeyMoveHandler->SetModel(modelRenderingLayer);
  arrowKeyMoveHandler->SetView(visuView);
  controller->AddActionHandler(arrowKeyMoveHandler);

  return controller;
}

void
ImageViewerManagerModel
::CloseImage(unsigned int selectedItem)
{
  m_ObjectTrackedList.erase(m_ObjectTrackedList.begin()+selectedItem-1);
}

void
ImageViewerManagerModel
::UpdateRGBChannelOrder(int redChoice , int greenChoice, int BlueChoice, unsigned int selectedItem)
{
  StandardRenderingFunctionType::Pointer renderFunction = m_ObjectTrackedList.at(selectedItem-1).pRenderFuntion;
  renderFunction->SetRedChannelIndex(redChoice);
  renderFunction->SetGreenChannelIndex(greenChoice);
  renderFunction->SetBlueChannelIndex(BlueChoice);

  //Update the layer
  m_ObjectTrackedList.at(selectedItem-1).pLayer->SetRenderingFunction(renderFunction);
  m_ObjectTrackedList.at(selectedItem-1).pRendering->Update();

  //Notify
  m_HasChangedChannelOrder = true;
  this->NotifyAll();
  m_HasChangedChannelOrder = false;
}

void
ImageViewerManagerModel
::UpdateGrayScaleChannelOrder(int choice, unsigned int selectedItem)
{
  StandardRenderingFunctionType::Pointer renderFunction = m_ObjectTrackedList.at(selectedItem-1).pRenderFuntion;
  renderFunction->SetAllChannels(choice);

  //Update the layer
  m_ObjectTrackedList.at(selectedItem-1).pLayer->SetRenderingFunction(renderFunction);
  m_ObjectTrackedList.at(selectedItem-1).pRendering->Update();

  //Notify
  m_HasChangedChannelOrder = true;
  this->NotifyAll();
  m_HasChangedChannelOrder = false;
}

void
ImageViewerManagerModel
::UpdateModulusChannelOrder(int realChoice , int imChoice, unsigned int selectedItem )
{
  ModulusRenderingFunction::Pointer modulusFunction = ModulusRenderingFunction::New();
  modulusFunction->SetRedChannelIndex(realChoice);
  modulusFunction->SetGreenChannelIndex(imChoice);
  modulusFunction->Initialize();

  //Update the layer
  m_ObjectTrackedList.at(selectedItem-1).pLayer->SetRenderingFunction(modulusFunction);
  m_ObjectTrackedList.at(selectedItem-1).pRendering->Update();

  //Notify
  m_HasChangedChannelOrder = true;
  this->NotifyAll();
  m_HasChangedChannelOrder = false;
}


void
ImageViewerManagerModel
::UpdatePhaseChannelOrder(int realChoice , int imChoice, unsigned int selectedItem )
{
  PhaseRenderingFunction::Pointer phaseFunction = PhaseRenderingFunction::New();
  phaseFunction->SetRedChannelIndex(realChoice);
  phaseFunction->SetGreenChannelIndex(imChoice);
  phaseFunction->Initialize();

  //Update the layer
  m_ObjectTrackedList.at(selectedItem-1).pLayer->SetRenderingFunction(phaseFunction);
  m_ObjectTrackedList.at(selectedItem-1).pRendering->Update();

  //Notify
  m_HasChangedChannelOrder = true;
  this->NotifyAll();
  m_HasChangedChannelOrder = false;
}

/**
 *
 */
void
ImageViewerManagerModel
::Link(unsigned int leftChoice, unsigned int rightChoice, OffsetType offset)
{

  //Create A null offset
  OffsetType nullOffset;
  nullOffset.Fill(0);

  //Get the controllers of the selected images
  WidgetControllerPointerType rightController = m_ObjectTrackedList.at(rightChoice-1).pWidgetController;
  WidgetControllerPointerType leftController = m_ObjectTrackedList.at(leftChoice-1).pWidgetController;

  //Get the models related to the choosen images
  VisuModelPointerType rightRenderModel       = m_ObjectTrackedList.at(rightChoice-1).pRendering;
  VisuModelPointerType leftRenderModel        = m_ObjectTrackedList.at(leftChoice-1).pRendering;

  //Get the views related to the choosen images
  VisuViewPointerType  pRightVisuView         = m_ObjectTrackedList.at(rightChoice-1).pVisuView;;
  VisuViewPointerType  pLeftVisuView          = m_ObjectTrackedList.at(leftChoice-1).pVisuView;

  //Pixel View
  PixelDescriptionModelPointerType rightPixelModel = m_ObjectTrackedList.at(rightChoice-1).pPixelModel;
  PixelDescriptionModelPointerType leftPixelModel  = m_ObjectTrackedList.at(leftChoice-1).pPixelModel;

  // Add the resizing handler
  ResizingHandlerType::Pointer rightResizingHandler = ResizingHandlerType::New();
  rightResizingHandler->SetModel(rightRenderModel);
  rightResizingHandler->SetView(pLeftVisuView);

  ResizingHandlerType::Pointer leftResizingHandler = ResizingHandlerType::New();
  leftResizingHandler->SetModel(leftRenderModel);
  leftResizingHandler->SetView(pRightVisuView);

  rightController->AddActionHandler( leftResizingHandler);
  leftController->AddActionHandler(rightResizingHandler);

  // Add the change scaled region handler--
  ChangeScaledRegionHandlerType::Pointer rightChangeScaledHandler =ChangeScaledRegionHandlerType::New();
  rightChangeScaledHandler->SetModel(rightRenderModel);
  rightChangeScaledHandler->SetView(pLeftVisuView);
  rightChangeScaledHandler->SetOffset(nullOffset-offset);

  ChangeScaledRegionHandlerType::Pointer leftChangeScaledHandler =ChangeScaledRegionHandlerType::New();
  leftChangeScaledHandler->SetModel(leftRenderModel);
  leftChangeScaledHandler->SetView(pRightVisuView);
  leftChangeScaledHandler->SetOffset(offset);

  rightController->AddActionHandler(leftChangeScaledHandler);
  leftController->AddActionHandler( rightChangeScaledHandler);

  // Add the change extract region handler--
  ChangeRegionHandlerType::Pointer rightChangeHandler =ChangeRegionHandlerType::New();
  rightChangeHandler->SetModel(rightRenderModel);
  rightChangeHandler->SetView(pLeftVisuView);
  rightChangeHandler->SetOffset(nullOffset-offset);

  ChangeRegionHandlerType::Pointer leftChangeHandler =ChangeRegionHandlerType::New();
  leftChangeHandler->SetModel(leftRenderModel);
  leftChangeHandler->SetView(pRightVisuView);
  leftChangeHandler->SetOffset(offset);

  rightController->AddActionHandler( leftChangeHandler);
  leftController->AddActionHandler(rightChangeHandler);

  // Add the change scaled handler
  ChangeScaleHandlerType::Pointer rightChangeScaleHandler =ChangeScaleHandlerType::New();
  rightChangeScaleHandler->SetModel(rightRenderModel );
  rightChangeScaleHandler->SetView(pLeftVisuView);

  ChangeScaleHandlerType::Pointer leftChangeScaleHandler =ChangeScaleHandlerType::New();
  leftChangeScaleHandler->SetModel(leftRenderModel );
  leftChangeScaleHandler->SetView(pRightVisuView);

  rightController->AddActionHandler( leftChangeScaleHandler);
  leftController->AddActionHandler(rightChangeScaleHandler);

 //Pixel Description Handling--
  PixelDescriptionActionHandlerType::Pointer rightPixelActionHandler = PixelDescriptionActionHandlerType::New();
  rightPixelActionHandler->SetView(pLeftVisuView );
  rightPixelActionHandler->SetModel(rightPixelModel);
  rightPixelActionHandler->SetOffset(nullOffset-offset);

  PixelDescriptionActionHandlerType::Pointer leftPixelActionHandler = PixelDescriptionActionHandlerType::New();
  leftPixelActionHandler->SetView(pRightVisuView);
  leftPixelActionHandler->SetModel(leftPixelModel);
  leftPixelActionHandler->SetOffset(offset);

  rightController->AddActionHandler(leftPixelActionHandler );
  leftController->AddActionHandler(rightPixelActionHandler);

}

/**
 *
 */
void
ImageViewerManagerModel
::InitializeImageViewController(unsigned int selectedItem)
{
  VisuModelPointerType  render = m_ObjectTrackedList.at(selectedItem-1).pRendering;
  VisuViewPointerType   view   = m_ObjectTrackedList.at(selectedItem-1).pVisuView;
  PixelDescriptionModelPointerType pixelModel = m_ObjectTrackedList.at(selectedItem-1).pPixelModel;

  m_ObjectTrackedList.at(selectedItem-1).pWidgetController = this->BuiltController(render,view,pixelModel);
  m_ObjectTrackedList.at(selectedItem-1).pVisuView->SetController(m_ObjectTrackedList.at(selectedItem-1).pWidgetController);
}

}


