OTB_CREATE_APPLICATION(NAME           ComputeImagesStatistics
                       SOURCES        otbComputeImagesStatistics.cxx
                       LINK_LIBRARIES OTBIO;OTBCommon;OTBBasicFilters)

OTB_CREATE_APPLICATION(NAME           ImageSVMClassifier
                       SOURCES        otbImageSVMClassifier.cxx
                       LINK_LIBRARIES OTBIO;OTBCommon;OTBBasicFilters;OTBFeatureExtraction;OTBLearning)

OTB_CREATE_APPLICATION(NAME           KMeansClassification
                       SOURCES        otbKMeansClassification.cxx
                       LINK_LIBRARIES OTBIO;OTBCommon;OTBBasicFilters;OTBFeatureExtraction;OTBLearning)

OTB_CREATE_APPLICATION(NAME           TrainSVMImagesClassifier
                       SOURCES        otbTrainSVMImagesClassifier.cxx
                       LINK_LIBRARIES OTBIO;OTBCommon;OTBBasicFilters;OTBFeatureExtraction;OTBLearning)

OTB_CREATE_APPLICATION(NAME           ValidateSVMImagesClassifier
                       SOURCES        otbValidateSVMImagesClassifier.cxx
                       LINK_LIBRARIES OTBIO;OTBCommon;OTBBasicFilters;OTBFeatureExtraction;OTBLearning)                                            

OTB_CREATE_APPLICATION(NAME           SOMClassification
                       SOURCES        otbSOMClassification.cxx
                       LINK_LIBRARIES OTBIO;OTBCommon;OTBBasicFilters;OTBFeatureExtraction;OTBLearning)                                            