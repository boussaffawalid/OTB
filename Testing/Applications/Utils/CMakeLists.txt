
#--- Rescale ---#
OTB_TEST_APPLICATION(NAME  apTvUtRescaleTest
                     APP  Rescale
                     OPTIONS -in ${INPUTDATA}/poupees.tif
                             -out ${TEMP}/apTvUtRescaleTest.png uint8
                             -outmin 20
                             -outmax 150
                     VALID   --compare-image ${NOTOL}
                             ${BASELINE}/apTvUtRescaleTest.png
                             ${TEMP}/apTvUtRescaleTest.png)

#--- Smoothing ---#
OTB_TEST_APPLICATION(NAME  apTvUtSmoothingTest
                     APP  Smoothing
                     OPTIONS -in ${INPUTDATA}/poupees.tif
               	             -out ${TEMP}/apTvUtSmoothingTest.tif
                             -type mean
                     VALID   --compare-image ${NOTOL}
                             ${BASELINE}/apTvUtSmoothingTest.tif
                             ${TEMP}/apTvUtSmoothingTest.tif)
#--- ReadImageInfo ---#

OTB_TEST_APPLICATION(NAME  apTvUtReadImageInfoTest
                     APP  ReadImageInfo
                     OPTIONS -in ${INPUTDATA}/poupees.tif  
                     TESTENVOPTIONS ${TEMP}/apTvUtReadImageInfoTest.txt
                     VALID   --compare-ascii ${EPSILON_7}
                             ${BASELINE_FILES}/apTvUtReadImageInfoTest.txt
                	     ${TEMP}/apTvUtReadImageInfoTest.txt)

IF(OTB_DATA_USE_LARGEINPUT)

OTB_TEST_APPLICATION(NAME  apTvUtReadImageInfoSpot5
                     APP  ReadImageInfo
                     OPTIONS -in ${LARGEDATA}/SPOT5/TEHERAN/IMAGERY.TIF  
                     TESTENVOPTIONS ${TEMP}/apTvUtReadImageInfoSpot5.txt
                     VALID   --compare-ascii ${EPSILON_7}
                             ${BASELINE_FILES}/apTvUtReadImageInfoSpot5.txt
                	     ${TEMP}/apTvUtReadImageInfoSpot5.txt)
                     
OTB_TEST_APPLICATION(NAME  apTvUtReadImageInfoERS
                     APP  ReadImageInfo
                     OPTIONS -in ${LARGEDATA}/SAR_ERS2_SLCI_SCENE1/DAT_01.001  
                     TESTENVOPTIONS ${TEMP}/apTvUtReadImageInfoERS.txt
                     VALID   --compare-ascii ${EPSILON_7}
                             ${BASELINE_FILES}/apTvUtReadImageInfoERS.txt
                	     ${TEMP}/apTvUtReadImageInfoERS.txt)

OTB_TEST_APPLICATION(NAME  apTvUtReadImageInfoRADARSAT1
                     APP  ReadImageInfo
                     OPTIONS -in ${LARGEDATA}/RADARSAT1/GOMA/SCENE01/DAT_01.001  
                     TESTENVOPTIONS ${TEMP}/apTvUtReadImageInfoRADARSAT1.txt
                     VALID   --compare-ascii ${EPSILON_7}
                             ${BASELINE_FILES}/apTvUtReadImageInfoRADARSAT1.txt
                	     ${TEMP}/apTvUtReadImageInfoRADARSAT1.txt)

ENDIF(OTB_DATA_USE_LARGEINPUT)

#--- ExtractROI ---#

OTB_TEST_APPLICATION(NAME apTvUtExtractROIRightInputFile
                     APP  ExtractROI
                     OPTIONS -in ${INPUTDATA}/couleurs_extrait.png
                             -out ${TEMP}/apTvUtExtractROIRightInputFile.tif
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/couleurs_extrait.png
                             ${TEMP}/apTvUtExtractROIRightInputFile.tif)

