/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPluginActivator.h"
#include "DicomEventHandler.h"
#include <service/event/ctkEventConstants.h>
#include <ctkDictionary.h>
#include <mitkLogMacros.h>
#include <mitkDicomSeriesReader.h>
#include <mitkDataNode.h>
#include <mitkIDataStorageService.h>
#include <service/event/ctkEventAdmin.h>
#include <ctkServiceReference.h>
#include <mitkRenderingManager.h>
#include <QVector>
#include "mitkImage.h"

#include <mitkContourModelSet.h>

#include <mitkRTDoseReader.h>
#include <mitkRTStructureSetReader.h>
#include <mitkRTConstants.h>
#include <mitkIsoDoseLevelCollections.h>
#include <mitkIsoDoseLevelSetProperty.h>
#include <mitkIsoDoseLevelVectorProperty.h>
#include <mitkDoseImageVtkMapper2D.h>
#include <mitkRTUIConstants.h>
#include <mitkIsoLevelsGenerator.h>

#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingModeProperty.h>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

DicomEventHandler::DicomEventHandler()
{
}

DicomEventHandler::~DicomEventHandler()
{
}

void DicomEventHandler::OnSignalAddSeriesToDataManager(const ctkEvent& ctkEvent)
{
  QStringList listOfFilesForSeries;
  mitk::DicomSeriesReader::StringContainer seriesToLoad;

  listOfFilesForSeries = ctkEvent.getProperty("FilesForSeries").toStringList();

  if (!listOfFilesForSeries.isEmpty()){

    //for rt data, if the modality tag isnt defined or is "CT" the image is handled like before
    if(ctkEvent.containsProperty("Modality") &&
       (ctkEvent.getProperty("Modality").toString().compare("RTDOSE",Qt::CaseInsensitive) == 0 ||
        ctkEvent.getProperty("Modality").toString().compare("RTSTRUCT",Qt::CaseInsensitive) == 0))
    {
      QString modality = ctkEvent.getProperty("Modality").toString();

      if(modality.compare("RTDOSE",Qt::CaseInsensitive) == 0)
      {
        mitk::RTDoseReader::Pointer doseReader = mitk::RTDoseReader::New();

        mitk::DataNode::Pointer doseImageNode = mitk::DataNode::New();
        mitk::DataNode::Pointer doseOutlineNode = mitk::DataNode::New();

        doseImageNode = doseReader->LoadRTDose(listOfFilesForSeries.at(0).toStdString().c_str());
        doseOutlineNode->SetData(doseImageNode->GetData());
        if(doseImageNode.IsNotNull() && doseOutlineNode->GetData() != NULL)
        {
          berry::IPreferencesService::Pointer prefService =
              berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
          berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);

          typedef std::vector<std::string> NamesType;
          NamesType names = prefNode->ChildrenNames();

          std::map<std::string, mitk::IsoDoseLevelSet::Pointer> presetMap;

          for (NamesType::const_iterator pos = names.begin(); pos != names.end(); ++pos)
          {
            berry::IPreferences::Pointer aPresetNode = prefNode->Node(*pos);

            if (aPresetNode.IsNull())
            {
              mitkThrow()<< "Error in preference interface. Cannot find preset node under given name. Name: "<<*pos;
            }

            mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

            NamesType levelNames =  aPresetNode->ChildrenNames();
            for (NamesType::const_iterator levelName = levelNames.begin(); levelName != levelNames.end(); ++levelName)
            {
              berry::IPreferences::Pointer levelNode = aPresetNode->Node(*levelName);
              if (aPresetNode.IsNull())
              {
                mitkThrow()<< "Error in preference interface. Cannot find level node under given preset name. Name: "<<*pos<<"; Level id: "<<*levelName;
              }

              mitk::IsoDoseLevel::Pointer isoLevel = mitk::IsoDoseLevel::New();

              isoLevel->SetDoseValue(levelNode->GetDouble(mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID,0.0));
              mitk::IsoDoseLevel::ColorType color;
              color.SetRed(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID,1.0));
              color.SetGreen(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID,1.0));
              color.SetBlue(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID,1.0));
              isoLevel->SetColor(color);
              isoLevel->SetVisibleIsoLine(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID,true));
              isoLevel->SetVisibleColorWash(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID,true));

              levelSet->SetIsoDoseLevel(isoLevel);
            }

            presetMap.insert(std::make_pair(*pos,levelSet));
          }

          if (presetMap.size() == 0)
          {
            presetMap.insert(std::make_pair(std::string("Virtuos"), mitk::GeneratIsoLevels_Virtuos()));
          }

          double referenceDose = 40.0;
          //set some specific colorwash and isoline properties
          doseImageNode->SetBoolProperty(mitk::Constants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), true);
          doseOutlineNode->SetBoolProperty(mitk::Constants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), true);
          //Set reference dose property
          doseImageNode->SetFloatProperty(mitk::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);
          doseOutlineNode->SetFloatProperty(mitk::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), referenceDose);

          berry::IPreferences::Pointer nameNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);
          std::string presetName = nameNode->Get(mitk::RTUIConstants::SELECTED_ISO_PRESET_ID,"");

          mitk::IsoDoseLevelSet::Pointer isoDoseLevelPreset = presetMap[presetName];
          mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(isoDoseLevelPreset);
          doseImageNode->SetProperty(mitk::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),levelSetProp);
          doseOutlineNode->SetProperty(mitk::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),levelSetProp);

          mitk::IsoDoseLevelVector::Pointer levelVector = mitk::IsoDoseLevelVector::New();
          mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(levelVector);
          doseImageNode->SetProperty(mitk::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);
          doseOutlineNode->SetProperty(mitk::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);

          //Generating the Colorwash
          vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
          mitk::TransferFunction::ControlPoints scalarOpacityPoints;
          scalarOpacityPoints.push_back( std::make_pair(0, 1 ) );

          //Backgroud
          transferFunction->AddHSVPoint(((isoDoseLevelPreset->Begin())->GetDoseValue()*referenceDose)-0.001,0,0,0,1.0,1.0);

          for(mitk::IsoDoseLevelSet::ConstIterator itIsoDoseLevel = isoDoseLevelPreset->Begin(); itIsoDoseLevel != isoDoseLevelPreset->End(); ++itIsoDoseLevel)
          {
            float *hsv = new float[3];
            //used for transfer rgb to hsv
            vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();
            if(itIsoDoseLevel->GetVisibleColorWash())
            {
              cCalc->RGBToHSV(itIsoDoseLevel->GetColor()[0],itIsoDoseLevel->GetColor()[1],itIsoDoseLevel->GetColor()[2],&hsv[0],&hsv[1],&hsv[2]);
              transferFunction->AddHSVPoint(itIsoDoseLevel->GetDoseValue()*referenceDose,hsv[0],hsv[1],hsv[2],1.0,1.0);
            }
            else
            {
              scalarOpacityPoints.push_back( std::make_pair(itIsoDoseLevel->GetDoseValue()*referenceDose, 1 ) );
            }
          }

          mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
          mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
          mitkTransFunc->SetColorTransferFunction(transferFunction);
          mitkTransFunc->SetScalarOpacityPoints(scalarOpacityPoints);
          mitkTransFuncProp->SetValue(mitkTransFunc);

          mitk::RenderingModeProperty::Pointer renderingModeProp = mitk::RenderingModeProperty::New();
          renderingModeProp->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);

          doseImageNode->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);
          doseImageNode->SetProperty("Image Rendering.Mode", renderingModeProp);
          doseImageNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

          //set the outline properties
          doseOutlineNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
          doseOutlineNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false) );
          mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
          renderingMode->SetValue( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR );
          doseOutlineNode->SetProperty("Image Rendering.Mode", renderingMode);
          doseOutlineNode->SetProperty("outline width", mitk::FloatProperty::New(1.5f));

          ctkServiceReference serviceReference =mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
          mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
          mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

          dataStorage->Add(doseImageNode);
          dataStorage->Add(doseOutlineNode, doseImageNode);

          //set the dose mapper for outline drawing; the colorwash is realized by the imagevtkmapper2D
          mitk::DoseImageVtkMapper2D::Pointer contourMapper = mitk::DoseImageVtkMapper2D::New();
          doseOutlineNode->SetMapper(1,contourMapper);

          mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
        }//END DOSE
      }
      else if(modality.compare("RTSTRUCT",Qt::CaseInsensitive) == 0)
      {
        mitk::RTStructureSetReader::Pointer structreader = mitk::RTStructureSetReader::New();
        std::deque<mitk::DataNode::Pointer> modelVector = structreader->ReadStructureSet(listOfFilesForSeries.at(0).toStdString().c_str());

        if(modelVector.empty())
        {
          MITK_ERROR << "No structuresets were created" << endl;
        }
        else
        {
          ctkServiceReference serviceReference =mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
          mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
          mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

          for(int i=0; i<modelVector.size();i++)
          {
            dataStorage->Add(modelVector.at(i));
          }
          mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
        }
      }
    }
    else
    {

      QStringListIterator it(listOfFilesForSeries);

      while (it.hasNext())
      {
        seriesToLoad.push_back(it.next().toStdString());
      }


      mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(seriesToLoad);
      if (node.IsNull())
      {
        MITK_ERROR << "Error loading series: " << ctkEvent.getProperty("SeriesName").toString().toStdString()
                   << " id: " <<ctkEvent.getProperty("SeriesUID").toString().toStdString();
      }
      else
      {
        //Get Reference for default data storage.
        ctkServiceReference serviceReference =mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
        mitk::IDataStorageService* storageService = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(serviceReference);
        mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

        dataStorage->Add(node);
        mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
      }
    }
  }
  else
  {
    MITK_INFO << "There are no files for the current series";
  }
}

void DicomEventHandler::OnSignalRemoveSeriesFromStorage(const ctkEvent& /*ctkEvent*/)
{
}

void DicomEventHandler::SubscribeSlots()
{
  ctkServiceReference ref = mitk::PluginActivator::getContext()->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = mitk::PluginActivator::getContext()->getService<ctkEventAdmin>(ref);
    ctkDictionary properties;
    properties[ctkEventConstants::EVENT_TOPIC] = "org/mitk/gui/qt/dicom/ADD";
    eventAdmin->subscribeSlot(this, SLOT(OnSignalAddSeriesToDataManager(ctkEvent)), properties);
    properties[ctkEventConstants::EVENT_TOPIC] = "org/mitk/gui/qt/dicom/DELETED";
    eventAdmin->subscribeSlot(this, SLOT(OnSignalRemoveSeriesFromStorage(ctkEvent)), properties);
  }
}
