/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/*===========================================================================*/
/*===============================[ Includes ]================================*/
/*===========================================================================*/
#include "otbGeometriesSource.h"
#include "otbGeometriesSet.h"

/*===========================================================================*/
/*==============================[ other stuff ]==============================*/
/*===========================================================================*/

otb::GeometriesSource::GeometriesSource()
{
  Superclass::SetNumberOfRequiredOutputs(1);
  // The following line has for side effect to set of DataSource with the wrong
  // type as default value instead of a plain nil
  // smartpointer... => NO
  // Superclass::SetNthOutput(0, 0);

  // Moreover, the default, and correct, value will be set in AllocateOutputs if
  // nothing is set by then => nothing more to do.

}

/*virtual*/  otb::GeometriesSource::~GeometriesSource()
{
}

/*virtual*/
otb::GeometriesSource::OutputGeometriesType* otb::GeometriesSource::GetOutput(void )
{
  return static_cast<GeometriesSet*> (Superclass::GetOutput(0));
}

/*virtual*/
otb::GeometriesSource::OutputGeometriesType* otb::GeometriesSource::GetOutput(unsigned int idx)
{
  return static_cast<GeometriesSet*> (Superclass::GetOutput(idx));
}

/*virtual*/
void otb::GeometriesSource::SetOutput(OutputGeometriesType* output, unsigned int idx/* = 0 */)
{
  Superclass::SetNthOutput(idx, output);
}

/*virtual*/ void otb::GeometriesSource::AllocateOutputs()
{
  // The default behaviour is to prepare a in-memory OGR datasource in case
  // filters are piped.
  // In the filter is meant to produce a file, use SetOutput, or the New(string)
  // function to built the GeometriesSource and filters
  if (!GetOutput() || !GetOutput()->IsSet())
    {
    GeometriesSet::Pointer gs = GeometriesSet::New(); // in-memory DataSource
    assert(gs);
    this->SetOutput(gs);
    }
}

/*virtual*/ void otb::GeometriesSource::PrepareOutputs()
{
  AllocateOutputs();
  Superclass::PrepareOutputs();
}
