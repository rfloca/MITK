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

#include <vtkImageData.h>

#include "mitkTestingMacros.h"

#include "vtkMitkThickSlicesFilter.h"


/**
* 3x3x3 test image
*/
vtkImageData *GenerateTestImageForTSFilter()
{
  // a 2x2x2 image
  short myData[] =
  {

    234,234,
    123,565,

    -213,800,
    1000,-20
  };

  vtkImageData *i = vtkImageData::New();

  i->SetExtent(0,1,0,1,0,1);

  i->AllocateScalars(VTK_SHORT,1);

  short *p = (short*)i->GetScalarPointer();

  memcpy(p,myData,2*2*2*sizeof(short));

  return i;
}

void CheckResultImageForTSFilter(vtkImageData *i)
{
  int *e=i->GetExtent();

  MITK_TEST_CONDITION_REQUIRED( e[0] == 0 && e[1] == 1 && e[2] == 0 && e[3] == 1 && e[4] == 0 && e[5] == 0 , "output image has correct extent" )
  MITK_TEST_CONDITION_REQUIRED( i->GetScalarType() == VTK_SHORT , "output image has correct scalar type" )

  short expectedResult[] =
  {

    234,800,
    1000,565
  };

  short *d = (short*)i->GetScalarPointer();

  MITK_TEST_CONDITION_REQUIRED( memcmp(d,expectedResult,sizeof(expectedResult)) == 0 , "output image has correct content" )
}


/**
* todo
*/
int vtkMitkThickSlicesFilterTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ThickSlicesFilter")

  vtkImageData *i,*o;

  i = GenerateTestImageForTSFilter();

  vtkMitkThickSlicesFilter *f = vtkMitkThickSlicesFilter::New();
  f->SetThickSliceMode( 0 ); // MIP
  f->SetInputData( i );
  f->Update();
  o = f->GetOutput();

  CheckResultImageForTSFilter(o);

  //Delete vtk variable correctly
  i->Delete();
  f->Delete();

  MITK_TEST_END()
}