OTB_TEST_APPLICATION(NAME apTvUtExtractROISizeNULL
                     APP  ExtractROI
                     OPTIONS -in ${INPUTDATA}/couleurs_extrait.png
                 			 -out ${TEMP}/apTvUtExtractROISizeNULL.tif
                 			 -sizex 0
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/couleurs_extrait.png
                             ${TEMP}/apTvUtExtractROISizeNULL.tif)
                             
OTB_TEST_APPLICATION(NAME apTvUtExtractROISizeTooBig
                     APP  ExtractROI
                     OPTIONS -in ${INPUTDATA}/couleurs_extrait.png
                 			 -out ${TEMP}/apTvUtExtractROISizeTooBig.tif
                 			 -sizex 1000000 
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/couleurs_extrait.png
                             ${TEMP}/apTvUtExtractROISizeTooBig.tif)


#--- ConcatenateImages ---#
OTB_TEST_APPLICATION(NAME apTvUtConcatenateImages_1Image
                     APP  ConcatenateImages
                     OPTIONS -il ${INPUTDATA}/poupees_c1
                             -out ${TEMP}/apTvUtConcatenateImages_1Image.tif
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/poupees_c1
                             ${TEMP}/apTvUtConcatenateImages_1Image.tif)

OTB_TEST_APPLICATION(NAME apTvUtConcatenateImages
                     APP  ConcatenateImages
                     OPTIONS -il ${INPUTDATA}/poupees_sub_c1.png 
                                  ${INPUTDATA}/poupees_sub_c2.png 
                                  ${INPUTDATA}/poupees_sub_c3.png
                 			 -out ${TEMP}/apTvUtConcatenateImages.png
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/poupees_sub_3c.png
                             ${TEMP}/apTvUtConcatenateImages.png)

#--- BandMath application -----

OTB_TEST_APPLICATION(NAME apTvUtBandMath
                     APP  BandMath
                     OPTIONS -il ${INPUTDATA}/poupees_sub_c1.png 
                                  ${INPUTDATA}/poupees_sub_c2.png 
                                  ${INPUTDATA}/poupees_sub.png
                             -out ${TEMP}/apTvUtBandMathOutput.tif
                             -exp "cos(im1b1)+im2b1*im3b1-im3b2+ndvi(im3b3,im3b4)"
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/apTvUtBandMathOutput.tif
                             ${TEMP}/apTvUtBandMathOutput.tif)


#--- Split Image application ----

OTB_TEST_APPLICATION(NAME apTvUtSplitImage
                     APP SplitImage
                     OPTIONS -in  ${INPUTDATA}/poupees_sub.png
                             -out ${TEMP}/apTvUtSplitImageOutput.tif
                     VALID   --compare-n-images ${NOTOL} 3
                             ${INPUTDATA}/poupees_sub_c1.png
                             ${TEMP}/apTvUtSplitImageOutput_0.tif

                             ${INPUTDATA}/poupees_sub_c2.png
                             ${TEMP}/apTvUtSplitImageOutput_1.tif

                             ${INPUTDATA}/poupees_sub_c3.png
                             ${TEMP}/apTvUtSplitImageOutput_2.tif)

#--- Convert ----
OTB_TEST_APPLICATION(NAME apTvUtConvertBasic
                     APP Convert
                     OPTIONS -in ${INPUTDATA}/QB_Toulouse_Ortho_XS.tif
                             -out ${TEMP}/apTvUtConvertBasicOutput.tif
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/apTvUtConvertBasicOutput.tif
                	     ${TEMP}/apTvUtConvertBasicOutput.tif
)

OTB_TEST_APPLICATION(NAME apTvUtConvertWithScaling
                     APP Convert
                     OPTIONS -in ${INPUTDATA}/QB_Toulouse_Ortho_XS.tif
                             -out ${TEMP}/apTvUtConvertWithScalingOutput.tif
                             -type linear
                     VALID   --compare-image ${NOTOL}
                             ${INPUTDATA}/apTvUtConvertWithScalingOutput.tif
                	     ${TEMP}/apTvUtConvertWithScalingOutput.tif
)

