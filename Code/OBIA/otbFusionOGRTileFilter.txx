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
#ifndef __otbFusionOGRTileFilter_txx
#define __otbFusionOGRTileFilter_txx

#include "otbFusionOGRTileFilter.h"

#include "ogrsf_frmts.h"
#include <iomanip>
#include "itkTimeProbe.h"


namespace otb
{

template<class TImage>
FusionOGRTileFilter<TImage>
::FusionOGRTileFilter() : m_Radius(2), m_LayerName("Layer")
{
   m_StreamSize.Fill(0);
}

template <class TInputImage>
void
FusionOGRTileFilter<TInputImage>
::SetInput(const InputImageType *input)
{
  this->Superclass::SetNthInput(0, const_cast<InputImageType *>(input));
}

template <class TInputImage>
const typename FusionOGRTileFilter<TInputImage>
::InputImageType *
FusionOGRTileFilter<TInputImage>
::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
    {
    return 0;
    }

  return static_cast<const InputImageType *>(this->Superclass::GetInput(0));
}

template<class TInputImage>
void
FusionOGRTileFilter<TInputImage>
::SetOGRDataSource( OGRDataSourcePointerType ogrDS )
{
   this->itk::ProcessObject::SetNthInput(1, ogrDS);
}

template<class TInputImage>
typename FusionOGRTileFilter<TInputImage>::OGRDataSourceType *
FusionOGRTileFilter<TInputImage>
::GetOGRDataSource( void )
{
   return static_cast<OGRDataSourceType *> (this->itk::ProcessObject::GetInput(1));
}


