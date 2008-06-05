/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "PageLayoutDialog.h"

// KSpread
#include "Map.h"
#include "PrintSettings.h"
#include "Sheet.h"
#include "ui_PageLayoutSheetPage.h"

// KDE
#include <klocale.h>

using namespace KSpread;

class PageLayoutDialog::Private
{
public:
    Sheet* sheet;
    Ui::SheetPage sheetPage;

public:
    void setup();
};

void PageLayoutDialog::Private::setup()
{
    const PrintSettings* settings = sheet->printSettings();
    sheetPage.gridCheckBox->setChecked(settings->printGrid());
    sheetPage.commentCheckBox->setChecked(settings->printCommentIndicator());
    sheetPage.formulaCheckBox->setChecked(settings->printFormulaIndicator());
    sheetPage.chartsCheckBox->setChecked(settings->printCharts());
    sheetPage.objectsCheckBox->setChecked(settings->printObjects());
    sheetPage.drawingsCheckBox->setChecked(settings->printGraphics());
    sheetPage.zeroValuesCheckBox->setChecked(settings->printZeroValues());
    sheetPage.headersCheckBox->setChecked(settings->printHeaders());
    sheetPage.ltrButton->setChecked(settings->pageOrder() == PrintSettings::LeftToRight);

    QStringList zoomLevels;
    for (int zoomLevel = 25; zoomLevel <= 500; zoomLevel += 25)
    {
        zoomLevels.append(i18n("%1%", zoomLevel));
        if (qRound(settings->zoom() * 100) > zoomLevel &&
            qRound(settings->zoom() * 100) < zoomLevel + 25)
        {
            zoomLevels.append(i18n("%1%", qRound(settings->zoom() * 100)));
        }
    }
    sheetPage.zoomComboBox->insertItems(0, zoomLevels);
    const QString zoomLevel = i18n("%1%", qRound(settings->zoom() * 100));
    sheetPage.zoomComboBox->setCurrentIndex(sheetPage.zoomComboBox->findText(zoomLevel));

    const QSize pageLimits = settings->pageLimits();
    sheetPage.pageLimitsButton->setChecked(pageLimits.isValid());

    QStringList limits;
    limits.append(i18n("No Limit"));
    for (int limit = 1; limit <= 20; ++limit)
        limits.append(QString("%1").arg(limit));
    sheetPage.horizontalComboBox->insertItems(0, limits);
    sheetPage.verticalComboBox->insertItems(0, limits);
    if (pageLimits.width() > 20)
    {
        sheetPage.horizontalComboBox->addItem(QString("%1").arg(pageLimits.width()));
        sheetPage.horizontalComboBox->setCurrentIndex(20);
    }
    else
        sheetPage.horizontalComboBox->setCurrentIndex(qMax(0, pageLimits.width()));
    if (pageLimits.height() > 20)
    {
        sheetPage.verticalComboBox->addItem(QString("%1").arg(pageLimits.height()));
        sheetPage.verticalComboBox->setCurrentIndex(20);
    }
    else
        sheetPage.verticalComboBox->setCurrentIndex(qMax(0, pageLimits.height()));
}


PageLayoutDialog::PageLayoutDialog(QWidget* parent, Sheet* sheet)
    : KoPageLayoutDialog(parent, sheet->printSettings()->pageLayout())
    , d(new Private)
{
    setFaceType(KPageDialog::Tabbed);
    showPageSpread(false);

    QWidget* page = new QWidget(this);
    d->sheetPage.setupUi(page);
    addPage(page, i18n("Sheet"));

    connect(d->sheetPage.zoomButton, SIGNAL(toggled(bool)),
            d->sheetPage.zoomComboBox, SLOT(setEnabled(bool)));
    connect(d->sheetPage.pageLimitsButton, SIGNAL(toggled(bool)),
            d->sheetPage.horizontalComboBox, SLOT(setEnabled(bool)));
    connect(d->sheetPage.pageLimitsButton, SIGNAL(toggled(bool)),
            d->sheetPage.verticalComboBox, SLOT(setEnabled(bool)));

    d->sheet = sheet;
    d->setup();
}

PageLayoutDialog::~PageLayoutDialog()
{
    delete d;
}

void PageLayoutDialog::accept()
{
    PrintSettings settings;
    settings.setPageLayout(pageLayout());
    settings.setPrintGrid(d->sheetPage.gridCheckBox->isChecked());
    settings.setPrintCommentIndicator(d->sheetPage.commentCheckBox->isChecked());
    settings.setPrintFormulaIndicator(d->sheetPage.formulaCheckBox->isChecked());
    settings.setPrintCharts(d->sheetPage.chartsCheckBox->isChecked());
    settings.setPrintGraphics(d->sheetPage.drawingsCheckBox->isChecked());
    settings.setPrintObjects(d->sheetPage.objectsCheckBox->isChecked());
    settings.setPrintZeroValues(d->sheetPage.zeroValuesCheckBox->isChecked());
    settings.setPrintHeaders(d->sheetPage.headersCheckBox->isChecked());
    settings.setPageOrder(d->sheetPage.ltrButton->isChecked() ? PrintSettings::LeftToRight : PrintSettings::TopToBottom);

    bool isValid = false;
    settings.setZoom(0.01 * d->sheetPage.zoomComboBox->currentText().replace('%', "").toDouble(&isValid));
    if (!isValid)
        settings.setZoom(1.0);

    QSize pageLimits;
    if (d->sheetPage.pageLimitsButton->isChecked())
    {
        pageLimits.setWidth(d->sheetPage.horizontalComboBox->currentText().toInt(&isValid));
        if (!isValid)
            pageLimits.setWidth(0);
        pageLimits.setHeight(d->sheetPage.verticalComboBox->currentText().toInt(&isValid));
        if (!isValid)
            pageLimits.setHeight(0);
    }
    settings.setPageLimits(pageLimits);

    if (applyToDocument())
    {
        // Apply to all sheets.
        const QList<Sheet*> sheets = d->sheet->map()->sheetList();
        for (int i = 0; i < sheets.count(); ++i)
            *sheets[i]->printSettings() = settings;
    }
    else
        *d->sheet->printSettings() = settings;

    KoPageLayoutDialog::accept();
}
