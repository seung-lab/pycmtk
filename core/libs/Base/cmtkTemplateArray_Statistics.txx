/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010 SRI International
//
//  This file is part of the Computational Morphometry Toolkit.
//
//  http://www.nitrc.org/projects/cmtk/
//
//  The Computational Morphometry Toolkit is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  The Computational Morphometry Toolkit is distributed in the hope that it
//  will be useful, but WITHOUT ANY WARRANTY; without even the implied
//  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with the Computational Morphometry Toolkit.  If not, see
//  <http://www.gnu.org/licenses/>.
//
//  $Revision$
//
//  $LastChangedDate$
//
//  $LastChangedBy$
//
*/

namespace
cmtk
{

/** \addtogroup Base */
//@{

template<class T> size_t
TemplateArray<T>::GetStatistics
( Types::DataItem& mean, Types::DataItem& variance ) const 
{
  size_t Count = 0;
  Types::DataItem Sum = 0, SumOfSquares = 0;
  for ( size_t i = 0; i < DataSize; ++i ) 
    {
    if ( !this->PaddingFlag || (this->Data[i] != this->Padding) ) 
      {
      ++Count;
      Sum += this->Data[i];
      SumOfSquares += MathUtil::Square<Types::DataItem>( this->Data[i] );
      }
    }
  
  if ( Count ) 
    {
    mean = Sum / Count;
    variance = (SumOfSquares - 2*mean*Sum)/Count + MathUtil::Square(mean);
    } 
  else
    {
    variance = mean = 0;
    }
  
  return Count;
}

template<class T>
Histogram<unsigned int>::SmartPtr
TemplateArray<T>::GetHistogram( const unsigned int numberOfBins, const bool centeredBins ) const
{
  Histogram<unsigned int>::SmartPtr histogram( new Histogram<unsigned int>( numberOfBins ) );

  if ( centeredBins )
    histogram->SetRangeCentered( Types::DataItemRange( this->GetRangeTemplate() ) );
  else
    histogram->SetRange( Types::DataItemRange( this->GetRangeTemplate() ) );
  
  for ( size_t idx = 0; idx < DataSize; ++idx )
    if ( !this->PaddingFlag || (this->Data[idx] != this->Padding) )
      histogram->Increment( histogram->ValueToBin( this->Data[idx] ) );
  
  return histogram;
}

} // namespace cmtk
