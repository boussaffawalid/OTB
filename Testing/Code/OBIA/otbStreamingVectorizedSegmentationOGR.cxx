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

#include "otbVectorImage.h"
#include "otbStreamingVectorizedSegmentationOGR.h"
#include "otbImageFileReader.h"

#include "otbMeanShiftVectorImageFilter.h"
#include "otbMeanShiftImageFilter.h"


#include "otbPersistentImageToOGRDataFilter.h"
#include "otbPersistentFilterStreamingDecorator.h"

#include "otbOGRDataSourceWrapper.h"

int otbStreamingVectorizedSegmentationOGRNew(int argc, char * argv[])
{
  typedef float InputPixelType;
  const unsigned int Dimension = 2;

  /** Typedefs */
  typedef otb::Image<InputPixelType,  Dimension>          ImageType;
  typedef otb::MeanShiftImageFilter<ImageType, ImageType> MeanShiftImageFilterType;
  typedef otb::StreamingVectorizedSegmentationOGR<ImageType, MeanShiftImageFilterType>::FilterType StreamingVectorizedSegmentationOGRType;

  StreamingVectorizedSegmentationOGRType::Pointer filter = StreamingVectorizedSegmentationOGRType::New();

  std::cout << filter << std::endl;

  return EXIT_SUCCESS;
}

int otbStreamingVectorizedSegmentationOGR(int argc, char * argv[])
{

  if (argc != 11)
    {
      std::cerr << "Usage: " << argv[0];
      std::cerr << " inputImage maskImage outputVec layerName TileDimension spatialRadius rangeRadius minObjectSize filterSmallObj minSize" << std::endl;
      return EXIT_FAILURE;
    }

  const char * imageName                    = argv[1];
  const char * maskName                     = argv[2];
  const char * dataSourceName               = argv[3];
  const char * layerName                    = argv[4];
  const unsigned int tileSize               = atoi(argv[5]);
  const unsigned int spatialRadiusOldMS     = atoi(argv[6]);
  const double rangeRadiusOldMS             = atof(argv[7]);
  const unsigned int minimumObjectSizeOldMS = atoi(argv[8]);
  const bool filterSmallObj                 = atoi(argv[9]);
  const unsigned int minSize                = atoi(argv[10]);


  typedef float InputPixelType;
  const unsigned int Dimension = 2;
  const std::string fieldName("DN");

  // Typedefs
  typedef otb::VectorImage<InputPixelType,  Dimension>    ImageType;
  typedef otb::Image<unsigned int, Dimension>             LabelImageType;

  //old mean shift filter
  typedef otb::MeanShiftVectorImageFilter<ImageType, ImageType, LabelImageType> SegmentationFilterType;
  typedef otb::StreamingVectorizedSegmentationOGR<ImageType, SegmentationFilterType> StreamingVectorizedSegmentationOGRType;
  typedef otb::ImageFileReader<ImageType>                      ReaderType;
  typedef otb::ImageFileReader<LabelImageType>                 MaskReaderType;

  ReaderType::Pointer             reader = ReaderType::New();
  MaskReaderType::Pointer         maskReader = MaskReaderType::New();
  StreamingVectorizedSegmentationOGRType::Pointer filter = StreamingVectorizedSegmentationOGRType::New();

  reader->SetFileName(imageName);
  reader->UpdateOutputInformation();
  
  maskReader->SetFileName(maskName);
  maskReader->UpdateOutputInformation();
  
  otb::ogr::DataSource::Pointer ogrDS = otb::ogr::DataSource::New(dataSourceName, otb::ogr::DataSource::Modes::write);

  filter->SetInput(reader->GetOutput());
  //filter->SetInputMask(maskReader->GetOutput());
  filter->SetOGRDataSource(ogrDS);
  //filter->GetStreamer()->SetNumberOfLinesStrippedStreaming(atoi(argv[3]));
  filter->GetStreamer()->SetTileDimensionTiledStreaming(tileSize);
  filter->SetLayerName(layerName);
  filter->SetFieldName(fieldName);
  filter->SetStartLabel(1);
  filter->SetUse8Connected(false);
  filter->SetFilterSmallObject(filterSmallObj);
  filter->SetMinimumObjectSize(minSize);
  filter->GetSegmentationFilter()->SetSpatialRadius(spatialRadiusOldMS);
  filter->GetSegmentationFilter()->SetRangeRadius(rangeRadiusOldMS);
  filter->GetSegmentationFilter()->SetMinimumRegionSize(minimumObjectSizeOldMS);


  filter->Initialize(); //must be called !

  filter->Update();

  return EXIT_SUCCESS;
}
