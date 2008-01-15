
#include "ChartConfigWidget.h"

// Qt
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMenu>

// KDE
#include <KLocale>
#include <KIconLoader>
#include <KDebug>

// KDChart
#include <KDChartChart>
#include <KDChartPosition>
#include <KDChartCartesianAxis>
#include <KDChartGridAttributes>
#include <KDChartAbstractCartesianDiagram>
#include <KDChartLegend>

// KChart
#include "kchart_global.h"
#include "ChartShape.h"
#include "ChartProxyModel.h"
#include "ui_ChartTableEditor.h"
#include "ui_ChartConfigWidget.h"
#include "ChartTableView.h"
#include "commands/ChartTypeCommand.h"

using namespace KChart;


// FIXME: What is this button for?
//
class Button : public QToolButton
{
public:
    Button(QWidget* parent, const QString& text, const KIcon& icon)
        : QToolButton(parent)
    {
        setCheckable(true);
        setIcon(icon);
        //setIconSize(QSize(48, 48));
        setToolTip(text);
        setText(text);
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
};


// ================================================================


class ChartConfigWidget::Private
{
public:
    Private();
    ~Private();

    // Basic properties of the chart.
    ChartShape            *shape;
    OdfChartType           type;
    OdfChartSubtype        subtype;
    bool                   threeDMode;
    QVBoxLayout           *leftLayout;
    QVBoxLayout           *rightLayout;
    Ui::ChartConfigWidget  ui;
    
    // chart type selection actions
    QAction  *normalBarChartAction;
    QAction  *stackedBarChartAction;
    QAction  *percentBarChartAction;
    
    QAction  *normalLineChartAction;
    QAction  *stackedLineChartAction;
    QAction  *percentLineChartAction;

    QAction  *normalAreaChartAction;
    QAction  *stackedAreaChartAction;
    QAction  *percentAreaChartAction;

    // Table Editor
    Ui::ChartTableEditor  tableEditor;
    QDialog              *tableEditorDialog;
    ChartTableView       *tableView;

    // Legend
    QButtonGroup         *positionButtonGroup;
    int                   lastHorizontalAlignment;
    int                   lastVerticalAlignment;
    KDChart::Position     fixedPosition;
    KDChart::Position     lastFixedPosition;
    
    int                   selectedDataset;
    
