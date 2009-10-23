/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
See OTBCopyright.txt for details.


    This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULA
     PURPOSE,  See the above copyright notices for more information.

=========================================================================*/
#include <FL/Fl_Text_Buffer.H>
#include "otbImageViewerManagerViewGUI.h"
#include "otbMacro.h"
#include "otbFileName.h"

#include "base/ossimFilename.h"
#include "base/ossimDirectory.h"

namespace otb
{
/**
 * Constructor
 */

ImageViewerManagerViewGUI
::ImageViewerManagerViewGUI():m_TemplateViewerName(""),m_DisplayedLabel("+ "),
			      m_UndisplayedLabel("- "),m_DiaporamaCurrentIndex(0)
{

  m_VisuView               =  VisuViewType::New();
  m_PreviewWidget          =  ImageWidgetType::New();
  m_pRenderingFunction      =  StandardRenderingFunctionType::New();
  m_WidgetManagerList      =  WidgetManagerList::New();
  m_LinkWidgetManagerList  =  WidgetManagerList::New();

  //Get an instance of the model
  m_ImageViewerManagerModel = ImageViewerManagerModel::GetInstance();
  m_ImageViewerManagerModel->RegisterListener(this);

  //Create the component of the GUI
  this->CreateGUI();
  bSplitted->activate();
  bPacked->activate();

  //Create buffer for the guiViewerInformation
  Fl_Text_Buffer * buffer = new Fl_Text_Buffer();
  this->guiViewerInformation->buffer(buffer);

  //init the previewWindow
  m_PreviewWidget->label("PreviewWidget");
  gPreviewWindow->add(m_PreviewWidget);
  gPreviewWindow->box(FL_NO_BOX);
  gPreviewWindow->resizable(gPreviewWindow);
  m_PreviewWidget->resize(gPreviewWindow->x(), gPreviewWindow->y(), gPreviewWindow->w(), gPreviewWindow->h() );

  //Color Definition
  m_Red.Fill(0);
  m_Green.Fill(0);
  m_Blue.Fill(0);
  m_Grey.Fill(0.5);
  m_Red[0]  = 1.;   m_Red[3]   = 0.5;
  m_Green[1]= 1.;   m_Green[3] = 0.5;
  m_Blue[2] = 1.;   m_Blue[3]  = 0.5;

  //Slide Show
  m_WidgetManager  =  PackedWidgetManagerType::New();
}

void
ImageViewerManagerViewGUI
::OpenImage(const char * inputFileName)
{
  //Initialize
  this->Initialize(inputFileName);
}

/**
 *
 */
void
ImageViewerManagerViewGUI
::OpenImage()
{
  std::string pikedFileName="";
  char * cfname ;
  cfname = fl_file_chooser("Pick an image file", "*.*",pikedFileName.c_str());

  if (cfname == NULL || strlen(cfname)<1)
    {
      otbMsgDebugMacro(<<"Empty file name!");
      return ;
    }

  Fl::check();
  guiMainWindow->redraw();

  //Initialize
  this->Initialize(cfname);
}


/**
 *
 */
void
ImageViewerManagerViewGUI
::Initialize(const char * cfname)
{
  //Initialise the boolean pair
  PairType      pair(false,false); //(Not displayed , Packed View)

  //Put a new WidgetManager in the list
  if(bSplitted->value() && !bPacked->value())
    {
      SplittedWidgetManagerType::Pointer widgetManager      =  SplittedWidgetManagerType::New();
      SplittedWidgetManagerType::Pointer linkwidgetManager  =  SplittedWidgetManagerType::New();

      m_WidgetManagerList->PushBack(widgetManager);
      m_LinkWidgetManagerList->PushBack(linkwidgetManager);
      pair.second = true;
    }
  else
    {
      PackedWidgetManagerType::Pointer widgetManager         =   PackedWidgetManagerType::New();
      PackedWidgetManagerType::Pointer linkwidgetManager     =   PackedWidgetManagerType::New();
      m_WidgetManagerList->PushBack(widgetManager);
      m_LinkWidgetManagerList->PushBack(linkwidgetManager);
    }

  //Put the status of the last added image
  m_DisplayStatusList.push_back(pair);
  m_LinkedDisplayStatusList.push_back(false);

  // Call the Controller
  m_ImageViewerManagerController->OpenInputImage(cfname);

  //Update the Progress Bar
  this->UpdateDiaporamaProgressBar();

  //Update the Link Setup
  this->UpdateLinkSetupWindow();
}

/**
 * Handle the notification of the model
 */
void
ImageViewerManagerViewGUI
::Notify()
{
  if(m_ImageViewerManagerModel->GetHasImageOpened())
    this->AddImageListName();

  //Update the widget when channel order modified
  if(m_ImageViewerManagerModel->GetHasChangedChannelOrder())
    {
      unsigned int selectedItem = guiImageList->value();
      if (selectedItem == 0)
	return;        // no image selected, return

      //DipalyPreviewWidget
      this->DisplayPreviewWidget(selectedItem);

      //Update the widget dispalyed
      if(m_DisplayStatusList[selectedItem-1].first)
	{
	  m_WidgetManagerList->GetNthElement(selectedItem-1)->Refresh();
	  this->Display(m_WidgetManagerList,selectedItem);
	}

      this->UpdateInformation(selectedItem);
    }
}

/**
 * CloseImage , Send the notification to the controller then to the model
 */
void
ImageViewerManagerViewGUI
::CloseImage()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
    return;        // no image selected, return

