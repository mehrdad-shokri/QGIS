/***************************************************************************
    qgslabellineanchorwidget.cpp
    ----------------------
    begin                : August 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslabellineanchorwidget.h"
#include "qgsexpressioncontextutils.h"
#include "qgsapplication.h"

QgsLabelLineAnchorWidget::QgsLabelLineAnchorWidget( QWidget *parent, QgsVectorLayer *vl )
  : QgsLabelSettingsWidgetBase( parent, vl )
{
  setupUi( this );

  setPanelTitle( tr( "Line Anchor Settings" ) );

  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorCenter.svg" ) ), tr( "Center of Line" ), 0.5 );
  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorStart.svg" ) ), tr( "Start of Line" ), 0.0 );
  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorEnd.svg" ) ), tr( "End of Line" ), 1.0 );
  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorCustom.svg" ) ), tr( "Custom…" ), -1.0 );

  mAnchorTypeComboBox->addItem( tr( "Preferred Placement Hint" ), static_cast< int >( QgsLabelLineSettings::AnchorType::HintOnly ) );
  mAnchorTypeComboBox->addItem( tr( "Strict" ), static_cast< int >( QgsLabelLineSettings::AnchorType::Strict ) );

  connect( mPercentPlacementComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    if ( !mBlockSignals )
      emit changed();

    if ( mPercentPlacementComboBox->currentData().toDouble() < 0 )
      mCustomPlacementSpinBox->setEnabled( true );
    else
    {
      mCustomPlacementSpinBox->setEnabled( false );
      mBlockSignals = true;
      mCustomPlacementSpinBox->setValue( mPercentPlacementComboBox->currentData().toDouble() * 100 );
      mBlockSignals = false;
    }
  } );
  connect( mCustomPlacementSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, [ = ]( double )
  {
    if ( !mBlockSignals )
      emit changed();
  } );

  connect( mAnchorTypeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    if ( !mBlockSignals )
      emit changed();

    QString hint;
    switch ( static_cast< QgsLabelLineSettings::AnchorType >( mAnchorTypeComboBox->currentData().toInt() ) )
    {
      case QgsLabelLineSettings::AnchorType::Strict:
        hint = tr( "Labels are placed exactly on the label anchor only, and no other fallback placements are permitted." );
        break;

      case QgsLabelLineSettings::AnchorType::HintOnly:
        hint = tr( "The label anchor is treated as a hint for the preferred label placement, but other placements close to the anchor point are permitted." );
        break;
    }
    mAnchorTypeHintLabel->setText( hint );
  } );

  registerDataDefinedButton( mLinePlacementDDBtn, QgsPalLayerSettings::LineAnchorPercent );
}

void QgsLabelLineAnchorWidget::setSettings( const QgsLabelLineSettings &settings )
{
  mBlockSignals = true;
  const int comboIndex = mPercentPlacementComboBox->findData( settings.lineAnchorPercent() );
  if ( comboIndex >= 0 )
  {
    mPercentPlacementComboBox->setCurrentIndex( comboIndex );
  }
  else
  {
    // set custom control
    mPercentPlacementComboBox->setCurrentIndex( mPercentPlacementComboBox->findData( -1.0 ) );
    mCustomPlacementSpinBox->setValue( settings.lineAnchorPercent() * 100.0 );
  }
  mCustomPlacementSpinBox->setEnabled( mPercentPlacementComboBox->currentData().toDouble() < 0 );

  mAnchorTypeComboBox->setCurrentIndex( mAnchorTypeComboBox->findData( static_cast< int >( settings.anchorType() ) ) );
  mBlockSignals = false;
}

QgsLabelLineSettings QgsLabelLineAnchorWidget::settings() const
{
  QgsLabelLineSettings settings;

  if ( mPercentPlacementComboBox->currentData().toDouble() >= 0 )
  {
    settings.setLineAnchorPercent( mPercentPlacementComboBox->currentData().toDouble() );
  }
  else
  {
    settings.setLineAnchorPercent( mCustomPlacementSpinBox->value() / 100.0 );
  }

  settings.setAnchorType( static_cast< QgsLabelLineSettings::AnchorType >( mAnchorTypeComboBox->currentData().toInt() ) );
  return settings;
}

void QgsLabelLineAnchorWidget::updateDataDefinedProperties( QgsPropertyCollection &properties )
{
  properties.setProperty( QgsPalLayerSettings::LineAnchorPercent, mDataDefinedProperties.property( QgsPalLayerSettings::LineAnchorPercent ) );
}
