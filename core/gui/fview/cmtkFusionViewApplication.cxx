/*
//
//  Copyright 2010-2011, 2013 SRI International
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

#include "cmtkFusionViewApplication.h"

#include <System/cmtkCommandLine.h>
#include <System/cmtkConsole.h>
#include <System/cmtkDebugOutput.h>

#include <IO/cmtkVolumeIO.h>
#include <IO/cmtkXformListIO.h>

#include <Qt/cmtkQtIcons.h>

#include <QtGui/QActionGroup>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QScrollBar>

#include <vector>
#include <string>

cmtk::FusionViewApplication
::FusionViewApplication( int& argc, char* argv[] ) 
  : QApplication( argc, argv ),
    m_MainWindow( new QMainWindow ),
    m_XformModel( 2 ), // default to full nonrigid
    m_SliceAxis( -1 ),
    m_SliceIndex( -1 ),
    m_Interpolator( Interpolators::LINEAR ),
    m_ZoomFactor( 1.0 ),
    m_Transparency( 1.0 ),
    m_CursorDisplayed( true )
{
  CommandLine cl;
  cl.SetProgramInfo( CommandLine::PRG_TITLE, "Fusion viewer." );

  std::string imagePathFix = "";
  cl.AddOption( CommandLine::Key( "fixed" ), &imagePathFix, "Fixed image path. If this is not provided, the program attempts to automatically deduce the fixed image path of the first transformation in the given sequence." )
    ->SetProperties( cmtk::CommandLine::PROPS_IMAGE );

  std::string imagePathMov = "";
  cl.AddOption( CommandLine::Key( "moving" ), &imagePathMov, "Moving image path. If this is not provided, the program attempts to automatically deduce the fixed image path of the first transformation in the given sequence." )
    ->SetProperties( cmtk::CommandLine::PROPS_IMAGE );

  std::vector<std::string> xformList;
  cl.AddParameterVector( &xformList, "XformList", "List of concatenated transformations. Insert '--inverse' to use the inverse of the transformation listed next." )
    ->SetProperties( cmtk::CommandLine::PROPS_XFORM | cmtk::CommandLine::PROPS_OPTIONAL );  
  
  try
    {
    cl.Parse( argc, const_cast<const char**>( argv ) );
    }
  catch ( const CommandLine::Exception& ex )
    {
    throw(ex);
    }

  this->m_XformList = XformListIO::MakeFromStringList( xformList );

  try
    {
    this->m_XformListAllAffine = this->m_XformList.MakeAllAffine();
    }
  catch ( const AffineXform::MatrixType::SingularMatrixException& )
    {
    StdErr << "WARNING: encountered singular matrix when creating affine-only transformation. Will substitute (incorrect) identify transformation instead.\n";
    this->m_XformListAllAffine = XformList();
    }

  if ( imagePathFix.empty() )
    {
    imagePathFix = this->m_XformList.GetFixedImagePath();
    if ( !imagePathFix.empty() )
      {
      DebugOutput( 2 ) << "INFO: deduced fixed image path as " << imagePathFix << "\n";
      }
    else
      {
      StdErr << "ERROR: could not deduce fixed image path from transformation(s).\n";
      throw( ExitException( 1 ) );
      }
    }
  
  this->m_Fixed.m_Volume = VolumeIO::ReadOriented( imagePathFix );
  if ( ! this->m_Fixed.m_Volume )
    {
    StdErr << "Fixed image '" << imagePathFix << "' could not be read.\n";
    throw( ExitException( 1 ) );
    }
  this->m_Fixed.m_DataRange = this->m_Fixed.m_Volume->GetData()->GetRange();

  // per-dimension scale factors make sure non-square pixels are displayed square
  this->m_ScalePixels = this->m_Fixed.m_Volume->Deltas();
  this->m_ScalePixels *= 1.0 / this->m_ScalePixels.MinValue();

  (this->m_CursorPosition = this->m_Fixed.m_Volume->GetDims() ) *= 0.5;

  if ( imagePathMov.empty() )
    {
    imagePathMov = this->m_XformList.GetMovingImagePath();
    if ( !imagePathMov.empty() )
      {
      DebugOutput( 2 ) << "INFO: deduced moving image path as " << imagePathMov << "\n";
      }
    else
      {
      StdErr << "ERROR: could not deduce moving image path from transformation(s).\n";
      throw( ExitException( 1 ) );
      }
    }
  
  this->m_Moving.m_Volume = VolumeIO::ReadOriented( imagePathMov );
  if ( ! this->m_Moving.m_Volume )
    {
    StdErr << "Moving image '" << imagePathMov << "' could not be read.\n";
    throw( ExitException( 1 ) );
    }
  this->m_Moving.m_DataRange = this->m_Moving.m_Volume->GetData()->GetRange();

  this->m_MainWindowUI.setupUi( this->m_MainWindow );
  this->m_MainWindow->setWindowIcon( QtIcons::WindowIcon() );

  this->InitViewData( this->m_Fixed, this->m_MainWindowUI.fixedView );
  this->InitViewData( this->m_Moving, this->m_MainWindowUI.movingView );

  QObject::connect( this->m_MainWindowUI.blackSliderFix, SIGNAL( valueChanged( int ) ), this, SLOT( fixedBlackWhiteChanged() ) );
  QObject::connect( this->m_MainWindowUI.whiteSliderFix, SIGNAL( valueChanged( int ) ), this, SLOT( fixedBlackWhiteChanged() ) );
  
  QObject::connect( this->m_MainWindowUI.blackSliderMov, SIGNAL( valueChanged( int ) ), this, SLOT( movingBlackWhiteChanged() ) );
  QObject::connect( this->m_MainWindowUI.whiteSliderMov, SIGNAL( valueChanged( int ) ), this, SLOT( movingBlackWhiteChanged() ) );
  
  QActionGroup* zoomGroup = new QActionGroup( this->m_MainWindow );
  zoomGroup->setExclusive( true );
  this->m_MainWindowUI.actionZoom25->setData( QVariant( 0.25 ) );
  zoomGroup->addAction( this->m_MainWindowUI.actionZoom25 );
  this->m_MainWindowUI.actionZoom50->setData( QVariant( 0.50 ) );
  zoomGroup->addAction( this->m_MainWindowUI.actionZoom50 );
  this->m_MainWindowUI.actionZoom100->setData( QVariant( 1.00 ) );
  zoomGroup->addAction( this->m_MainWindowUI.actionZoom100 );
  this->m_MainWindowUI.actionZoom200->setData( QVariant( 2.00 ) );
  zoomGroup->addAction( this->m_MainWindowUI.actionZoom200 );
  this->m_MainWindowUI.actionZoom300->setData( QVariant( 3.00 ) );
  zoomGroup->addAction( this->m_MainWindowUI.actionZoom300 );
  this->m_MainWindowUI.actionZoom400->setData( QVariant( 4.00 ) );
  zoomGroup->addAction( this->m_MainWindowUI.actionZoom400 );

  QObject::connect( zoomGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( changeZoom( QAction* ) ) );

  QActionGroup* fixedColorGroup = new QActionGroup( this->m_MainWindow );
  fixedColorGroup->setExclusive( true );
  this->m_MainWindowUI.actionFixedGrey->setData( QVariant( 0 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedGrey );
  this->m_MainWindowUI.actionFixedRed->setData( QVariant( 1 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedRed );
  this->m_MainWindowUI.actionFixedGreen->setData( QVariant( 2 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedGreen );
  this->m_MainWindowUI.actionFixedBlue->setData( QVariant( 3 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedBlue );
  this->m_MainWindowUI.actionFixedCyan->setData( QVariant( 4 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedCyan );
  this->m_MainWindowUI.actionFixedYellow->setData( QVariant( 5 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedYellow );
  this->m_MainWindowUI.actionFixedMagenta->setData( QVariant( 6 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedMagenta );
  this->m_MainWindowUI.actionFixedBlueRed->setData( QVariant( 7 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedBlueRed );
  this->m_MainWindowUI.actionFixedRedBlue->setData( QVariant( 8 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedRedBlue );
  this->m_MainWindowUI.actionFixedLabels->setData( QVariant( 9 ) );
  fixedColorGroup->addAction( this->m_MainWindowUI.actionFixedLabels );

  QObject::connect( fixedColorGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( changeFixedColor( QAction* ) ) );

  QActionGroup* movingColorGroup = new QActionGroup( this->m_MainWindow );
  movingColorGroup->setExclusive( true );
  this->m_MainWindowUI.actionMovingGrey->setData( QVariant( 0 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingGrey );
  this->m_MainWindowUI.actionMovingRed->setData( QVariant( 1 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingRed );
  this->m_MainWindowUI.actionMovingGreen->setData( QVariant( 2 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingGreen );
  this->m_MainWindowUI.actionMovingBlue->setData( QVariant( 3 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingBlue );
  this->m_MainWindowUI.actionMovingCyan->setData( QVariant( 4 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingCyan );
  this->m_MainWindowUI.actionMovingYellow->setData( QVariant( 5 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingYellow );
  this->m_MainWindowUI.actionMovingMagenta->setData( QVariant( 6 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingMagenta );
  this->m_MainWindowUI.actionMovingBlueRed->setData( QVariant( 7 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingBlueRed );
  this->m_MainWindowUI.actionMovingRedBlue->setData( QVariant( 8 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingRedBlue );
  this->m_MainWindowUI.actionMovingLabels->setData( QVariant( 9 ) );
  movingColorGroup->addAction( this->m_MainWindowUI.actionMovingLabels );

  QObject::connect( movingColorGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( changeMovingColor( QAction* ) ) );

  QActionGroup* sliceGroup = new QActionGroup( this->m_MainWindow );
  sliceGroup->setExclusive( true );
  sliceGroup->addAction( this->m_MainWindowUI.actionSliceAxial_XY );
  this->m_MainWindowUI.actionSliceAxial_XY->setData( QVariant( AXIS_Z ) );
  sliceGroup->addAction( this->m_MainWindowUI.actionSliceCoronal_XZ );
  this->m_MainWindowUI.actionSliceCoronal_XZ->setData( QVariant( AXIS_Y ) );
  sliceGroup->addAction( this->m_MainWindowUI.actionSliceSagittal_YZ );
  this->m_MainWindowUI.actionSliceSagittal_YZ->setData( QVariant( AXIS_X ) );
  
  QObject::connect( sliceGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( changeSliceDirection( QAction* ) ) );
  
  QActionGroup* interpGroup = new QActionGroup( this->m_MainWindow );
  interpGroup->setExclusive( true );
  this->m_MainWindowUI.actionInterpLinear->setData( QVariant( Interpolators::LINEAR ) );
  interpGroup->addAction( this->m_MainWindowUI.actionInterpLinear );
  this->m_MainWindowUI.actionInterpCubic->setData( QVariant( Interpolators::CUBIC ) );
  interpGroup->addAction( this->m_MainWindowUI.actionInterpCubic );
  this->m_MainWindowUI.actionInterpSinc->setData( QVariant( Interpolators::COSINE_SINC ) );
  interpGroup->addAction( this->m_MainWindowUI.actionInterpSinc );
  this->m_MainWindowUI.actionInterpNearestNeighbour->setData( QVariant( Interpolators::NEAREST_NEIGHBOR ) );
  interpGroup->addAction( this->m_MainWindowUI.actionInterpNearestNeighbour );
  this->m_MainWindowUI.actionInterpPartialVolume->setData( QVariant( Interpolators::PARTIALVOLUME ) );
  interpGroup->addAction( this->m_MainWindowUI.actionInterpPartialVolume );

  QObject::connect( interpGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( changeInterpolator( QAction* ) ) );
  
  QActionGroup* xformGroup = new QActionGroup( this->m_MainWindow );
  xformGroup->setExclusive( true );
  this->m_MainWindowUI.actionXformIdentity->setData( QVariant( 0 ) );
  xformGroup->addAction( this->m_MainWindowUI.actionXformIdentity );
  this->m_MainWindowUI.actionXformAffine->setData( QVariant( 1 ) );
  xformGroup->addAction( this->m_MainWindowUI.actionXformAffine );
  this->m_MainWindowUI.actionXformWarp->setData( QVariant( 2 ) );
  xformGroup->addAction( this->m_MainWindowUI.actionXformWarp );

  QObject::connect( xformGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( changeXform( QAction* ) ) );
  
  this->m_MainWindowUI.alphaSlider->setRange( 0, 1000 );
  this->m_MainWindowUI.alphaSlider->setValue( this->m_Transparency * 1000 );
  QObject::connect( this->m_MainWindowUI.alphaSlider, SIGNAL( valueChanged( int ) ), this, SLOT( setTransparency( int ) ) );
  
  this->changeSliceDirection( AXIS_Z );
  QObject::connect( this->m_MainWindowUI.sliceSlider, SIGNAL( valueChanged( int ) ), this, SLOT( setFixedSlice( int ) ) );

  QObject::connect( this->m_MainWindowUI.actionLinkedCursor, SIGNAL( toggled( bool ) ), this, SLOT( setLinkedCursorFlag( bool ) ) );  

  // synchronize sliders of the two graphics views
  QObject::connect( this->m_MainWindowUI.fixedView->horizontalScrollBar(), SIGNAL( valueChanged( int ) ), 
		    this->m_MainWindowUI.movingView->horizontalScrollBar(), SLOT( setValue( int ) ) );
  QObject::connect( this->m_MainWindowUI.fixedView->verticalScrollBar(), SIGNAL( valueChanged( int ) ), 
		    this->m_MainWindowUI.movingView->verticalScrollBar(), SLOT( setValue( int) ) );

  QObject::connect( this->m_MainWindowUI.movingView->horizontalScrollBar(), SIGNAL( valueChanged( int ) ), 
		    this->m_MainWindowUI.fixedView->horizontalScrollBar(), SLOT( setValue( int ) ) );
  QObject::connect( this->m_MainWindowUI.movingView->verticalScrollBar(), SIGNAL( valueChanged( int ) ), 
		    this->m_MainWindowUI.fixedView->verticalScrollBar(), SLOT( setValue( int ) ) );

  this->m_MainWindow->show();
}

void
cmtk::FusionViewApplication
::InitViewData( Self::Data& data, QGraphicsView* view )
{
  data.m_ColorMapIndex = 0;

  const QPen redPen( QColor( 255, 0, 0 ) );

  data.m_Scene = new QGraphicsScene;
  (data.m_CursorLines[0] = data.m_Scene->addLine( 0, 0, 0, 0, redPen ))->setZValue( 100 );
  (data.m_CursorLines[1] = data.m_Scene->addLine( 0, 0, 0, 0, redPen ))->setZValue( 100 );
  (data.m_PixmapItem = new QGraphicsPixmapItemEvents)->setZValue( 0 );
  data.m_Scene->addItem( data.m_PixmapItem );

  QObject::connect( data.m_PixmapItem, SIGNAL( mousePressed( QGraphicsSceneMouseEvent* ) ), this, SLOT( mousePressed( QGraphicsSceneMouseEvent* ) ) );

  data.m_View = view;
  data.m_View->setScene( data.m_Scene );
}


void
cmtk::FusionViewApplication
::setTransparency( int slice )
{
  this->m_Transparency = static_cast<float>( slice ) / 1000;
  this->UpdateMovingImage();
}

void
cmtk::FusionViewApplication
::changeFixedColor( QAction* action )
{
  this->m_Fixed.m_ColorMapIndex = action->data().toInt();
  this->UpdateFixedImage();
}

void
cmtk::FusionViewApplication
::changeMovingColor( QAction* action )
{
  this->m_Moving.m_ColorMapIndex = action->data().toInt();
  this->UpdateMovingImage();
}

void
cmtk::FusionViewApplication
::setLinkedCursorFlag( bool flag )
{
  this->m_CursorDisplayed = flag;

  for ( int i = 0; i < 2; ++i )
    {
    this->m_Fixed.m_CursorLines[i]->setVisible( this->m_CursorDisplayed );
    this->m_Moving.m_CursorLines[i]->setVisible( this->m_CursorDisplayed );
    }
}

void
cmtk::FusionViewApplication
::setFixedSlice( int slice )
{
  if ( this->m_SliceIndex != slice )
    {
    this->m_SliceIndex = slice;
    this->m_CursorPosition[this->m_SliceAxis] = this->m_SliceIndex;

    this->m_Fixed.m_Slice = this->m_Fixed.m_Volume->ExtractSlice( this->m_SliceAxis, this->m_SliceIndex );
    this->UpdateFixedImage();

    this->UpdateMovingSlice();

    this->m_MainWindowUI.sliceLabel->setText( QString("Slice: %1").arg( this->m_SliceIndex ) );
    }
}

void
cmtk::FusionViewApplication
::changeZoom( QAction* action )
{
  this->m_ZoomFactor = static_cast<float>( action->data().toDouble() ); // older Qt doesn't have QVariant::toFloat()
  this->UpdateFixedImage();
  this->UpdateMovingImage();
}

void
cmtk::FusionViewApplication
::changeInterpolator( QAction* action )
{
  this->m_Interpolator = static_cast<Interpolators::InterpolationEnum>( action->data().toInt() );
  this->UpdateMovingSlice();
}

void
cmtk::FusionViewApplication
::changeXform( QAction* action )
{
  this->m_XformModel = action->data().toInt();
  this->UpdateMovingSlice();
}

void
cmtk::FusionViewApplication
::fixedBlackWhiteChanged()
{
  this->UpdateFixedImage();
}

void
cmtk::FusionViewApplication
::movingBlackWhiteChanged()
{
  this->UpdateMovingImage();
}

void
cmtk::FusionViewApplication
::changeSliceDirection( QAction* action )
{
  this->changeSliceDirection( action->data().toInt() );
}

void
cmtk::FusionViewApplication
::mousePressed( QGraphicsSceneMouseEvent* event )  
{
  const int idxX = this->GetAxis2DX();
  const int idxY = this->GetAxis2DY();

  this->m_CursorPosition[idxX] = event->pos().x() / this->m_ScalePixels[idxX];
  this->m_CursorPosition[idxY] = event->pos().y() / this->m_ScalePixels[idxY];

  if ( this->m_CursorDisplayed )
    {
    this->UpdateView( this->m_Fixed, this->m_Fixed.m_Image );
    this->UpdateView( this->m_Moving, this->m_FusedImage );
    }
}

void
cmtk::FusionViewApplication
::changeSliceDirection( const int sliceAxis )
{
  if ( sliceAxis != this->m_SliceAxis )
    {
    this->m_SliceAxis = sliceAxis;

    this->m_SliceIndex = -1; // unset previously set slice index to ensure update of moving slice
    const int newSliceIndex = static_cast<int>( this->m_CursorPosition[this->m_SliceAxis] ); // store to safeguard against unwanted updates
    this->m_MainWindowUI.sliceSlider->setRange( 0, this->m_Fixed.m_Volume->GetDims()[this->m_SliceAxis]-1 );

    this->setFixedSlice( newSliceIndex );
    this->m_MainWindowUI.sliceSlider->setValue( this->m_SliceIndex );
    
    this->UpdateMovingSlice();

    const char* labelFrom[3] = { "Left", "Posterior", "Inferior" };
    const char* labelTo[3] = { "Right", "Anterior", "Superior" };

    this->m_MainWindowUI.sliceLabelFrom->setText( labelFrom[this->m_SliceAxis] );
    this->m_MainWindowUI.sliceLabelTo->setText( labelTo[this->m_SliceAxis] );
    }
}

void
cmtk::FusionViewApplication
::UpdateMovingSlice()
{
  ReformatVolume::Plain plain( TYPE_FLOAT );
  UniformVolumeInterpolatorBase::SmartPtr interpolator ( ReformatVolume::CreateInterpolator( this->m_Interpolator, this->m_Moving.m_Volume ) );
  
  const XformList noXforms;
  TypedArray::SmartPtr reformatData;
  switch ( this->m_XformModel )
    {
    case 0:
      reformatData = ReformatVolume::ReformatUnmasked( this->m_Fixed.m_Slice, noXforms, noXforms, plain, this->m_Moving.m_Volume, interpolator );
      break;
    case 1:
      reformatData = ReformatVolume::ReformatUnmasked( this->m_Fixed.m_Slice, this->m_XformListAllAffine, noXforms, plain, this->m_Moving.m_Volume, interpolator );
      break;
    case 2:
    default:
      reformatData = ReformatVolume::ReformatUnmasked( this->m_Fixed.m_Slice, this->m_XformList, noXforms, plain, this->m_Moving.m_Volume, interpolator );
      break;
    }
  
  UniformVolume::SmartPtr movingSlice = this->m_Fixed.m_Slice->CloneGrid();
  movingSlice->SetData( reformatData );
  
  this->m_Moving.m_Slice = movingSlice;
  this->UpdateMovingImage();
}

void
cmtk::FusionViewApplication
::MakeColorTable( Self::Data& data )
{
  data.m_ColorTable.resize( 256 );

  switch ( data.m_ColorMapIndex )
    {
    default:
    case 0: // black/white
      for ( int i = 0; i < 256; ++i )
	{
	data.m_ColorTable[i] = QColor( i, i, i ).rgb();
	}
      break;
    case 1: // red
      for ( int i = 0; i < 256; ++i )
	{
	data.m_ColorTable[i] = QColor( i, 0, 0 ).rgb();
	}
      break;
    case 2: // green
      for ( int i = 0; i < 256; ++i )
	{
	data.m_ColorTable[i] = QColor( 0, i, 0 ).rgb();
	}
      break;
    case 3: // blue
      for ( int i = 0; i < 256; ++i )
	{
	data.m_ColorTable[i] = QColor( 0, 0, i ).rgb();
	}
      break;
    case 4: // cyan
      for ( int i = 0; i < 256; ++i )
	{
	data.m_ColorTable[i] = QColor( 0, i, i ).rgb();
	}
      break;
    case 5: // yellow
      for ( int i = 0; i < 256; ++i )
	{
	data.m_ColorTable[i] = QColor( i, i, 0 ).rgb();
	}
      break;
    case 6: // magenta
      for ( int i = 0; i < 256; ++i )
	{
	data.m_ColorTable[i] = QColor( i, 0, i ).rgb();
	}
      break;
    case 7: // blue to red
      for ( int i = 0; i < 256; ++i )
	{
	QColor color;
	color.setHsv( 255-i, 255, 255 );
	data.m_ColorTable[i] = color.rgb();
	}
      break;
    case 8: // red to blue
      for ( int i = 0; i < 256; ++i )
	{
	QColor color;
	color.setHsv( i, 255, 255 );
	data.m_ColorTable[i] = color.rgb();
	}
      break;
    }
}

void
cmtk::FusionViewApplication
::UpdateFixedImage()
{
  this->MakeColorTable( this->m_Fixed );

  const float black = this->m_Fixed.m_DataRange.m_LowerBound + this->m_Fixed.m_DataRange.Width() * this->m_MainWindowUI.blackSliderFix->value() / 500;
  const float white = this->m_Fixed.m_DataRange.m_LowerBound + this->m_Fixed.m_DataRange.Width() * this->m_MainWindowUI.whiteSliderFix->value() / 500;

  this->MakeImage( this->m_Fixed.m_Image, *(this->m_Fixed.m_Slice), this->m_Fixed.m_ColorTable, black, white );
  this->UpdateView( this->m_Fixed, this->m_Fixed.m_Image );
}

void
cmtk::FusionViewApplication
::UpdateMovingImage()
{
  this->MakeColorTable( this->m_Moving );

  const float black = this->m_Moving.m_DataRange.m_LowerBound + this->m_Moving.m_DataRange.Width() * this->m_MainWindowUI.blackSliderMov->value() / 500;
  const float white = this->m_Moving.m_DataRange.m_LowerBound + this->m_Moving.m_DataRange.Width() * this->m_MainWindowUI.whiteSliderMov->value() / 500;

  this->MakeImage( this->m_Moving.m_Image, *(this->m_Moving.m_Slice), this->m_Moving.m_ColorTable, black, white );

  this->m_FusedImage = QImage( this->m_Moving.m_Image.width(), this->m_Moving.m_Image.height(), QImage::Format_RGB32 );
  for ( int y = 0; y < this->m_Moving.m_Image.height(); ++y )
    {
    for ( int x = 0; x < this->m_Moving.m_Image.width(); ++x )
      {
      QColor rgbMov( this->m_Moving.m_Image.pixel( x, y ) );
      const QColor rgbFix( this->m_Fixed.m_Image.pixel( x, y ) );

      rgbMov = QColor( this->m_Transparency * rgbMov.red() + (1.0-this->m_Transparency) * rgbFix.red(),
		       this->m_Transparency * rgbMov.green() + (1.0-this->m_Transparency) * rgbFix.green(),
		       this->m_Transparency * rgbMov.blue() + (1.0-this->m_Transparency) * rgbFix.blue() );
      this->m_FusedImage.setPixel( x, y, rgbMov.rgb() );
      }
    }

  this->UpdateView( this->m_Moving, this->m_FusedImage );
}

void
cmtk::FusionViewApplication
::MakeImage( QImage& image, const UniformVolume& slice, const QVector<QRgb>& colorTable, const float blackLevel, const float whiteLevel )
{
  const int idxX = this->GetAxis2DX();
  const int idxY = this->GetAxis2DY();
  
  int dimX = slice.GetDims()[idxX];
  int dimY = slice.GetDims()[idxY];

  image = QImage( dimX, dimY, QImage::Format_Indexed8 );
  image.setColorTable( colorTable );
  
  const float scaleLevel = 1.0 / (whiteLevel-blackLevel);
  
  size_t idx = 0;
  for ( int y = 0; y < dimY; ++y )
    {
    for ( int x = 0; x < dimX; ++x, ++idx )
      {
      image.setPixel( x, y, static_cast<int>( 255 * std::min<float>( 1, std::max<float>( 0, (slice.GetDataAt( idx ) - blackLevel) * scaleLevel ) ) ) );
      }
    }
}

void
cmtk::FusionViewApplication
::UpdateView( Self::Data& data, QImage& image )
{
  data.m_PixmapItem->setPixmap( QPixmap::fromImage( image ) );

  const QRectF bb = data.m_PixmapItem->boundingRect();
  data.m_Scene->setSceneRect( bb );

  const int idxX = this->GetAxis2DX();
  const int idxY = this->GetAxis2DY();
  
  if ( this->m_CursorDisplayed )
    {    
    data.m_CursorLines[0]->setLine( this->m_CursorPosition[idxX], bb.top(), this->m_CursorPosition[idxX], bb.bottom() );
    data.m_CursorLines[1]->setLine( bb.left(), this->m_CursorPosition[idxY], bb.right(), this->m_CursorPosition[idxY] );
    }

  QTransform zoomTransform = QTransform::fromScale( -this->m_ZoomFactor * this->m_ScalePixels[idxX], -this->m_ZoomFactor * this->m_ScalePixels[idxY] );
  data.m_View->setTransform( zoomTransform );
}
