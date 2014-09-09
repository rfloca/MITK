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

#include "mitkContourModelSet.h"
#include "mitkContourModelSetMapper3D.h"
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>

mitk::ContourModelSetMapper3D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New()),
    m_LastMTime(0)
{
}

mitk::ContourModelSetMapper3D::LocalStorage::~LocalStorage()
{
}

void mitk::ContourModelSetMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool overwrite)
{
  if (node != NULL)
  {
    node->AddProperty("color", ColorProperty::New(1, 0, 0), renderer, overwrite);
    node->AddProperty("contour.3D.width", FloatProperty::New(1), renderer, overwrite);

    Superclass::SetDefaultProperties(node, renderer, overwrite);
  }
}

mitk::ContourModelSetMapper3D::ContourModelSetMapper3D()
{
}

mitk::ContourModelSetMapper3D::~ContourModelSetMapper3D()
{
}

void mitk::ContourModelSetMapper3D::ApplyColorAndOpacityProperties(vtkActor* actor, BaseRenderer* renderer)
{
  DataNode* dataNode = this->GetDataNode();

  if (dataNode != NULL)
  {
    float rgb[3] = {0};
    dataNode->GetColor(rgb, renderer, "contour.color");

    float opacity = 1;
    dataNode->GetOpacity(opacity, renderer);

    vtkProperty* property = actor->GetProperty();
    property->SetColor(rgb[0], rgb[1], rgb[2]);
    property->SetOpacity(opacity);
  }
}

void mitk::ContourModelSetMapper3D::ApplyContourModelSetProperties(BaseRenderer* renderer)
{
  DataNode* dataNode = this->GetDataNode();

  if (dataNode != NULL)
  {
    float lineWidth = 1;
    dataNode->GetFloatProperty("contour.3D.width", lineWidth, renderer);

    vtkProperty* property = m_LocalStorageHandler.GetLocalStorage(renderer)->m_Actor->GetProperty();
    property->SetLineWidth(lineWidth);
  }
}

void mitk::ContourModelSetMapper3D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  LocalStorage* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  ContourModelSet* contourModelSet = dynamic_cast<ContourModelSet*>(GetDataNode()->GetData());

  if (contourModelSet != NULL)
  {
    unsigned long mTime = contourModelSet->GetMTime();

    if (mTime > localStorage->m_LastMTime)
    {
      localStorage->m_LastMTime = mTime;

      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
      vtkIdType baseIndex = 0;

      ContourModelSet::ContourModelSetIterator it = contourModelSet->Begin();
      ContourModelSet::ContourModelSetIterator end = contourModelSet->End();

      while (it != end)
      {
        ContourModel* contourModel = it->GetPointer();

        ContourModel::VertexIterator vertIt = contourModel->Begin();
        ContourModel::VertexIterator vertEnd = contourModel->End();

        while (vertIt != vertEnd)
        {
          points->InsertNextPoint((*vertIt)->Coordinates[0], (*vertIt)->Coordinates[1], (*vertIt)->Coordinates[2]);
          ++vertIt;
        }

        vtkSmartPointer<vtkPolyLine> line = vtkSmartPointer<vtkPolyLine>::New();
        vtkIdList* pointIds = line->GetPointIds();

        vtkIdType numPoints = contourModel->GetNumberOfVertices();
        pointIds->SetNumberOfIds(numPoints + 1);

        for (vtkIdType i = 0; i < numPoints; ++i)
          pointIds->SetId(i, baseIndex + i);

        pointIds->SetId(numPoints, baseIndex);

        cells->InsertNextCell(line);

        baseIndex += numPoints;

        ++it;
      }

      vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
      polyData->SetPoints(points);
      polyData->SetLines(cells);

      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputData(polyData);

      localStorage->m_Actor->SetMapper(mapper);
    }
  }

  this->ApplyColorAndOpacityProperties(localStorage->m_Actor, renderer);
  this->ApplyContourModelSetProperties(renderer);
}

vtkProp* mitk::ContourModelSetMapper3D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_Actor;
}
