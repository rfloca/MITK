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

#ifndef mitkContourModelSetMapper3D_h
#define mitkContourModelSetMapper3D_h

#include <mitkVtkMapper.h>
#include <vtkSmartPointer.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  class MitkContourModel_EXPORT ContourModelSetMapper3D : public VtkMapper
  {
    class LocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      vtkSmartPointer<vtkActor> m_Actor;
      unsigned long m_LastMTime;

    private:
      LocalStorage(const LocalStorage&);
      LocalStorage& operator=(const LocalStorage&);
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    mitkClassMacro(ContourModelSetMapper3D, VtkMapper);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    using VtkMapper::ApplyColorAndOpacityProperties;
    void ApplyColorAndOpacityProperties(vtkActor*, BaseRenderer*);
    void ApplyContourModelSetProperties(BaseRenderer* renderer);
    vtkProp* GetVtkProp(BaseRenderer* renderer);

  protected:
    void GenerateDataForRenderer(BaseRenderer* renderer);

  private:
    ContourModelSetMapper3D();
    ~ContourModelSetMapper3D();

    ContourModelSetMapper3D(const ContourModelSetMapper3D&);
    ContourModelSetMapper3D& operator=(const ContourModelSetMapper3D&);

    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