    QList<KDChart::CartesianAxis*> axes;
};

ChartConfigWidget::Private::Private()
{
    lastHorizontalAlignment = 1; // Qt::AlignCenter
    lastVerticalAlignment   = 1; // Qt::AlignCenter
    fixedPosition           = KDChart::Position::East;
    lastFixedPosition       = KDChart::Position::East;
}

ChartConfigWidget::Private::~Private()
{
}


ChartConfigWidget::ChartConfigWidget()
    : d(new Private)
{
    d->shape = 0;
    d->type    = KChart::LastChartType;
    d->subtype = KChart::NoChartSubtype;
    setObjectName("Chart Type");
    d->ui.setupUi( this );
    
    // Chart type button
    QMenu *chartTypeMenu = new QMenu( this );
    chartTypeMenu->setIcon( KIcon( "chart_bar_beside" ) );
    
    QMenu *barChartMenu = chartTypeMenu->addMenu( KIcon( "chart_bar" ), "Bar Chart" );
    d->normalBarChartAction  = barChartMenu->addAction( KIcon( "chart_bar_beside" ), i18n("Normal") );
    d->stackedBarChartAction = barChartMenu->addAction( KIcon( "chart_bar_layer" ), i18n("Stacked") );
    d->percentBarChartAction = barChartMenu->addAction( KIcon( "chart_bar_percent" ), i18n("Percent") );
    
    QMenu *lineChartMenu = chartTypeMenu->addMenu( KIcon( "chart_line" ), "Line Chart" );
    d->normalLineChartAction  = lineChartMenu->addAction( KIcon( "chart_line_normal" ), i18n("Normal") );
    d->stackedLineChartAction = lineChartMenu->addAction( KIcon( "chart_line_stacked" ), i18n("Stacked") );
    d->percentLineChartAction = lineChartMenu->addAction( KIcon( "chart_line_percent" ), i18n("Percent") );
    
    QMenu *areaChartMenu = chartTypeMenu->addMenu( KIcon( "chart_area" ), "Area Chart" );
    d->normalAreaChartAction  = areaChartMenu->addAction( KIcon( "chart_area_normal" ), i18n("Normal") );
    d->stackedAreaChartAction = areaChartMenu->addAction( KIcon( "chart_area_stacked" ), i18n("Stacked") );
    d->percentAreaChartAction = areaChartMenu->addAction( KIcon( "chart_area_percent" ), i18n("Percent") );
    
    d->ui.chartTypeMenu->setMenu( chartTypeMenu );
    
    connect( chartTypeMenu, SIGNAL( triggered( QAction* ) ), 
             this,          SLOT( chartTypeSelected( QAction* ) ) );
    
    // Quick-access settings
    d->tableEditorDialog = new QDialog( this );
    d->tableEditor.setupUi( d->tableEditorDialog );
    d->tableView = new ChartTableView;
    d->tableEditor.gridLayout->addWidget( d->tableView );
    d->tableEditorDialog->hide();
    
    connect( d->ui.threeDLook, SIGNAL( toggled( bool ) ),
             this, SLOT( setThreeDMode( bool ) ) );
    connect( d->ui.showLegend, SIGNAL( toggled( bool ) ),
             this, SIGNAL( showLegendChanged( bool ) ) );
    connect( d->ui.editData, SIGNAL( clicked( bool ) ),
             this, SLOT( slotShowTableEditor( bool ) ) );
    
    // "Axis Tab"
    //connect( d->ui.xAxisTitle, SIGNAL( textChanged( const QString& ) ),
    //         this, SIGNAL( xAxisTitleChanged( const QString& ) ) );
    //connect( d->ui.yAxisTitle, SIGNAL( textChanged( const QString& ) ),
    //         this, SIGNAL( yAxisTitleChanged( const QString& ) ) );
    //connect( d->ui.showVerticalLines, SIGNAL( toggled( bool ) ),
    //         this, SIGNAL( showVerticalLinesChanged( bool ) ) );
    //connect( d->ui.showHorizontalLines, SIGNAL( toggled( bool ) ),
    //         this, SIGNAL( showHorizontalLinesChanged( bool ) ) );
    
    // "Datasets" Tab
    connect( d->ui.datasetColor, SIGNAL( changed( const QColor& ) ),
             this, SLOT( datasetColorSelected( const QColor& ) ) );
    connect( d->ui.gapBetweenBars, SIGNAL( valueChanged( int ) ),
             this, SIGNAL( gapBetweenBarsChanged( int ) ) );
    connect( d->ui.gapBetweenSets, SIGNAL( valueChanged( int ) ),
             this, SIGNAL( gapBetweenSetsChanged( int ) ) );
    
    // "Legend" Tab
    connect( d->ui.legendTitle, SIGNAL( textChanged( const QString& ) ),
             this, SIGNAL( legendTitleChanged( const QString& ) ) );
    connect( d->ui.legendShowFrame, SIGNAL( toggled( bool ) ),
             this, SIGNAL( legendShowFrameChanged( bool ) ) );
    connect( d->ui.legendOrientationIsVertical, SIGNAL( toggled( bool ) ),
             this, SLOT( setLegendOrientationIsVertical( bool ) ) );
    
	d->ui.addAxis->setIcon( KIcon( "list-add" ) );
	d->ui.removeAxis->setIcon( KIcon( "list-remove" ) );
	
	connect( d->ui.axisTitle, SIGNAL( textChanged( const QString& ) ),
			 this, SLOT( ui_axisTitleChanged( const QString& ) ) );
	connect( d->ui.axisShowTitle, SIGNAL( toggled( bool ) ),
			 this, SLOT( ui_axisShowTitleChanged( bool ) ) );
	connect( d->ui.axisShowGridLines, SIGNAL( toggled( bool ) ),
			 this, SLOT( ui_axisShowGridLinesChanged( bool ) ) );
	connect ( d->ui.axes, SIGNAL( currentIndexChanged( int ) ),
			  this, SLOT( ui_axisSelectionChanged( int ) ) );

    createActions();

    // We need only connect one of the data direction buttons, since
    // they are mutually exclusive.
    //connect( d->ui.m_dataInRows, SIGNAL( toggled( bool ) ),
    //         this,               SLOT( setDataInRows( bool ) ) );
    //connect( d->ui.m_firstRowAsLabel, SIGNAL( toggled( bool ) ),
    //         this,                    SIGNAL( firstRowIsLabelChanged( bool ) ) );
    //connect( d->ui.m_firstColumnAsLabel, SIGNAL( toggled( bool ) ),
    //         this,                       SIGNAL( firstColumnIsLabelChanged( bool ) ) );
}

ChartConfigWidget::~ChartConfigWidget()
{
    delete d;
}

void ChartConfigWidget::open( KoShape* shape )
{
    d->shape = dynamic_cast<ChartShape*>( shape );
    
    // Update the axis titles
    //d->ui.xAxisTitle->setText( ((KDChart::AbstractCartesianDiagram*)d->shape->chart()->coordinatePlane()->diagram())->axes()[0]->titleText() );
    //d->ui.yAxisTitle->setText( ((KDChart::AbstractCartesianDiagram*)d->shape->chart()->coordinatePlane()->diagram())->axes()[1]->titleText() );
    
    // Update the legend title
    //d->ui.legendTitle->setText( d->shape->chart()->legend()->titleText() );
    
    // Fill the data table
    //d->tableView->setModel( ((ChartProxyModel*)d->shape->model()) );
    
    update();
}

void ChartConfigWidget::save()
{
    ChartTypeCommand command( d->shape );
    command.setChartType( d->type, d->subtype );
    command.redo();
}

KAction* ChartConfigWidget::createAction()
{
    return 0;
}

void ChartConfigWidget::chartTypeSelected( QAction *action )
{
    OdfChartType     type;
    OdfChartSubtype  subtype;
    
    if ( action == d->normalBarChartAction ) {
        type    = BarChartType;
        subtype = NormalChartSubtype;
    } else if ( action == d->stackedBarChartAction ) {
        type    = BarChartType;
        subtype = StackedChartSubtype;
    } else if ( action == d->percentBarChartAction ) {
        type    = BarChartType;
        subtype = PercentChartSubtype;
    } else if ( action == d->normalLineChartAction ) {
        type    = LineChartType;
        subtype = NormalChartSubtype;
    } else if ( action == d->stackedLineChartAction ) {
        type    = LineChartType;
        subtype = StackedChartSubtype;
    } else if ( action == d->percentLineChartAction ) {
        type    = LineChartType;
        subtype = PercentChartSubtype;
    } else if ( action == d->normalAreaChartAction ) {
        type    = AreaChartType;
        subtype = NormalChartSubtype;
    } else if ( action == d->stackedAreaChartAction ) {
        type    = AreaChartType;
        subtype = StackedChartSubtype;
    } else if ( action == d->percentAreaChartAction ) {
        type    = AreaChartType;
        subtype = PercentChartSubtype;
    }
    
    emit chartTypeChanged( type, subtype );
    
    update();
}

void ChartConfigWidget::chartSubtypeSelected( int type )
{
    d->subtype = (OdfChartSubtype) type;
    emit chartSubtypeChanged( d->subtype );
}

void ChartConfigWidget::datasetColorSelected( const QColor& color )
{
    emit datasetColorChanged( d->selectedDataset, color );
}

void ChartConfigWidget::setThreeDMode( bool threeD )
{
    d->threeDMode = threeD;
    emit threeDModeToggled( threeD );
}

void ChartConfigWidget::update()
{
    if ( !d->shape )
        return;
    
    // We only want to update this widget according to the current
    // state of the shape
    blockSignals( true );
    
    const KDChart::AbstractCartesianDiagram *cartesianDiagram = dynamic_cast<KDChart::AbstractCartesianDiagram*>(d->shape->diagram());
    
    // Update cartesian diagram-specific properties
    if ( cartesianDiagram ) {
    	if ( d->axes != cartesianDiagram->axes() ) {
    		d->axes = cartesianDiagram->axes();
    		
    		for ( int item = 0; item < d->ui.axes->count(); item++ ) {
    			d->ui.axes->removeItem( item );
    		}
    		foreach ( KDChart::CartesianAxis *axis, cartesianDiagram->axes() ) {
    			d->ui.axes->addItem( axis->titleText(), axis );
    		}
    		
    		const KDChart::CartesianAxis *selectedAxis = cartesianDiagram->axes().first();
    		const KDChart::CartesianCoordinatePlane *plane = dynamic_cast<KDChart::CartesianCoordinatePlane*>(d->shape->coordinatePlane());
    		
    		Q_ASSERT( plane );
    		
    		const Qt::Orientation axisOrientation = (   selectedAxis->position() == KDChart::CartesianAxis::Top
			                                         || selectedAxis->position() == KDChart::CartesianAxis::Bottom ) ?
			                                            Qt::Horizontal : Qt::Vertical;
    		d->ui.axisShowGridLines->setChecked( plane->gridAttributes( axisOrientation ).isGridVisible() );
    		d->ui.axisShowTitle->setChecked( !selectedAxis->titleText().isEmpty() );
    		d->ui.axisTitle->setText( selectedAxis->titleText() );
    	}
    }
    
    if (    d->type    != d->shape->chartType()
         || d->subtype != d->shape->chartSubtype() )
    {
        bool needSeparator = false;
        if ( d->shape->chartType() == BarChartType ) {
            d->ui.barPropertiesLabel->show();
            d->ui.gapBetweenBarsLabel->show();
            d->ui.gapBetweenSetsLabel->show();
            d->ui.gapBetweenBars->show();
            d->ui.gapBetweenSets->show();
            needSeparator = true;
        } else {
            d->ui.barPropertiesLabel->hide();
            d->ui.gapBetweenBarsLabel->hide();
            d->ui.gapBetweenSetsLabel->hide();
            d->ui.gapBetweenBars->hide();
            d->ui.gapBetweenSets->hide();
        }
        d->ui.propertiesSeparator->setVisible( needSeparator );
        switch ( d->shape->chartSubtype() ) {
        case NormalChartSubtype:
            switch ( d->shape->chartType() ) {
            case BarChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_bar_beside" ) );
                break;
            case LineChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_line_normal" ) );
                break;
            case AreaChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_area_normal" ) );
                break;
            case CircleChartType:
            case RingChartType:
            case RadarChartType:
            case StockChartType:
            case BubbleChartType:
            case SurfaceChartType:
            case GanttChartType:
            case LastChartType:
            default:
                break;
            }
            break;
        case StackedChartSubtype:
            //d->ui.subtypeStacked->blockSignals( true );
            //d->ui.subtypeStacked->setChecked( true );
            //d->ui.subtypeStacked->blockSignals( false );
            switch ( d->shape->chartType() ) {
            case BarChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_bar_layer" ) );
                break;
            case LineChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_line_stacked" ) );
                break;
            case AreaChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_area_stacked" ) );
                break;
            case CircleChartType:
            case RingChartType:
            case RadarChartType:
            case StockChartType:
            case BubbleChartType:
            case SurfaceChartType:
            case GanttChartType:
            case LastChartType:
            default:
                break;
            }
            break;
        case PercentChartSubtype:
            //d->ui.subtypePercent->blockSignals( true );
            //d->ui.subtypePercent->setChecked( true );
            //d->ui.subtypePercent->blockSignals( false );
            switch ( d->shape->chartType() ) {
            case BarChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_bar_percent" ) );
                break;
            case LineChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_line_percent" ) );
                break;
            case AreaChartType:
                d->ui.chartTypeMenu->setIcon( KIcon( "chart_area_percent" ) );
                break;
            case CircleChartType:
            case RingChartType:
            case RadarChartType:
            case StockChartType:
            case BubbleChartType:
            case SurfaceChartType:
            case GanttChartType:
            case LastChartType:
            default:
                break;
            }
            break;
        case NoChartSubtype:
        default:
            break;
        }
        d->type    = d->shape->chartType();
        d->subtype = d->shape->chartSubtype();
    }
    
    d->ui.threeDLook->setChecked( d->shape->threeDMode() );
    if ( d->shape->legend() ) {
        d->ui.legendTitle->setText( d->shape->legend()->titleText() );
    }
    
    blockSignals( false );
}


void ChartConfigWidget::slotShowTableEditor( bool show )
{
    if ( show )
        d->tableEditorDialog->hide();
    else
        d->tableEditorDialog->show();
}

void ChartConfigWidget::setDataInRows( bool checked )
{
    if ( checked )
        emit dataDirectionChanged( Qt::Horizontal );
    else
        emit dataDirectionChanged( Qt::Vertical );
}


void ChartConfigWidget::setLegendOrientation( int boxEntryIndex )
{
    //emit legendOrientationChanged( ( Qt::Orientation ) ( d->ui.orientation->itemData( boxEntryIndex ).toInt() ) );
}
/*
void ChartConfigWidget::setLegendShowTitle( bool show )
{
    if ( show ) {
        d->ui.legendTitle->setEnabled( true );
        emit legendTitleChanged( d->ui.legendTitle->text() );
    } else {
        d->ui.legendTitle->setEnabled( false );
        emit legendTitleChanged( "" );
    }
}
*/
void ChartConfigWidget::setLegendAlignment( int boxEntryIndex )
{
    if (    d->fixedPosition == KDChart::Position::North
         || d->fixedPosition == KDChart::Position::South ) {
        //d->lastHorizontalAlignment = d->ui.alignment->currentIndex();
     } else if (   d->fixedPosition == KDChart::Position::East
                || d->fixedPosition == KDChart::Position::West ) {
        //d->lastVerticalAlignment = d->ui.alignment->currentIndex();
    }
    //emit legendAlignmentChanged( ( Qt::Alignment ) ( d->ui.alignment->itemData( boxEntryIndex ).toInt() ) );
}

void ChartConfigWidget::setLegendFixedPosition( int buttonGroupIndex )
{
    d->lastFixedPosition = d->fixedPosition;
    //d->fixedPosition = buttonIndexToFixedPosition[ buttonGroupIndex ];
    //emit legendFixedPositionChanged( buttonIndexToFixedPosition[ buttonGroupIndex ] );
}

void ChartConfigWidget::updateFixedPosition( const KDChart::Position position )
{
/*
    if (    position == KDChart::Position::North
         || position == KDChart::Position::South ) {
        d->ui.alignment->setEnabled( true );
        d->ui.alignment->setItemText( 0, i18n( "Left" ) );
        d->ui.alignment->setItemData( 0, Qt::AlignLeft );
        d->ui.alignment->setItemData( 1, Qt::AlignCenter );
        d->ui.alignment->setItemText( 2, i18n( "Right" ) );
        d->ui.alignment->setItemData( 2, Qt::AlignRight );
        // Set the alignment to the one last used for horizontal legend alignment
        if (    d->lastFixedPosition != KDChart::Position::North
             && d->lastFixedPosition != KDChart::Position::South ) {
            // Make sure that the combobox gets updated. Since we changed the values of the entries,
            // same index doesn't mean same value, though it will think so. Solution: Select no entry first
            d->ui.alignment->blockSignals( true );
            d->ui.alignment->setCurrentIndex( -1 );
            d->ui.alignment->blockSignals( false );

            d->ui.alignment->setCurrentIndex( d->lastHorizontalAlignment );
        }
    } else if (    position == KDChart::Position::East
                || position == KDChart::Position::West ) {
        d->ui.alignment->setEnabled( true );
        d->ui.alignment->setItemText( 0, i18n( "Top" ) );
        d->ui.alignment->setItemData( 0, Qt::AlignTop );
        d->ui.alignment->setItemData( 1, Qt::AlignCenter );
        d->ui.alignment->setItemText( 2, i18n( "Bottom" ) );
        d->ui.alignment->setItemData( 2, Qt::AlignBottom );
        // Set the alignment to the one last used for vertical legend alignment
        if (    d->lastFixedPosition != KDChart::Position::East
             && d->lastFixedPosition != KDChart::Position::West ) {
            // Make sure that the combobox gets updated. Since we changed the values of the entries,
            // same index doesn't mean same value, though it will think so. Solution: Select no entry first
            d->ui.alignment->blockSignals( true );
            d->ui.alignment->setCurrentIndex( -1 );
            d->ui.alignment->blockSignals( false );

            d->ui.alignment->setCurrentIndex( d->lastVerticalAlignment );
        }
    } else {
        d->ui.alignment->setEnabled( false );
    }

    for( int i = 0; i < NUM_FIXED_POSITIONS; i++ ) {
        if( position == buttonIndexToFixedPosition[i] ) {
            if ( d->positionButtonGroup->checkedId() != i )
                d->positionButtonGroup->button( i )->setChecked( true );
            break;
        }
    }
*/
}


void ChartConfigWidget::createActions()
{
    QAction *cutRowsAction       = new QAction( KIcon( "edit-cut" ), i18n( "Cut Rows" ),    d->tableView );
    QAction *cutColumnsAction    = new QAction( KIcon( "edit-cut" ), i18n( "Cut Columns" ), d->tableView );
    QAction *cutCellsAction      = new QAction( KIcon( "edit-cut" ), i18n( "Cut Cells" ), d->tableView );
    QAction *copyRowsAction      = new QAction( KIcon( "edit-copy" ), i18n( "Copy Rows" ), d->tableView );
    QAction *copyColumnsAction   = new QAction( KIcon( "edit-copy" ), i18n( "Copy Columns" ), d->tableView );
    QAction *copyCellsAction     = new QAction( KIcon( "edit-copy" ), i18n( "Copy Cells" ), d->tableView );
    QAction *deleteRowsAction    = new QAction( KIcon( "edit-delete" ), i18n( "Delete Rows" ), d->tableView );
    QAction *deleteColumnsAction = new QAction( KIcon( "edit-delete" ), i18n( "Delete Columns" ), d->tableView );
    QAction *deleteCellsAction   = new QAction( KIcon( "edit-delete" ), i18n( "Delete Cells" ), d->tableView );
    QAction *insertRowsAction    = new QAction( KIcon( "edit-paste" ), i18n( "Insert Rows" ), d->tableView );
    QAction *insertColumnsAction = new QAction( KIcon( "edit-paste" ), i18n( "Insert Columns" ), d->tableView );
    QAction *insertCellsAction   = new QAction( KIcon( "edit-paste" ), i18n( "Insert Cells" ), d->tableView );

    QAction *separator1 = new QAction( d->tableView );
    QAction *separator2 = new QAction( d->tableView );
    separator1->setSeparator( true );
    separator2->setSeparator( true );

    d->tableView->addAction( copyRowsAction );
    d->tableView->addAction( cutRowsAction );
    d->tableView->addAction( deleteRowsAction );
    d->tableView->addAction( insertRowsAction );
    d->tableView->addAction( separator1 );
    d->tableView->addAction( copyColumnsAction );
    d->tableView->addAction( cutColumnsAction );
    d->tableView->addAction( deleteColumnsAction );
    d->tableView->addAction( insertColumnsAction );
    d->tableView->addAction( separator2 );
    d->tableView->addAction( copyCellsAction );
    d->tableView->addAction( cutCellsAction );
    d->tableView->addAction( deleteCellsAction );
    d->tableView->addAction( insertCellsAction );

    d->tableView->setContextMenuPolicy( Qt::ActionsContextMenu );
}

void ChartConfigWidget::selectDataset( int dataset )
{
    if ( dataset >= 0 ) {
        d->ui.datasetColorLabel->setEnabled( true );
        d->ui.datasetColor->setEnabled( true );
        
        d->ui.datasetColor->blockSignals( true );
        if ( d->shape && d->shape->chart() && d->shape->chart()->coordinatePlane() ) {
            d->ui.datasetColor->setColor( d->shape->chart()->coordinatePlane()->diagram()->brush( dataset ).color() );
        }
        d->ui.datasetColor->blockSignals( false );
    } else {
        d->ui.datasetColorLabel->setEnabled( false );
        d->ui.datasetColor->setEnabled( false );
    }
    d->selectedDataset = dataset;
}

void ChartConfigWidget::setLegendOrientationIsVertical( bool b )
{
    if ( b )
        emit legendOrientationChanged( Qt::Vertical );
    else
        emit legendOrientationChanged( Qt::Horizontal );
}

void ChartConfigWidget::ui_axisSelectionChanged( int index ) {
	Q_ASSERT( d->axes.size() > index );
	
	const KDChart::CartesianAxis *axis = d->axes[ index ];
	d->ui.axisTitle->setText( axis->titleText() );
	d->ui.axisShowTitle->setChecked( !axis->titleText().isEmpty() );
	
	KDChart::CartesianCoordinatePlane *plane = dynamic_cast<KDChart::CartesianCoordinatePlane*>( d->shape->diagram()->coordinatePlane() );
	
	// Avoid crash since having a non-cartesian coordinate plane means we can't set
	// properties for cartesian axes, but is not fatal
	if ( !plane )
		return;
	
	const Qt::Orientation axisOrientation = ( axis->position() == KDChart::CartesianAxis::Top || axis->position() == KDChart::CartesianAxis::Bottom ) ?
								            Qt::Horizontal : Qt::Vertical;
	KDChart::GridAttributes attributes = plane->gridAttributes( axisOrientation );
	d->ui.axisShowGridLines->setChecked( attributes.isGridVisible() );
}

void ChartConfigWidget::ui_axisTitleChanged( const QString& title ) {
	Q_ASSERT( d->axes.size() > d->ui.axes->currentIndex() );
	
	emit axisTitleChanged( d->axes[ d->ui.axes->currentIndex() ], title );
}

void ChartConfigWidget::ui_axisShowTitleChanged( bool b ) {
	Q_ASSERT( d->axes.size() > d->ui.axes->currentIndex() );
	
	// To hide the axis title, we pass an empty string
	emit axisTitleChanged( d->axes[ d->ui.axes->currentIndex() ], b ? d->ui.axisTitle->text() : "" );
}

void ChartConfigWidget::ui_axisShowGridLinesChanged( bool b ) {
	Q_ASSERT( d->axes.size() > d->ui.axes->currentIndex() );
	
	emit axisShowGridLinesChanged( d->axes[ d->ui.axes->currentIndex() ], b );
}

#include "ChartConfigWidget.moc"
