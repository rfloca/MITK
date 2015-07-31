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


#include "mitkDoseVisPreferenceHelper.h"
#include "mitkRTUIConstants.h"
#include "mitkIsoLevelsGenerator.h"


#include <berryIPreferencesService.h>

#include <berryIPreferences.h>

#include <ctkPluginContext.h>
#include <service/event/ctkEventAdmin.h>

void mitk::StorePresetsMap(const PresetMapType& presetMap)
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer doseVisNode = prefService->GetSystemPreferences()->Node(
        QString::fromStdString(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID));
  berry::IPreferences::Pointer presetsNode = doseVisNode->Node(QString::fromStdString(
        mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID));

  presetsNode->RemoveNode();
  doseVisNode->Flush();

  //new empty preset node
  presetsNode = doseVisNode->Node(QString::fromStdString(
                                    mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID));

  //store map in new node
  for (PresetMapType::const_iterator pos = presetMap.begin(); pos != presetMap.end(); ++pos)
  {
    berry::IPreferences::Pointer aPresetNode = presetsNode->Node(QString::fromStdString(pos->first));

    unsigned int id = 0;

    for (mitk::IsoDoseLevelSet::ConstIterator levelPos = pos->second->Begin();
         levelPos != pos->second->End(); ++levelPos, ++id)
    {
      std::ostringstream stream;
      stream << id;

      berry::IPreferences::Pointer levelNode = aPresetNode->Node(QString::fromStdString(stream.str()));

      levelNode->PutDouble(QString::fromStdString(mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID),
                           levelPos->GetDoseValue());
      levelNode->PutFloat(QString::fromStdString(mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID),
                          levelPos->GetColor().GetRed());
      levelNode->PutFloat(QString::fromStdString(mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID),
                          levelPos->GetColor().GetGreen());
      levelNode->PutFloat(QString::fromStdString(mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID),
                          levelPos->GetColor().GetBlue());
      levelNode->PutBool(QString::fromStdString(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID),
                         levelPos->GetVisibleIsoLine());
      levelNode->PutBool(QString::fromStdString(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID),
                         levelPos->GetVisibleColorWash());
      levelNode->Flush();
    }

    aPresetNode->Flush();
  }

  presetsNode->Flush();
}

mitk::PresetMapType mitk::LoadPresetsMap()
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer presetsNode = prefService->GetSystemPreferences()->Node(
        QString::fromStdString(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID));

  QStringList names = presetsNode->ChildrenNames();

  PresetMapType presetMap;

  for (QStringList::const_iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    berry::IPreferences::Pointer aPresetNode = presetsNode->Node(*pos);

    if (aPresetNode.IsNull())
    {
      mitkThrow() << "Error in preference interface. Cannot find preset node under given name. Name: " <<
                  pos->toStdString();
    }

    mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

    QStringList levelNames = aPresetNode->ChildrenNames();

    for (QStringList::const_iterator levelName = levelNames.begin(); levelName != levelNames.end();
         ++levelName)
    {
      berry::IPreferences::Pointer levelNode = aPresetNode->Node(*levelName);

      if (aPresetNode.IsNull())
      {
        mitkThrow() <<
                    "Error in preference interface. Cannot find level node under given preset name. Name: " <<
                    pos->toStdString() <<
                    "; Level id: " << levelName->toStdString();
      }

      mitk::IsoDoseLevel::Pointer isoLevel = mitk::IsoDoseLevel::New();

      isoLevel->SetDoseValue(levelNode->GetDouble(QString::fromStdString(
                               mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID), 0.0));
      mitk::IsoDoseLevel::ColorType color;
      color.SetRed(levelNode->GetFloat(QString::fromStdString(
                                         mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID), 1.0));
      color.SetGreen(levelNode->GetFloat(QString::fromStdString(
                                           mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID), 1.0));
      color.SetBlue(levelNode->GetFloat(QString::fromStdString(
                                          mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID), 1.0));
      isoLevel->SetColor(color);
      isoLevel->SetVisibleIsoLine(levelNode->GetBool(QString::fromStdString(
                                    mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID), true));
      isoLevel->SetVisibleColorWash(levelNode->GetBool(QString::fromStdString(
                                      mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID), true));

      levelSet->SetIsoDoseLevel(isoLevel);
    }

    presetMap.insert(std::make_pair(pos->toStdString(), levelSet));
  }

  if (presetMap.size() == 0)
  {
    //if there are no presets use fallback and store it.
    presetMap = mitk::GenerateDefaultPresetsMap();
    StorePresetsMap(presetMap);
  }

  return presetMap;
}