template<class TInputImage>
void
FusionOGRTileFilter<TInputImage>
::ProcessStreamingLine( bool line )
{
   typename InputImageType::ConstPointer inputImage = this->GetInput();
   OGRDataSourcePointerType inputDataSource = this->GetOGRDataSource();
   OGRLayerType inputLayer = inputDataSource->GetLayerChecked(m_LayerName);
   
   //compute the number of stream division in row and column
   SizeType imageSize = this->GetInput()->GetLargestPossibleRegion().GetSize();
   unsigned int nbRowStream = static_cast<unsigned int>(imageSize[1] / m_StreamSize[1] + 1);
   unsigned int nbColStream = static_cast<unsigned int>(imageSize[0] / m_StreamSize[0] + 1);
   
   
   for(unsigned int x=1; x<=nbColStream; x++)
   {
      inputLayer.ogr().StartTransaction();
      for(unsigned int y=1; y<=nbRowStream; y++)
      {
         //First we get all the feature that intersect the streaming line of the Upper/left stream
         std::vector<FeatureStruct> upperStreamFeatureList;
         upperStreamFeatureList.clear();
         IndexType  UpperLeftCorner;
         IndexType  LowerRightCorner;
         
         if(!line)
         {
            // Treat Row stream
            //Compute the spatial filter of the upper stream
            UpperLeftCorner[0] = x*m_StreamSize[0] - 1 - m_Radius;
            UpperLeftCorner[1] = m_StreamSize[1]*(y-1) + 1;

            LowerRightCorner[0] = m_StreamSize[0]*x - 1 - 1;
            LowerRightCorner[1] = m_StreamSize[1]*y - 1 - 1;
         }
         else
         {  // Treat Column stream
            //Compute the spatial filter of the left stream
            UpperLeftCorner[0] = (x-1)*m_StreamSize[0] + 1;
            UpperLeftCorner[1] = m_StreamSize[1]*y - 1 - 1 - m_Radius;
            
            LowerRightCorner[0] = m_StreamSize[0]*x - 1 - 1;
            LowerRightCorner[1] = m_StreamSize[1]*y - 1 - 1; //-1 to stop just before stream line
         }

         OriginType  ulCorner;
         inputImage->TransformIndexToPhysicalPoint(UpperLeftCorner, ulCorner);
         OriginType  lrCorner;
         inputImage->TransformIndexToPhysicalPoint(LowerRightCorner, lrCorner);
         
         inputLayer.SetSpatialFilterRect(ulCorner[0],lrCorner[1],lrCorner[0],ulCorner[1]);
         
         OGRLayerType::const_iterator featIt = inputLayer.begin();
         for(; featIt!=inputLayer.end(); ++featIt)
         {
            FeatureStruct s(inputLayer.GetLayerDefn());
            s.feat = *featIt;
            s.fusioned = false;
            upperStreamFeatureList.push_back(s);
         }
         
         //Do the same thing for the lower/right stream
         std::vector<FeatureStruct> lowerStreamFeatureList;
         lowerStreamFeatureList.clear();

         if(!line)
         {
            //Compute the spatial filter of the lower stream
            UpperLeftCorner[0] = x*m_StreamSize[0] + 1;
            UpperLeftCorner[1] = m_StreamSize[1]*(y-1) + 1;
            
            LowerRightCorner[0] = m_StreamSize[0]*x + 1 + m_Radius;
            LowerRightCorner[1] = m_StreamSize[1]*y - 1 - 1;
         }
         else
         {
            //Compute the spatial filter of the right stream
            UpperLeftCorner[0] = (x-1)*m_StreamSize[0] + 1;
            UpperLeftCorner[1] = m_StreamSize[1]*y + 1;
            
            LowerRightCorner[0] = m_StreamSize[0]*x - 1 - 1;
            LowerRightCorner[1] = m_StreamSize[1]*y + 1 + m_Radius;
         }
         
         inputImage->TransformIndexToPhysicalPoint(UpperLeftCorner, ulCorner);
         inputImage->TransformIndexToPhysicalPoint(LowerRightCorner, lrCorner);
         
         inputLayer.SetSpatialFilterRect(ulCorner[0],lrCorner[1],lrCorner[0],ulCorner[1]);
         
         for(featIt = inputLayer.begin(); featIt!=inputLayer.end(); ++featIt)
         {
            FeatureStruct s(inputLayer.GetLayerDefn());
            s.feat = *featIt;
            s.fusioned = false;
            lowerStreamFeatureList.push_back(s);
         }
      
         unsigned int nbUpperPolygons = upperStreamFeatureList.size();
         unsigned int nbLowerPolygons = lowerStreamFeatureList.size();
         std::vector<FusionStruct> fusionList;
         fusionList.clear();
         for(unsigned int u=0; u<nbUpperPolygons; u++)
         {
            for(unsigned int l=0; l<nbLowerPolygons; l++)
            {
               FeatureStruct upper = upperStreamFeatureList[u];
               FeatureStruct lower = lowerStreamFeatureList[l];
               if (ogr::Intersects(*upper.feat.GetGeometry(), *lower.feat.GetGeometry()))
               {
                  ogr::UniqueGeometryPtr intersection = ogr::Intersection(*upper.feat.GetGeometry(),*lower.feat.GetGeometry());
                  if (intersection)
                  {
                     FusionStruct fusion;
                     fusion.indStream1 = u;
                     fusion.indStream2 = l;
                     fusion.overlap = 0.;
                     
                     if(intersection->getGeometryType() == wkbPolygon)
                     {
                        fusion.overlap = dynamic_cast<OGRPolygon *>(intersection.get())->get_Area();
                     }
                     else if(intersection->getGeometryType() == wkbMultiPolygon)
                     {
                        fusion.overlap = dynamic_cast<OGRMultiPolygon *>(intersection.get())->get_Area();
                     }
                     else if(intersection->getGeometryType() == wkbGeometryCollection)
                     {
                        fusion.overlap = dynamic_cast<OGRGeometryCollection *>(intersection.get())->get_Area();
                     }
                     else if(intersection->getGeometryType() == wkbLineString)
                     {
                        fusion.overlap = dynamic_cast<OGRLineString *>(intersection.get())->get_Length();
                     }
                     else if (intersection->getGeometryType() == wkbMultiLineString)
                     {
                        fusion.overlap = dynamic_cast<OGRMultiLineString *>(intersection.get())->get_Length();
                     }
                     
                     long upperFID = upper.feat.GetFID();
                     long lowerFID = lower.feat.GetFID();
                     fusionList.push_back(fusion);
                  }
               }
            }
         }
         unsigned int fusionListSize = fusionList.size();
         std::sort(fusionList.begin(),fusionList.end(),SortFeature);
         for(unsigned int i=0; i<fusionListSize; i++)
         {
            FeatureStruct upper = upperStreamFeatureList.at(fusionList.at(i).indStream1);
            FeatureStruct lower = lowerStreamFeatureList.at(fusionList.at(i).indStream2);
            if( !upper.fusioned && !lower.fusioned )
            {
               upperStreamFeatureList[fusionList[i].indStream1].fusioned = true;
               lowerStreamFeatureList[fusionList[i].indStream2].fusioned = true;
               ogr::UniqueGeometryPtr fusionPolygon = ogr::Union(*upper.feat.GetGeometry(),*lower.feat.GetGeometry());
               OGRFeatureType fusionFeature(inputLayer.GetLayerDefn());
               fusionFeature.SetGeometry( fusionPolygon.get() );
               
               ogr::Field field = upper.feat[0];
               fusionFeature[0].SetValue(field.GetValue<int>());
               inputLayer.CreateFeature(fusionFeature);
               inputLayer.DeleteFeature(lower.feat.GetFID());
               inputLayer.DeleteFeature(upper.feat.GetFID());
            }
         }
      } //end for x
      inputLayer.ogr().CommitTransaction();
   } //end for y
   inputLayer.ogr().CommitTransaction();

}

template<class TImage>
void
FusionOGRTileFilter<TImage>
::GenerateData(void)
{
   //Process column
   this->ProcessStreamingLine(false);
   //Process row
   this->ProcessStreamingLine(true);
   

}


} // end namespace otb

#endif