#--- KmzExport ----
OTB_TEST_APPLICATION(NAME apTvUtKmzExport
                     APP KmzExport
                     OPTIONS -in   ${INPUTDATA}/qb_RoadExtract.img
                             -out ${TEMP}/apTvUtKmzExport.kmz
                             -logo ${INPUTDATA}/cnes.png
                             -legend ${INPUTDATA}/otb_big.png
                             -elev dem
                             -elev.dem.path ${INPUTDATA}/DEM/srtm_directory)


IF(OTB_DATA_USE_LARGEINPUT)
#--- OSMDownloader ---
OTB_TEST_APPLICATION(NAME apTvUtOSMDownloader
                     APP OSMDownloader
                     OPTIONS -support ${OTB_DATA_LARGEINPUT_ROOT}/DEMPSTER-SHAFER/ROI_QB_TOULOUSE.TIF
                             -key highway
                             -out ${TEMP}/apTvUtOSMDownloader.shp
                     VALID   --compare-ogr ${NOTOL}  
                             ${OTBAPP_BASELINE_FILES}/otbOSMDownloaderOutput.shp
                             ${TEMP}/apTvUtOSMDownloader.shp)

#--- VectorDataTransform ---
OTB_TEST_APPLICATION(NAME apTvUtVectorDataTransform_Rotation
                     APP VectorDataTransform
                     OPTIONS -in ${OTB_DATA_LARGEINPUT_ROOT}/QUICKBIRD/TOULOUSE/000000128955_01_P001_MUL/02APR01105228-M1BS-000000128955_01_P001.TIF
                             -vd ${INPUTDATA}/ToulouseRoad-examples.shp
                             -out ${TEMP}/apTvUtVectorDataTransform_rotation.shp
                             -transform.ro 15
                     VALID   --compare-ogr ${NOTOL}  
                             ${OTBAPP_BASELINE_FILES}/utTvVectorDataTransformFilter_Rotation.shp
                             ${TEMP}/apTvUtVectorDataTransform_rotation.shp)

OTB_TEST_APPLICATION(NAME apTvUtVectorDataTransform_Translation_Rotation
                     APP VectorDataTransform
                     OPTIONS -in ${OTB_DATA_LARGEINPUT_ROOT}/QUICKBIRD/TOULOUSE/000000128955_01_P001_MUL/02APR01105228-M1BS-000000128955_01_P001.TIF
                             -vd ${INPUTDATA}/ToulouseRoad-examples.shp
                             -out ${TEMP}/apTvUtVectorDataTransform_Translation_rotation.shp
                             -transform.ro 15
                             -transform.tx  5
                             -transform.ty  6
                     VALID   --compare-ogr ${NOTOL}  
                             ${OTBAPP_BASELINE_FILES}/utTvVectorDataTransformFilter_Translation_rotation.shp
                             ${TEMP}/apTvUtVectorDataTransform_Translation_rotation.shp)

#--- VectorDataExtractROIApplication ---
OTB_TEST_APPLICATION(NAME apTvUtVectorDataExtractROIApplication
                     APP VectorDataExtractROIApplication
                     OPTIONS -io.in ${INPUTDATA}/QB_Toulouse_Ortho_XS.tif
                             -io.vd ${OTB_DATA_LARGEINPUT_ROOT}/VECTOR/MidiPyrenees/roads.shp
                             -io.out ${TEMP}/apTvUtVectorDataExtractROIApplicationTest.shp
                     VALID   --compare-ogr ${NOTOL}
                             ${BASELINE_FILES}/apTvUtVectorDataExtractROIApplicationTest.shp
                             ${TEMP}/apTvUtVectorDataExtractROIApplicationTest.shp
                     )