mitk::PresetMapType mitk::GenerateDefaultPresetsMap()
{
  mitk::PresetMapType result;

  result.insert(std::make_pair(std::string("Virtuos"), mitk::GeneratIsoLevels_Virtuos()));
  return result;
}

std::string mitk::GetSelectedPresetName()
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(
      QString::fromStdString(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID));

  std::string result = prefNode->Get(QString::fromStdString(
                                       mitk::RTUIConstants::SELECTED_ISO_PRESET_ID), "").toStdString();

  return result;
}

void mitk::SetSelectedPresetName(const std::string& presetName)
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(
      QString::fromStdString(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID));
  berry::IPreferences::Pointer presetsNode = prefService->GetSystemPreferences()->Node(
        QString::fromStdString(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID));

  QStringList presetNames = presetsNode->ChildrenNames();

  if (presetNames.indexOf(QString::fromStdString(presetName)) < 0)
  {
    mitkThrow() <<
                "Error. Tried to set invalid selected preset name. Preset name does not exist in the defined presets. Preset name: "
                << presetName;
  }

  prefNode->Put(QString::fromStdString(mitk::RTUIConstants::SELECTED_ISO_PRESET_ID),
                QString::fromStdString(presetName));
}

bool mitk::GetReferenceDoseValue(mitk::DoseValueAbs& value)
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(
      QString::fromStdString(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID));

  bool result = prefNode->GetBool(QString::fromStdString(
                                    mitk::RTUIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID), true);
  value = prefNode->GetDouble(QString::fromStdString(mitk::RTUIConstants::REFERENCE_DOSE_ID),
                              mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE);

  return result;
}

void mitk::SetReferenceDoseValue(bool globalSync, mitk::DoseValueAbs value)
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(
      QString::fromStdString(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID));

  prefNode->PutBool(QString::fromStdString(mitk::RTUIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID),
                    globalSync);

  if (value >= 0)
  {
    prefNode->PutDouble(QString::fromStdString(mitk::RTUIConstants::REFERENCE_DOSE_ID), value);
  }
}


bool mitk::GetDoseDisplayAbsolute()
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(
      QString::fromStdString(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID));

  bool result = prefNode->GetBool(QString::fromStdString(
                                    mitk::RTUIConstants::DOSE_DISPLAY_ABSOLUTE_ID), false);

  return result;
}

void mitk::SetDoseDisplayAbsolute(bool isAbsolute)
{
  berry::IPreferencesService* prefService =
    berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(
      QString::fromStdString(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID));

  prefNode->PutBool(QString::fromStdString(mitk::RTUIConstants::DOSE_DISPLAY_ABSOLUTE_ID),
                    isAbsolute);
}

void mitk::SignalReferenceDoseChange(bool globalSync, mitk::DoseValueAbs value,
                                     ctkPluginContext* context)
{
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();

  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    ctkProperties props;
    props["value"] = value;
    props["globalSync"] = globalSync;
    ctkEvent presetMapChangedEvent(mitk::RTCTKEventConstants::TOPIC_REFERENCE_DOSE_CHANGED.c_str());
    eventAdmin->sendEvent(presetMapChangedEvent);
  }
}

void mitk::SignalPresetMapChange(ctkPluginContext* context)
{
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();

  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    ctkEvent presetMapChangedEvent(
      mitk::RTCTKEventConstants::TOPIC_ISO_DOSE_LEVEL_PRESETS_CHANGED.c_str());
    eventAdmin->sendEvent(presetMapChangedEvent);
  }
}