  //Hide if showned
  if(m_DisplayStatusList[selectedItem-1].first)
    {
      this->Undisplay(selectedItem);
    }

  //Check if the closed image is linked and showed
  // if it is the case : undisplay it and update the linkSetup
  if(guiLinkSetupWindow->shown() != 0)
    {
      if(guiImageList->size() > 1)
	{
	  if(m_LinkedDisplayStatusList[selectedItem-1])
	    for(unsigned int i = 0; i<m_LinkedDisplayStatusList.size() ; i++)
	      if(m_LinkedDisplayStatusList[i])
		{
		  m_LinkWidgetManagerList->GetNthElement(i)->Hide();
		  m_LinkedDisplayStatusList[i] = false;
		}
	}
    }


  //Erase the item selected
  m_PreviewWidget->hide();
  guiImageList->remove(selectedItem);

  //Link when all images are closed
  if(guiLinkSetupWindow->shown() != 0)
      if(guiImageList->size()  == 0)
	this->LinkSetupOk();

  //Erase from the lists
  m_DisplayStatusList.erase( m_DisplayStatusList.begin()+(selectedItem-1));
  m_LinkedDisplayStatusList.erase( m_LinkedDisplayStatusList.begin()+(selectedItem-1));
  m_WidgetManagerList->Erase(selectedItem-1);
  m_LinkWidgetManagerList->Erase(selectedItem-1);


  //Diaporama
  if(guiDiaporama->shown() != 0)
    {
      if(guiImageList->size() > 0)
	{
	  //If the closed image is the current showed in the diaporama
	  if(selectedItem == m_DiaporamaCurrentIndex+1)
	    {
	      //if the closed image is the last one in the diapo : show the previous
	      if(selectedItem -1 == static_cast<unsigned int>(guiImageList->size()))
		{
		  this->DiaporamaPrevious();
		}
	      else
		{
		  //if the closed image is the first one : show the next one
		  this->UpdateDiaporamaProgressBar();
		  ++m_DiaporamaCurrentIndex;            //Increment to show the next image
		  this->DisplayDiaporama();
		  --m_DiaporamaCurrentIndex;            //Decrement because we remove the first image, so the current display become the first
		}
	    }
	  else
	    {
	      if(selectedItem < m_DiaporamaCurrentIndex+1)
		{
		  --m_DiaporamaCurrentIndex;         //Increment the current index because an image before the one displayed is removed
		}
	      this->UpdateDiaporamaProgressBar();
	    }
	}
    }

    //Case all Images closed
    if(guiDiaporama->shown() != 0)
    {
      if(m_DisplayStatusList.size() == 0)
	{
	  m_DiaporamaCurrentIndex=0;
	  this->DiaporamaQuit();          //If no image to display anymore : quit the diaporamaGUI
	}
    }

    //
    //Udpate the controller of the image linked with the suppressed one
    if(guiImageList->size() > 0)
    this->InitializeImageController(selectedItem);

  //Call the controller
  m_ImageViewerManagerController->CloseImage(selectedItem);

