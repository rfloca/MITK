MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(mitkImageReconstructionTest mitkImageReconstructionTest ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test.dwi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_dti.dti ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QN0.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA0.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA6.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA4.qbi ${MITK_DATA_DIR}/DiffusionImaging/ImageReconstruction/test_QA5.qbi)
