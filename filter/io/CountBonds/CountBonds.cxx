/*
MIT License

Copyright (c) 2021 Michael Bussler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "CountBonds.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkImplicitFunction.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include "vtkIncrementalPointLocator.h"

#include <math.h>
#include "vtkSmartPointer.h"
#include <algorithm>

vtkStandardNewMacro(CountBonds);

//----------------------------------------------------------------------------
// Construct with user-specified implicit function; InsideOut turned off; value
// set to 0.0; and generate clip scalars turned off.
CountBonds::CountBonds()
{
}

//----------------------------------------------------------------------------
CountBonds::~CountBonds()
{
}

//----------------------------------------------------------------------------
//
// Clip through data generating surface.
//
int CountBonds::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro(<< "Counting Bonds per Atom");


  vtkPointData *outPD = output->GetPointData();
  vtkPoints *inPts=input->GetPoints();

  vtkIdType numPts = input->GetNumberOfPoints();

  // Initialize self; create output objects
  //
  if ( numPts < 1 || inPts == NULL )
    {
    vtkDebugMacro(<<"No data to count bonds for");
    return 1;
    }



  vtkSmartPointer<vtkFloatArray> numBoundsArray = vtkSmartPointer<vtkFloatArray>::New();
  numBoundsArray->SetName("numBonds");
  numBoundsArray->SetNumberOfComponents(1);
  numBoundsArray->SetNumberOfTuples(numPts);

  //
  // Traverse all Input points, calculate and store number of bonds
  //
  input->BuildLinks();

  for ( vtkIdType inPtId=0; inPtId < numPts; inPtId++)
  {
      // count number of bonds per point
      vtkSmartPointer<vtkIdList> refNeighIds = getConnectedPoints(input, inPtId);

      // store to data array
      numBoundsArray->SetValue(inPtId, refNeighIds->GetNumberOfIds());
  }


  //
  // Update output and release memory
  //

  output->ShallowCopy(input);

  vtkPointData *pd = output->GetPointData();
  pd->AddArray(numBoundsArray);

  output->Squeeze();

  return 1;

}


vtkSmartPointer<vtkIdList> CountBonds::getConnectedPoints( vtkPolyData * pd, vtkIdType ptId, vtkSmartPointer<vtkIdList> bondIds /*= NULL*/)
{
	vtkSmartPointer<vtkIdList> cellIds  = vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> neighPts = vtkSmartPointer<vtkIdList>::New();

	pd->GetPointCells( ptId, cellIds);

	for(vtkIdType i = 0; i < cellIds->GetNumberOfIds(); i++)
	{
		vtkIdType cellId = cellIds->GetId(i);

		vtkSmartPointer<vtkIdList> pointIdList = vtkSmartPointer<vtkIdList>::New();
		pd->GetCellPoints( cellId, pointIdList);

		vtkIdType ptIdN = pointIdList->GetId(0);
		if( ptIdN == ptId)
			ptIdN = pointIdList->GetId(1);

		neighPts->InsertNextId(ptIdN);

		if( bondIds )
			bondIds->InsertNextId(cellId);
	}

	return neighPts;
}