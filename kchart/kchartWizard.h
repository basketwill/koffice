#ifndef kchartWIZARD_H
#define kchartWIZARD_H

#include <klocale.h>
#include "kwizard.h"
#include "kchart_part.h"

class KChartPart;
class KChartWizardSetupAxesPage;
class KChartWizardLabelsLegendPage;
class KChartWizardSelectChartSubTypePage;
class KChartWizardSelectDataPage;
class KChartWizardSetupDataPage;
class KChartWizardSelectChartTypePage;

class KChartWizard : public KWizard
{
    Q_OBJECT
public:
    KChartWizard ( KChartPart* chart, QWidget *parent, const char* name,
				   bool modal = true, WFlags f = 0 );
    ~KChartWizard();

    KChartPart* chart() const { return _chart; };
  void setDataArea( const QString &area );
  QString dataArea() const;

  enum RowCol { Row, Col };

  void emitNeedNewData( const char* area, int rowcol, bool firstRowIsLegend,
						bool firstColIsLabel );

  virtual bool appropriate( QWidget * w ) const;

signals:
  // valid values for rowcol: Row: data is in rows, Col: data is in cols
  void needNewData( const char* area, int rowcol, bool firstRowIsLegend,
					bool firstColIsLabel );
  void finished();
  void cancelled();

protected slots:
  virtual void next();
  virtual void reject();
  virtual void accept();
  virtual void subType(int);
private:
  KChartPart* _chart;
  KChartWizardSelectDataPage* _selectdatapage;
  KChartWizardSelectChartTypePage* _selectcharttypepage;
  KChartWizardSelectChartSubTypePage* _selectchartsubtypepage;
  KChartWizardSetupDataPage* _setupdatapage;
  KChartWizardLabelsLegendPage* _labelslegendpage;
  KChartWizardSetupAxesPage* _axespage;
};

#endif