  //Update the Link Setup
  this->UpdateLinkSetupWindow();



 }

 /**
  * Show the mainWindow
  */
 void
 ImageViewerManagerViewGUI
 ::Show()
 {
   guiMainWindow->show();
 }

 /**
  * Update the filename
  */
 void
 ImageViewerManagerViewGUI
 ::AddImageListName()
 {
   //Update the Image List widget
   unsigned int len     = m_ImageViewerManagerModel->GetObjectList().size();
   std::string fileName = m_ImageViewerManagerModel->GetObjectList().at(len-1).fileName;
   int slashIndex       = fileName.find_last_of("/",fileName.size());

   itk::OStringStream oss;
   oss<<m_UndisplayedLabel;
   oss<<m_TemplateViewerName<<fileName.substr(slashIndex+1,fileName.size());
   guiImageList->add(oss.str().c_str());
   guiImageList->redraw();
 }

 /**
  *
  */
 void
 ImageViewerManagerViewGUI
 ::SelectAction()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
     {
       // no image selected, return
       return;
     }

   //Update the radio button
   if( m_DisplayStatusList[selectedItem-1].second)
     {
       bSplitted->value(true);
       bPacked->value(false);
     }
   else
     {
       bSplitted->value(false);
       bPacked->value(true);
     }

   //DipalyPreviewWidget
   this->DisplayPreviewWidget(selectedItem);

   //Update SelectedImageInformation
   this->UpdateInformation(selectedItem);

   //Udpate the ViewerGUISetup
   this->UpdateViewerSetupWindow(selectedItem);

   //LINK SETUP
   this->UpdateLinkSetupWindow();
 }
 /**
  *
  */
 void
 ImageViewerManagerViewGUI
 ::DisplayPreviewWidget(unsigned int selectedItem)
 {
   //Build the m_PreviewWidget
   VisuModelPointerType rendering = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRendering;

   m_PreviewWidget->ClearBuffer();
   ImageViewerManagerModelType::ViewerImageType * quickLook = rendering->GetRasterizedQuicklook();
   m_PreviewWidget->ReadBuffer(quickLook, quickLook->GetLargestPossibleRegion());

   double newIsotropicZoom = this->UpdatePreviewWidgetIsotropicZoom(quickLook->GetLargestPossibleRegion().GetSize());
   m_PreviewWidget->SetIsotropicZoom(newIsotropicZoom);
   m_PreviewWidget ->show();
   m_PreviewWidget->redraw();
 }



 /**
  * Compute the size of the
  */
 double
 ImageViewerManagerViewGUI
 ::UpdatePreviewWidgetIsotropicZoom(SizeType size)
 {
   int h = gPreviewWindow->h();
   int w = gPreviewWindow->w();

   double zoomW = static_cast<double>(w)/static_cast<double>(size[0]);
   double zoomH = static_cast<double>(h)/static_cast<double>(size[1]);

   return std::min(zoomW,zoomH);
 }

 /**
  * Show Hide
  */
 void
 ImageViewerManagerViewGUI
 ::ShowHide()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
     {
       // no image selected, return
       return;
     }
   if(guiDiaporama->shown() == 0 && guiLinkSetupWindow->shown() == 0)
     {
       //check what to do
       if(!m_DisplayStatusList[selectedItem-1].first)
	 {
	   //New Display
	   m_DisplayStatusList[selectedItem-1].first = true;
	   this->UpdateImageListShowed(selectedItem, m_DisplayedLabel);
	   this->Display(m_WidgetManagerList,selectedItem);
	 }
       else
	 {
	   m_DisplayStatusList[selectedItem-1].first = false;
	   this->UpdateImageListShowed(selectedItem, m_UndisplayedLabel);
	   this->Undisplay(selectedItem);
	 }
     }
 }

 /**
  * Display the three widget
  */
 void
 ImageViewerManagerViewGUI
 ::Display(WidgetManagerList::Pointer  widgetList, unsigned int selectedItem)
 {
   //Get the view stored in the model
   CurvesWidgetType::Pointer         curveWidget         =  m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pCurveWidget;
   VisuViewPointerType               currentVisuView     =  m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pVisuView;

   //First get the histogram list
   RenderingFunctionType::Pointer pRenderingFunction = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction;





   curveWidget->ClearAllCurves();

   if (pRenderingFunction->GetPixelRepresentationSize() >=3)
   {
     HistogramCurveType::Pointer bhistogram = HistogramCurveType::New();
     bhistogram->SetHistogramColor(m_Blue);
     bhistogram->SetLabelColor(m_Blue);
     bhistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pLayer->GetHistogramList()->GetNthElement(2));
     curveWidget->AddCurve(bhistogram);
   }

   if (pRenderingFunction->GetPixelRepresentationSize() >=2)
   {
     HistogramCurveType::Pointer ghistogram = HistogramCurveType::New();
     ghistogram->SetHistogramColor(m_Green);
     ghistogram->SetLabelColor(m_Green);
     ghistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pLayer->GetHistogramList()->GetNthElement(1));
     curveWidget->AddCurve(ghistogram);
   }

   HistogramCurveType::Pointer rhistogram = HistogramCurveType::New();
   if (pRenderingFunction->GetPixelRepresentationSize() == 1)
   {
     rhistogram->SetHistogramColor(m_Grey);
     rhistogram->SetLabelColor(m_Grey);
   }
   else
   {
     rhistogram->SetHistogramColor(m_Red);
     rhistogram->SetLabelColor(m_Red);
   }
   rhistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pLayer->GetHistogramList()->GetNthElement(0));
   curveWidget->AddCurve(rhistogram);


   curveWidget->SetXAxisLabel("Pixels");
   curveWidget->SetYAxisLabel("Frequency");

   //Get the pixelView
   PixelDescriptionViewType::Pointer pixelView = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pPixelView;

   //   //Edit the Widget Manager
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->UnRegisterAll();
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->RegisterFullWidget(currentVisuView->GetFullWidget());
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->RegisterScrollWidget(currentVisuView->GetScrollWidget());
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->RegisterZoomWidget(currentVisuView->GetZoomWidget());
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->RegisterPixelDescriptionWidget(pixelView->GetPixelDescriptionWidget());
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->RegisterHistogramWidget(curveWidget);
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->SetLabel(this->CutFileName(selectedItem-1));
   //   m_WidgetManagerList->GetNthElement(selectedItem-1)->Show();

   //widgetList->GetNthElement(selectedItem-1)->UnRegisterAll();
   widgetList->GetNthElement(selectedItem-1)->RegisterFullWidget(currentVisuView->GetFullWidget());
   widgetList->GetNthElement(selectedItem-1)->RegisterScrollWidget(currentVisuView->GetScrollWidget());
   widgetList->GetNthElement(selectedItem-1)->RegisterZoomWidget(currentVisuView->GetZoomWidget());
   widgetList->GetNthElement(selectedItem-1)->RegisterPixelDescriptionWidget(pixelView->GetPixelDescriptionWidget());
   widgetList->GetNthElement(selectedItem-1)->RegisterHistogramWidget(curveWidget);
   widgetList->GetNthElement(selectedItem-1)->SetLabel(this->CutFileName(selectedItem-1));
   widgetList->GetNthElement(selectedItem-1)->Show();
 }

 /**
  */
 void
 ImageViewerManagerViewGUI
 ::SplittedViewMode()
 {
     unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
     {
       // no image selected, return
       return;
     }

   SplittedWidgetManagerType::Pointer widgetManager  =  SplittedWidgetManagerType::New();
   m_WidgetManagerList->GetNthElement(selectedItem-1)->Hide();
   m_WidgetManagerList->SetNthElement(selectedItem-1,widgetManager);

   //Update the list link
   m_LinkWidgetManagerList->GetNthElement(selectedItem-1)->Hide();
   m_LinkWidgetManagerList->SetNthElement(selectedItem-1,widgetManager);

   if(m_LinkedDisplayStatusList[selectedItem-1])
     {
       this->Display(m_LinkWidgetManagerList, selectedItem);
     }

   //If the guilink or te guiDiapo is opened don't display and don't change the view mode
   if(guiDiaporama->shown() == 0 && guiLinkSetupWindow->shown() == 0)
     {
       //Update the view mode if the link mode is not updated
       if(!m_DisplayStatusList[selectedItem-1].first && !m_LinkedDisplayStatusList[selectedItem-1])
	 {
	   m_DisplayStatusList[selectedItem-1].second = true;
	 }

       //If already displayed : update the view mode and display the new viewMode
       if(m_DisplayStatusList[selectedItem-1].first && !m_LinkedDisplayStatusList[selectedItem-1] )
	 {
	   m_DisplayStatusList[selectedItem-1].second = true;
	   this->Display(m_WidgetManagerList, selectedItem);
	 }
     }
 }

 /**
 *
 */
 void
 ImageViewerManagerViewGUI
 ::PackedViewMode()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
     {
       // no image selected, return
       return;
     }

   //
   PackedWidgetManagerType::Pointer widgetManager  =  PackedWidgetManagerType::New();
   m_WidgetManagerList->GetNthElement(selectedItem-1)->Hide();
   m_WidgetManagerList->SetNthElement(selectedItem-1,widgetManager);

   //Update the link list
   m_LinkWidgetManagerList->GetNthElement(selectedItem-1)->Hide();
   m_LinkWidgetManagerList->SetNthElement(selectedItem-1,widgetManager);

   if(m_LinkedDisplayStatusList[selectedItem-1])
     {
       this->Display(m_LinkWidgetManagerList, selectedItem);
     }


   //If the guilink or te guiDiapo is opened don't display and don't change the view mode
   if(guiDiaporama->shown() == 0 && guiLinkSetupWindow->shown() == 0)
     {
       //Update the view mode
       if(!m_DisplayStatusList[selectedItem-1].first && !m_LinkedDisplayStatusList[selectedItem-1])
	 {
	   m_DisplayStatusList[selectedItem-1].second = false;
	 }

       //If already displayed : update the view mode and display the new viewMode
       if(m_DisplayStatusList[selectedItem-1].first && !m_LinkedDisplayStatusList[selectedItem-1] )
	 {
	   m_DisplayStatusList[selectedItem-1].second = false;
	   this->Display(m_WidgetManagerList, selectedItem);
	 }
     }
 }

 /**
  * Update the guiImageList
  * Put a  "+" if the view is being showed, or a "-" otherwise in the begining of the imageName
  */
 void
 ImageViewerManagerViewGUI
 ::UpdateImageListShowed(unsigned int selectedItem, std::string status)
 {
   /* Update the ImageList using the status label "+" or "-" */
   std::string fileName = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).fileName;
   int slashIndex = fileName.find_last_of("/",fileName.size());

   itk::OStringStream oss;
   oss<<status;
   oss<<m_TemplateViewerName<<fileName.substr(slashIndex+1,fileName.size());
   guiImageList->text(selectedItem,oss.str().c_str());
   oss.str("");
 }

 /**
  * Hide all the widget opened
  */
 void
 ImageViewerManagerViewGUI
 ::Undisplay(unsigned int selectedItem)
 {
   m_WidgetManagerList->GetNthElement(selectedItem-1)->Hide();
   m_WidgetManagerList->GetNthElement(selectedItem-1)->UnRegisterAll();
 }

 /**
  * Hide all the widget opened
  */
 void
 ImageViewerManagerViewGUI
 ::HideAll()
 {
   //Hide all only if the diaporama and link window are not displayed
   if(guiDiaporama->shown() == 0 && guiLinkSetupWindow->shown() == 0)
     this->CloseAllDisplayedImages(true);
 }

 /**
  * Close all the displayed images:
  * showHideEvent : if true clear the showedlist else don't clear.
  */
 void
 ImageViewerManagerViewGUI
 ::CloseAllDisplayedImages(bool showHideEvent)
 {
   // Set the display Label to undislayed
   for(unsigned int i = 0; i<m_DisplayStatusList.size() ; i++)
     {
       if(m_DisplayStatusList[i].first)
	 {
	   if(showHideEvent)
	     {
	       this->UpdateImageListShowed(i+1, m_UndisplayedLabel);
	       m_DisplayStatusList[i].first = false;
	     }
	   m_WidgetManagerList->GetNthElement(i)->Hide();
	   m_WidgetManagerList->GetNthElement(i)->UnRegisterAll();
	 }
     }
 }

 /**
  *
  */
 void
 ImageViewerManagerViewGUI
 ::ShowTemporaryClosedDisplay()
 {
   for(unsigned int i = 0; i<m_DisplayStatusList.size() ; i++)
     if(m_DisplayStatusList[i].first )
       this->Display(m_WidgetManagerList,i+1);
 }


 /**
  * Quit GUI
  */
 void
 ImageViewerManagerViewGUI
 ::Quit()
 {
   guiMainWindow->hide();

   for(unsigned int i = 0; i<m_DisplayStatusList.size() ; i++)
     if(m_DisplayStatusList[i].first)
       {
	 m_WidgetManagerList->GetNthElement(i)->Hide();
	 m_WidgetManagerList->GetNthElement(i)->UnRegisterAll();
       }

   //Quit the Diaporama if shown
   if(guiDiaporama->shown() != 0 )
     {
       guiDiaporama->hide();
       m_WidgetManager->Hide();
     }

   //Hide the linkSetupWindow
   if(guiLinkSetupWindow->shown() != 0)
     {
       guiLinkSetupWindow->hide();
       //Close the  displays linked
       for(unsigned int i = 0; i<m_LinkedDisplayStatusList.size() ; i++)
	 if(m_LinkedDisplayStatusList[i])
	   {
	     m_LinkWidgetManagerList->GetNthElement(i)->Hide();
	   }
     }

 }

 /**
  *
  */
 void
 ImageViewerManagerViewGUI
 ::UpdateInformation(unsigned int selectedItem)
 {
   itk::OStringStream oss;
   oss.str("");
   std::string selectedImageName = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).fileName;
   // Clear the info buffer
   guiViewerInformation->buffer()->remove(0,guiViewerInformation->buffer()->length());
   oss<<"Filename: "<<selectedImageName<<std::endl;
   guiViewerInformation->insert(oss.str().c_str());
   oss.str("");
   oss<<"Image information:"<<std::endl;
   guiViewerInformation->insert(oss.str().c_str());
   oss.str("");
   oss<<"Number of bands: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader->GetOutput()->GetNumberOfComponentsPerPixel();
   oss<<" - Size: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader->GetOutput()->GetLargestPossibleRegion().GetSize()<<std::endl;

   guiViewerInformation->insert(oss.str().c_str());
   oss.str("");

   //update band information
   if(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader->GetOutput()->GetNumberOfComponentsPerPixel()>=3)
     {
       //FIXME why this is not using the Describe method of the rendering function?
//        oss<<"RGB Composition: ";
//        oss<<" Band 1: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction->GetRedChannelIndex();
//        oss<<" Band 2: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction->GetGreenChannelIndex();
//        oss<<" Band 3: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction->GetBlueChannelIndex()<<std::endl;
     }

   guiViewerInformation->insert(oss.str().c_str());
   oss.str("");
 }

 /**
  *
  */
 void
 ImageViewerManagerViewGUI
 ::UpdateViewerSetupWindow(unsigned int selectedItem)
 {

   ImageViewerManagerModelType::ReaderPointerType reader = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader;
   unsigned int nbComponent = reader->GetOutput()->GetNumberOfComponentsPerPixel();

   itk::OStringStream oss;
   oss.str("");

   //Clear all the choices
   guiGrayscaleChannelChoice->clear();
   guiRedChannelChoice->clear();
   guiGreenChannelChoice->clear();
   guiBlueChannelChoice->clear();
   //guiGrayscaleChannelChoice->clear();
   guiRealChannelChoice->clear();
   guiImaginaryChannelChoice->clear();

   for (unsigned int i = 0;i<nbComponent;++i)
   {
     oss.str("");
     oss<<i+1;
     guiGrayscaleChannelChoice->add(oss.str().c_str());
     guiRedChannelChoice->add(oss.str().c_str());
     guiGreenChannelChoice->add(oss.str().c_str());
     guiBlueChannelChoice->add(oss.str().c_str());
     guiGrayscaleChannelChoice->add(oss.str().c_str());
     guiRealChannelChoice->add(oss.str().c_str());
     guiImaginaryChannelChoice->add(oss.str().c_str());
   }
   guiViewerSetupWindow->redraw();
   //guiGrayscaleChannelChoice->redraw();

   switch(nbComponent){
   case 1 :
     this->GrayScaleSet();
     break;
   case 4 :
     this->RGBSet();
     break;
   case 3 :
     this->RGBSet();
     break;
   case 2:
     this->ComplexSet();
     break;
   default :
     break;
   }

   guiViewerSetupName->value(this->CutFileName(selectedItem-1));

 }

 /**
  * RGBSet();
  */
 void
 ImageViewerManagerViewGUI
 ::RGBSet()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
     {
       // no image selected, return
       return;
     }

   ImageViewerManagerModelType::ReaderPointerType reader = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader;
   unsigned int nbComponent = reader->GetOutput()->GetNumberOfComponentsPerPixel();

   guiViewerSetupColorMode->set();
   guiViewerSetupComplexMode->clear();
   guiViewerSetupGrayscaleMode->clear();
   guiGrayscaleChannelChoice->deactivate();
   guiRealChannelChoice->deactivate();
   guiImaginaryChannelChoice->deactivate();
   bAmplitude->deactivate();
   bPhase->deactivate();

   guiRedChannelChoice->activate();
   guiGreenChannelChoice->activate();
   guiBlueChannelChoice->activate();

   assert(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction.GetPointer());

   RenderingFunctionType::Pointer renderingFunction = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction;

   ChannelListType channels = renderingFunction->GetChannelList();
   unsigned int i=0;
   while (channels.size() < 3)
   {
     channels.push_back(i);
     ++i;
   }


   guiRedChannelChoice->value(std::min(channels[0],nbComponent-1));
   guiGreenChannelChoice->value(std::min(channels[1],nbComponent-1));
   guiBlueChannelChoice->value(std::min(channels[2],nbComponent-1));

 }

 /**
  * GrayScale();
  */
 void
 ImageViewerManagerViewGUI
 ::GrayScaleSet()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
     {
       // no image selected, return
       return;
     }
   ImageViewerManagerModelType::ReaderPointerType reader = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader;
   unsigned int nbComponent = reader->GetOutput()->GetNumberOfComponentsPerPixel();

   guiViewerSetupGrayscaleMode->set();
   guiViewerSetupComplexMode->clear();
   guiViewerSetupColorMode->clear();

   guiRealChannelChoice->deactivate();
   guiImaginaryChannelChoice->deactivate();
   bAmplitude->deactivate();
   bPhase->deactivate();
   guiRedChannelChoice->deactivate();
   guiGreenChannelChoice->deactivate();
   guiBlueChannelChoice->deactivate();

   guiGrayscaleChannelChoice->activate();

   assert(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction.GetPointer());

   RenderingFunctionType::Pointer renderingFunction = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction;

   ChannelListType channels = renderingFunction->GetChannelList();
   if (channels.size() < 1)
   {
     channels.push_back(0);
   }

   guiGrayscaleChannelChoice->value(std::min(channels[0],nbComponent-1));
 }


 void
 ImageViewerManagerViewGUI
 ::ComplexSet()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
   {
     // no image selected, return
     return;
   }
   ImageViewerManagerModelType::ReaderPointerType reader = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader;
   unsigned int nbComponent = reader->GetOutput()->GetNumberOfComponentsPerPixel();

   guiViewerSetupComplexMode->set();
   guiViewerSetupColorMode->clear();
   guiViewerSetupGrayscaleMode->clear();
   guiGrayscaleChannelChoice->deactivate();
   guiRedChannelChoice->deactivate();
   guiGreenChannelChoice->deactivate();
   guiBlueChannelChoice->deactivate();
   guiRealChannelChoice->activate();
   guiImaginaryChannelChoice->activate();
   bAmplitude->activate();
   bPhase->activate();


   assert(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction.GetPointer());
   RenderingFunctionType::Pointer renderingFunction = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFunction;

   ChannelListType channels = renderingFunction->GetChannelList();
   unsigned int i=0;
   while (channels.size() < 2)
   {
     channels.push_back(i);
     ++i;
   }

   guiRealChannelChoice->value(std::min(channels[0],nbComponent-1));
   guiImaginaryChannelChoice->value(std::min(channels[1],nbComponent-1));
 }


 /**
  * ViewerSetup();
  */
 void
 ImageViewerManagerViewGUI
 ::ViewerSetup()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
   {
     // no image selected, return
     return;
   }
   guiViewerSetupWindow->show();
 }

 void
 ImageViewerManagerViewGUI
 ::ViewerSetupOk()
 {
   unsigned int selectedItem = guiImageList->value();
   if (selectedItem == 0)
   {
     // no image selected, return
     return;
   }

   if (guiViewerSetupColorMode->value())
     {
       m_ImageViewerManagerController->UpdateRGBChannelOrder(atoi(guiRedChannelChoice->value())-1,
							     atoi(guiGreenChannelChoice->value())-1,
							     atoi(guiBlueChannelChoice->value())-1,
							     selectedItem);
     }
   else if (guiViewerSetupGrayscaleMode->value())
     {
       m_ImageViewerManagerController->UpdateGrayScaleChannelOrder(atoi(guiGrayscaleChannelChoice->value())-1,
								   selectedItem);
     }
   else if (guiViewerSetupComplexMode->value())
     {
       if (bAmplitude->value())
	 {
	   m_ImageViewerManagerController->UpdateAmplitudeChannelOrder(atoi(guiRealChannelChoice->value())-1,
								    atoi(guiImaginaryChannelChoice->value())-1,
								    selectedItem);
	 }
       else
	 {
	   m_ImageViewerManagerController->UpdatePhaseChannelOrder(atoi(guiRealChannelChoice->value())-1,
								   atoi(guiImaginaryChannelChoice->value())-1,
								   selectedItem);
	 }
     }
 }

 void
 ImageViewerManagerViewGUI
 ::ViewerSetupCancel()
 {
   guiViewerSetupWindow->hide();
 }



 void
 ImageViewerManagerViewGUI
 ::Diaporama()
 {
   if (guiImageList->size()  == 0 || guiDiaporama->shown())
   {
     // no image selected, return
     return;
   }

   if(guiLinkSetupWindow->shown() == 0)
     {
       guiDiaporama->show();


       //Close the showed image without clearing the ShowedList
       this->CloseAllDisplayedImages(false);

       //Show the diaporama widget
       this->DisplayDiaporama();
       UpdateDiaporamaProgressBar();
     }
 }


 /**
  * Cut a path to get only the imageName
  */
 const char *
 ImageViewerManagerViewGUI
 ::CutFileName(unsigned int selectedItem)
 {
   std::string fileName     = m_ImageViewerManagerModel->GetObjectList().at(selectedItem).fileName;
   int slashIndex           =  fileName.find_last_of("/",fileName.size());
   std::string  fileNameCut = fileName.substr(slashIndex+1,fileName.size());

   return fileNameCut.c_str();
 }

 void
 ImageViewerManagerViewGUI
 ::DisplayDiaporama()
 {
   //Get the view stored in the model
  CurvesWidgetType::Pointer         curveWidget         =  m_ImageViewerManagerModel->GetObjectList().at(m_DiaporamaCurrentIndex).pCurveWidget;
  VisuViewPointerType               currentVisuView     =  m_ImageViewerManagerModel->GetObjectList().at(m_DiaporamaCurrentIndex).pVisuView;

  //First get the histogram list
  RenderingFunctionType::Pointer pRenderingFunction = m_ImageViewerManagerModel->GetObjectList().at(m_DiaporamaCurrentIndex).pRenderFunction;

  HistogramCurveType::Pointer rhistogram = HistogramCurveType::New();
  HistogramCurveType::Pointer ghistogram = HistogramCurveType::New();
  HistogramCurveType::Pointer bhistogram = HistogramCurveType::New();

  //Color Definition
  HistogramCurveType::ColorType                 Red;
  HistogramCurveType::ColorType                 Green;
  HistogramCurveType::ColorType                 Blue;
  Red.Fill(0);
  Green.Fill(0);
  Blue.Fill(0);
  Red[0]  = 1.;   Red[3]   = 0.5;
  Green[1]= 1.;   Green[3] = 0.5;
  Blue[2] = 1.;   Blue[3]  = 0.5;

  ghistogram->SetHistogramColor(Green);
  ghistogram->SetLabelColor(Green);
  bhistogram->SetHistogramColor(Blue);
  bhistogram->SetLabelColor(Blue);
  rhistogram->SetHistogramColor(Red);
  rhistogram->SetLabelColor(Red);

  rhistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(m_DiaporamaCurrentIndex).pLayer->GetHistogramList()->GetNthElement(0));
  ghistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(m_DiaporamaCurrentIndex).pLayer->GetHistogramList()->GetNthElement(1));
  bhistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(m_DiaporamaCurrentIndex).pLayer->GetHistogramList()->GetNthElement(2));

  curveWidget->ClearAllCurves();
  curveWidget->AddCurve(bhistogram);
  curveWidget->AddCurve(ghistogram);
  curveWidget->AddCurve(rhistogram);
  curveWidget->SetXAxisLabel("Pixels");
  curveWidget->SetYAxisLabel("Frequency");

  //Get the pixelView
  PixelDescriptionViewType::Pointer pixelView = m_ImageViewerManagerModel->GetObjectList().at(m_DiaporamaCurrentIndex).pPixelView;

  //Edit the Widget Manager
  m_WidgetManager->UnRegisterAll();
  m_WidgetManager->RegisterFullWidget(currentVisuView->GetFullWidget());
  m_WidgetManager->RegisterScrollWidget(currentVisuView->GetScrollWidget());
  m_WidgetManager->RegisterZoomWidget(currentVisuView->GetZoomWidget());
  m_WidgetManager->RegisterPixelDescriptionWidget(pixelView->GetPixelDescriptionWidget());
  m_WidgetManager->RegisterHistogramWidget(curveWidget);
  m_WidgetManager->SetLabel(this->CutFileName(m_DiaporamaCurrentIndex));
  m_WidgetManager->Refresh();
  m_WidgetManager->Show();

}
void
ImageViewerManagerViewGUI
::DiaporamaNext()
{
  if (m_DiaporamaCurrentIndex < static_cast<unsigned int>(guiImageList->size())-1)
  {
    ++m_DiaporamaCurrentIndex;
    this->DisplayDiaporama();
  }
  this->UpdateDiaporamaProgressBar();
}

