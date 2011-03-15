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
#ifndef __otbTimeSeriesLSFF_h
#define __otbTimeSeriesLSFF_h


namespace otb
{
namespace Functor
{
/** \class otbTimeSeriesLeastSquareFittingFunctor \brief: This functor
   *  implements a least squares fitting of a time profile. The time
   *  series as well as the date series are supposed to accept the []
   *  syntax to get the values and the Size() method to get their length.
 *
 */
template <class TSeriesType, class TTimeFunction, class TDateType = TSeriesType>
class TimeSeriesLeastSquareFittingFunctor
{
public:

  typedef typename TTimeFunction::CoefficientsType CoefficientsType;

  /// Constructor
  TimeSeriesLeastSquareFittingFunctor()
  {
  }
  /// Destructor
  virtual ~TimeSeriesLeastSquareFittingFunctor() {}

  inline TSeriesType operator ()(const TSeriesType& series)
  {
    TSeriesType outSeries;
    return outSeries;
  }

  inline void SetDates(const TDateType& doy)
  {
    for(unsigned int i=0; i<doy.Size(); ++i)
      m_DoySeries[i] = doy[i];
  }

  inline CoefficientsType GetCoefficients() const
  {
    return m_TimeFunction.GetCoefficients();
  }


private:
  /// 
  TTimeFunction m_TimeFunction;
  TDateType m_DoySeries;
};
}
}
#endif