ENDIF(OTB_DATA_USE_LARGEINPUT)

#--- Concatenate VectorData ---
OTB_TEST_APPLICATION(NAME apTvUtConcatenateVectorData
                     APP ConcatenateVectorData
                     OPTIONS -vd   ${INPUTDATA}/waterways.shp
                                    ${INPUTDATA}/france_coastline.shp
                             -out ${TEMP}/apTvUtConcatenateVectorDataOutputTest.shp
                     VALID   --compare-ogr ${NOTOL}
                             ${OTBAPP_BASELINE_FILES}/utTvConcatenateVectorData.shp
                             ${TEMP}/apTvUtConcatenateVectorDataOutputTest.shp)

#--- CompareImages ---
OTB_TEST_APPLICATION(NAME apTvUtCompareImages
                     APP CompareImages
                     OPTIONS -ref.in ${INPUTDATA}/poupees.tif
                             -ref.channel 2
                             -meas.in ${INPUTDATA}/brain.png
                             -meas.channel 1
                             -roi.startx 20
                             -roi.starty 15
                             -roi.sizex 150
                             -roi.sizey 200
                             TESTENVOPTIONS ${TEMP}/apTvUtCompareImages.txt
                     VALID   --compare-ascii ${EPSILON_7}
                             ${BASELINE_FILES}/apTvUtCompareImages.txt
                	     ${TEMP}/apTvUtCompareImages.txt)

#--- VectorDataSetField ---
OTB_TEST_APPLICATION(NAME apTvUtVectorDataSetField
                     APP VectorDataSetField
                     OPTIONS -in ${INPUTDATA}/ToulousePoints-examples.shp
                             -out ${TEMP}/apTvUtVectorDataSetFieldTest.shp
                             -fn MyField
                             -fv MyValue
                     VALID   --compare-ogr ${NOTOL}
                             ${BASELINE_FILES}/apTvUtVectorDataSetFieldTest.shp
                             ${TEMP}/apTvUtVectorDataSetFieldTest.shp
                     )

#--- Quicklook ---
OTB_TEST_APPLICATION(NAME apTvUtQuicklookROI1Channel
                     APP Quicklook
                     OPTIONS -in ${INPUTDATA}/couleurs_extrait.png
                             -out ${TEMP}/apTvUtQuicklookROI1Channel.tif
                             -cl Channel1
                     VALID   --compare-image ${NOTOL}
                             ${BASELINE}/apTvUtQuicklookROI1Channel.tif
                             ${TEMP}/apTvUtQuicklookROI1Channel.tif
                     )

IF(OTB_DATA_USE_LARGEINPUT)
OTB_TEST_APPLICATION(NAME apTvUtQuicklookSpot5
                     APP Quicklook
                     OPTIONS -in ${LARGEDATA}/SPOT5/TEHERAN/IMAGERY.TIF
                             -out ${TEMP}/apTvUtQuicklookSpot5.tif
                             -cl Channel1 Channel2 Channel3
                             -rox 10
                             -roy 10
                             -rsx 100
                             -rsy 200
                             -sr 2
                     VALID   --compare-image ${NOTOL}
                             ${BASELINE}/apTvUtQuicklookSpot5.png
                             ${TEMP}/apTvUtQuicklookSpot5.tif
                     )
ENDIF(OTB_DATA_USE_LARGEINPUT)

#--- Pixel Value ---
OTB_TEST_APPLICATION(NAME apTvUtPixelValue
                     APP PixelValue
                     OPTIONS -in ${INPUTDATA}/QB_Toulouse_Ortho_XS.tif
                             -coordx 30
                             -coordy 30
                             -cl Channel1 Channel3 Channel4
                       TESTENVOPTIONS ${TEMP}/apTvUtPixelValue.txt
                       VALID --compare-ascii ${EPSILON_7}
                             ${BASELINE_FILES}/apTvUtPixelValue.txt
                	     ${TEMP}/apTvUtPixelValue.txt
                             )