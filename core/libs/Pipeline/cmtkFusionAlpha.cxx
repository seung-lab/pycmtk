/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//  Copyright 2004-2009 SRI International
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
//  $Revision: 5806 $
//
//  $LastChangedDate: 2009-05-29 13:36:00 -0700 (Fri, 29 May 2009) $
//
//  $LastChangedBy: torsten $
//
*/

#include <cmtkFusionAlpha.h>
#include <cmtkRGB.h>

namespace
cmtk
{

/** \addtogroup Pipeline */
//@{

void FusionAlpha::Execute()
{
  if ( !Input[0] || !Input[1] ) return;
  if ( !Input[0]->GetNumPixels() || !Input[1]->GetNumPixels() ) return;

  ImageRGB *output = this->GetOutput();
  output->CopyStructure( Input[0] );
  if ( OutputHasAlpha )
    output->SetAlphaChannel( IMAGE_RGBA );
  else
    output->SetAlphaChannel( IMAGE_RGB );
  output->GetDataPtr( true /* forceAlloc */ );

  RGBA rgb1, rgb2, rgba;
  RGB outrgb;
  unsigned int outputPixels = output->GetNumPixels();

  if ( TransparencyImage && TransparencyImage->GetNumPixels() ) 
    {
    for ( unsigned int idx = 0; idx < outputPixels; ++idx ) 
      {
      Input[  TopImageIndex]->GetPixel( rgb1, idx );
      Input[1-TopImageIndex]->GetPixel( rgb2, idx );
      TransparencyImage->GetPixel( rgba, idx );
      
      float alpha = ((float) rgba.Alpha) / 255.0;
      outrgb.R = (byte) (alpha*rgb1.R + (1-alpha)*rgb2.R);
      outrgb.G = (byte) (alpha*rgb1.G + (1-alpha)*rgb2.G);
      outrgb.B = (byte) (alpha*rgb1.B + (1-alpha)*rgb2.B);
      output->SetPixel( idx, outrgb );
      }
    } 
  else
    {
    for ( unsigned int idx = 0; idx < outputPixels; ++idx ) 
      {
      Input[  TopImageIndex]->GetPixel( rgb1, idx );
      Input[1-TopImageIndex]->GetPixel( rgb2, idx );
      
      float alpha = ((float) rgb1.Alpha) / 255.0;
      outrgb.R = (byte) (alpha*rgb1.R + (1-alpha)*rgb2.R);
      outrgb.G = (byte) (alpha*rgb1.G + (1-alpha)*rgb2.G);
      outrgb.B = (byte) (alpha*rgb1.B + (1-alpha)*rgb2.B);
      output->SetPixel( idx, outrgb );
      }
    }
}

} // namespace cmtk
