
IF( OTB_USE_MAPNIK )
IF( OTB_DATA_USE_LARGEINPUT )

OTB_TEST_APPLICATION(NAME apTvRsRasterizationLines
                     APP  Rasterization
                     OPTIONS -in   ${LARGEDATA}/VECTOR/MidiPyrenees/roads.shp
                             -im   ${BASELINE}/apTvRsRasterizationLines.tif
                 			 -out  ${TEMP}/apTvRsRasterizationLines.tif uint8
                     VALID   --compare-image ${NOTOL}
                              ${BASELINE}/apTvRsRasterizationLines.tif
                			  ${TEMP}/apTvRsRasterizationLines.tif)

OTB_TEST_APPLICATION(NAME apTvRsRasterizationLinesStreaming
                     APP  Rasterization
                     OPTIONS -in   ${LARGEDATA}/VECTOR/MidiPyrenees/roads.shp
                             -im   ${BASELINE}/apTvRsRasterizationLines.tif
                 			 -out  ${TEMP}/apTvRsRasterizationLinesStreaming.tif uint8
                 			 -ram 1
                     VALID   --compare-image ${NOTOL}
                              ${BASELINE}/apTvRsRasterizationLines.tif
                			  ${TEMP}/apTvRsRasterizationLinesStreaming.tif)

OTB_TEST_APPLICATION(NAME apTvRsRasterizationPoints
                     APP  Rasterization
                     OPTIONS -in   ${LARGEDATA}/VECTOR/MidiPyrenees/points.shp
                             -im   ${BASELINE}/apTvRsRasterizationPoints.tif
                 			 -out  ${TEMP}/apTvRsRasterizationPoints.tif uint8
                     VALID   --compare-image ${NOTOL}
                              ${BASELINE}/apTvRsRasterizationPoints.tif
                			  ${TEMP}/apTvRsRasterizationPoints.tif)

ENDIF( OTB_DATA_USE_LARGEINPUT )

OTB_TEST_APPLICATION(NAME apTvRsRasterizationPolygons
                     APP  Rasterization
                     OPTIONS -in   ${INPUTDATA}/Rasterization/polgons.shp
                             -szx 1100
                             -szy 1100
                 			 -out  ${TEMP}/apTvRsRasterizationPolygons.tif uint8
                     VALID   --compare-image ${NOTOL}
                              ${BASELINE}/apTvRsRasterizationPolygons.tif
                			  ${TEMP}/apTvRsRasterizationPolygons.tif)

ENDIF( OTB_USE_MAPNIK )