void
ImageViewerManagerViewGUI
::DiaporamaPrevious()
{
  if (m_DiaporamaCurrentIndex>0)
  {
    --m_DiaporamaCurrentIndex;
    this->DisplayDiaporama();
  }
  this->UpdateDiaporamaProgressBar();
}


void
ImageViewerManagerViewGUI
::UpdateDiaporamaProgressBar()
{
  itk::OStringStream oss;
  oss.str("");
  oss<<m_DiaporamaCurrentIndex+1<<"/"<<guiImageList->size();
  guiDiaporamaProgressBar->copy_label(oss.str().c_str());
  guiDiaporamaProgressBar->minimum(1.);
  guiDiaporamaProgressBar->maximum(static_cast<float>(guiImageList->size()));
  guiDiaporamaProgressBar->value(static_cast<float>(m_DiaporamaCurrentIndex));
}

void
ImageViewerManagerViewGUI
::DiaporamaQuit()
{
  guiDiaporama->hide();
  m_WidgetManager->Hide();
  m_WidgetManager->UnRegisterAll();
  this->ShowTemporaryClosedDisplay();
}

/**
 *
 */

void
ImageViewerManagerViewGUI
::LinkSetup()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
  {
    // no image selected, return
    return;
  }
  if(guiDiaporama->shown() == 0 )
    guiLinkSetupWindow->show();
}


