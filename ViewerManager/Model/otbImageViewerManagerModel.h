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
#ifndef __otbImageViewerManagerModel_h
#define __otbImageViewerManagerModel_h

#include "otbMVCModel.h"
#include "otbImageViewerManagerEventsListener.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "itkRGBAPixel.h"
#include "otbImageFileReader.h"
#include "otbObjectList.h"

/** NewVisu */
#include "otbImageLayer.h"
#include "otbImageLayerGenerator.h"
#include "otbImageLayerRenderingModel.h"

#include "otbWidgetResizingActionHandler.h"
#include "otbChangeScaledExtractRegionActionHandler.h"
#include "otbChangeExtractRegionActionHandler.h"
#include "otbChangeScaleActionHandler.h"
#include "otbArrowKeyMoveActionHandler.h"

#include "otbModulusRenderingFunction.h"
#include "otbPhaseRenderingFunction.h"


#include "otbPixelDescriptionModel.h"
#include "otbPixelDescriptionView.h"
#include "otbPixelDescriptionActionHandler.h"
#include "otbCurves2DWidget.h"


#include "otbImageView.h"
#include "otbImageWidgetController.h"

namespace otb
{
/** \class ImageViewerManagerModel
 *
 *
 *
 */

class ITK_EXPORT ImageViewerManagerModel
      : public MVCModel<ImageViewerManagerEventsListener>
{

public:
  /** Standard class typedefs */
  typedef ImageViewerManagerModel                         Self;
  typedef MVCModel<ImageViewerManagerEventsListener>      Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  /** Standard type macro */
  itkTypeMacro(ImageViewerManagerModel, MVCModel);

  /** Images typedefs */
  typedef double                                                          PixelType;

  /**  Image Type*/
  typedef VectorImage<PixelType , 2>                                                ImageType;
  typedef itk::RGBAPixel<unsigned char>                                              RGBPixelType;
  typedef Image<RGBPixelType,2>                                                     ViewerImageType;
  typedef ImageType::Pointer                                                        ImagePointerType;

  /** typedef support for layers */
  typedef otb::ImageLayer<ImageType>                                                 LayerType;
  typedef LayerType::Pointer                                                         LayerPointerType;

  typedef otb::ImageLayerGenerator<LayerType>                                        LayerGeneratorType;
  typedef LayerGeneratorType::Pointer                                                LayerGeneratorPointerType;
  typedef LayerGeneratorType::RenderingFunctionType                                  StandardRenderingFunctionType;

  typedef Function::ModulusRenderingFunction<ImageType::InternalPixelType, RGBPixelType>    ModulusRenderingFunction;
  typedef Function::PhaseRenderingFunction<ImageType::InternalPixelType, RGBPixelType>      PhaseRenderingFunction;

  /** typedef support for reader*/
  typedef ImageFileReader<ImageType>                                                  ReaderType;
  typedef ReaderType::Pointer                                                         ReaderPointerType;

  /** Typedef support for rendering image*/
  typedef otb::ImageLayerRenderingModel<ViewerImageType>                              VisuModelType;
  typedef VisuModelType::Pointer                                                      VisuModelPointerType;

  /** NewVisu */
  typedef ImageView<VisuModelType>                                                    VisuViewType;
  typedef VisuViewType::OffsetType                                                    OffsetType;
  typedef VisuViewType::Pointer                                                       VisuViewPointerType;

  typedef ImageWidgetController                                                       WidgetControllerType;
  typedef WidgetControllerType::Pointer                                               WidgetControllerPointerType;

  typedef WidgetResizingActionHandler<VisuModelType,VisuViewType>                     ResizingHandlerType;
  typedef ResizingHandlerType::Pointer                                                ResizingHandlerPointerType;
  typedef otb::ChangeScaledExtractRegionActionHandler<VisuModelType,VisuViewType>     ChangeScaledRegionHandlerType;
  typedef otb::ChangeExtractRegionActionHandler<VisuModelType,VisuViewType>           ChangeRegionHandlerType;
  typedef otb::ChangeScaleActionHandler<VisuModelType,VisuViewType>                   ChangeScaleHandlerType;
  typedef otb::ArrowKeyMoveActionHandler<VisuModelType,VisuViewType>                  ArrowKeyMoveActionHandlerType;

  typedef otb::PixelDescriptionModel<ViewerImageType>                                 PixelDescriptionModelType;
  typedef PixelDescriptionModelType::Pointer                                          PixelDescriptionModelPointerType;
  typedef PixelDescriptionView<PixelDescriptionModelType>                             PixelDescriptionViewType;
  typedef otb::PixelDescriptionActionHandler<PixelDescriptionModelType,VisuViewType>  PixelDescriptionActionHandlerType;

  typedef Curves2DWidget                                                              CurvesWidgetType;

  /**
   * Struct embedded in the model
   */
  struct _ObjectsTracked
  {
    ReaderPointerType                      pReader;
    LayerPointerType                       pLayer;
    VisuModelPointerType                   pRendering;
    WidgetControllerPointerType            pWidgetController;
    VisuViewPointerType                    pVisuView;
    StandardRenderingFunctionType::Pointer pRenderFuntion;
    PixelDescriptionViewType::Pointer      pPixelView;
    PixelDescriptionModelPointerType       pPixelModel;
    CurvesWidgetType::Pointer              pCurveWidget;
    std::string                            fileName;
  };

  typedef struct _ObjectsTracked                                          ObjectsTracked;

  /**
   * List of objectTracked, we cannot use ObjectList
   * for struct cause don't implenement Register method
   */
  typedef std::vector<ObjectsTracked>                                    ObjectTrackedList;

  /** Get the unique instanc1e of the model */
  static Pointer GetInstance();

  virtual void OpenImage(std::string strfilename);
  virtual void CloseImage(unsigned int selectedItem);
  virtual void UpdateRGBChannelOrder(int redChoice , int greenChoice, int BlueChoice, unsigned int selectedItem);
  virtual void UpdateGrayScaleChannelOrder(int choice, unsigned int selectedItem);
  virtual void UpdateModulusChannelOrder(int realChoice , int imChoice,unsigned int selectedItem );
  virtual void UpdatePhaseChannelOrder(int realChoice , int imChoice,unsigned int selectedItem );
  virtual void Link(unsigned int leftChoice, unsigned int rightChoice, OffsetType offset);
  virtual void InitializeImageViewController(unsigned int selectedItem);

  /** Method needed to Get the list of componenets stored*/
   ObjectTrackedList GetObjectList()
    {
      return m_ObjectTrackedList;
    }

   /** Boolean Flags */
   itkGetMacro(HasImageOpened,bool);
   itkGetMacro(HasChangedChannelOrder,bool);

protected:
  /** This is protected for the singleton. Use GetInstance() instead. */
  itkNewMacro(Self);

  /** Constructor */
  ImageViewerManagerModel();

  /** Destructor */
  virtual ~ImageViewerManagerModel();

  /** Built Visu & Controller*/
  virtual VisuViewPointerType BuiltVisu(VisuModelPointerType pRendering);
  virtual WidgetControllerPointerType BuiltController(VisuModelPointerType modelRenderingLayer, VisuViewPointerType visuView , PixelDescriptionModelType::Pointer pixelModel);


private:
  ImageViewerManagerModel(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Notify a given listener of changes */
  virtual void Notify(ImageViewerManagerEventsListener * listener);

  /** The instance singleton */
  static Pointer Instance;

  /** Boolean flags*/
  bool   m_HasImageOpened;
  bool   m_HasChangedChannelOrder;

  /** The manipuleted list*/
  ObjectTrackedList                          m_ObjectTrackedList;
};



}
#endif