/**
 *
 */
void
ImageViewerManagerViewGUI
::UpdateLinkSetupWindow()
{
  itk::OStringStream oss;
  oss.str("");
  guiLinkListLeft->clear();
  guiLinkListRight->clear();
  guiLinkListLeft->value(0);
  guiLinkListRight->value(0);

  //Fill the guiLinkList

  for (int i = 0 ; i < guiImageList->size() ; i++)
    {
      oss.str("");
      oss<<this->CutFileName(i);
      guiLinkListRight->add(oss.str().c_str());
      guiLinkListLeft->add(oss.str().c_str());
    }

  guiLinkXOffset->value("0");
  guiLinkYOffset->value("0");

  guiLinkListLeft->redraw();
  guiLinkListRight->redraw();
}

/**
*
*/
void
ImageViewerManagerViewGUI
::LinkSetupSave()
{
  unsigned int leftChoice = guiLinkListLeft->value();
  unsigned int rightChoice = guiLinkListRight->value();

  if(leftChoice == 0 || rightChoice == 0)
    return;
  else
    {
      //Fill the offset
      OffsetType                      offSet;
      offSet[0] = atoi(guiLinkXOffset->value());
      offSet[1] = atoi(guiLinkYOffset->value());

      //Update a list to

      //Call the controller
      this->m_ImageViewerManagerController->Link(leftChoice, rightChoice,offSet);

      //Close the temporary Showed images without clearing the showed list
      this->CloseAllDisplayedImages(false);

      //Add the linked image to the list
      m_LinkedDisplayStatusList[leftChoice-1] = true;
      m_LinkedDisplayStatusList[rightChoice-1] = true;

      //store  the images linked together
      UIntPairType        linkPair(leftChoice,rightChoice);
      m_LinkedImageList.push_back(linkPair);

      //Diplay the Linked images
      this->Display(m_LinkWidgetManagerList, leftChoice);
      this->Display(m_LinkWidgetManagerList, rightChoice);
    }
}

/**
*
*/
void
ImageViewerManagerViewGUI
::LinkSetupOk()
{
  guiLinkSetupWindow->hide();

  //Close the  displays linked
  for(unsigned int i = 0; i<m_LinkedDisplayStatusList.size() ; i++)
    {
      if(m_LinkedDisplayStatusList[i])
	{
	  m_LinkWidgetManagerList->GetNthElement(i)->Hide();
	  m_LinkedDisplayStatusList[i] = false;
	}
    }
  //Display temporary closed displays if any
  if(guiImageList->size()>0)
    this->ShowTemporaryClosedDisplay();
}

/**
 *
 */
void
ImageViewerManagerViewGUI
::LinkSetupRemove()
{
  //Close the  displays linked
  for(unsigned int i = 0; i<m_LinkedDisplayStatusList.size() ; i++)
    if(m_LinkedDisplayStatusList[i])
      {
	m_LinkWidgetManagerList->GetNthElement(i)->Hide();
	m_LinkedDisplayStatusList[i] = false;
      }
}

/**
 *
 */
void
ImageViewerManagerViewGUI
::InitializeImageController(unsigned int selectedItem)
{
  std::vector<unsigned int>  tempElementToRemove;

  for(unsigned int i = 0 ; i< m_LinkedImageList.size(); i++ )
    {
      if(m_LinkedImageList[i].first == selectedItem || m_LinkedImageList[i].second == selectedItem)
	{

	  if(m_LinkedImageList[i].first == selectedItem )
	    m_ImageViewerManagerController->UpdateImageViewController(m_LinkedImageList[i].second);

	  if(m_LinkedImageList[i].second == selectedItem)
	    m_ImageViewerManagerController->UpdateImageViewController(m_LinkedImageList[i].first);

	  tempElementToRemove.push_back(i);
	}
    }

  //Remove element from the list
  unsigned counter = 0;
  for(unsigned int p = 0; p < tempElementToRemove.size() ; p ++ )
    {
      m_LinkedImageList.erase(m_LinkedImageList.begin()+tempElementToRemove[p] - counter); // counter because the size of the list is decreasing after each iteration
      counter++;
    }

  m_LinkedImageList.clear();
}


/**
 * PrintSelf Method
 */
void
ImageViewerManagerViewGUI
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // End namespace otb

