/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TestDependencies.h"

#include "qtest_kde.h"

#include "KCCellStorage.h"
#include "KCDependencyManager.h"
#include "DependencyManager_p.h"
#include "KCFormula.h"
#include "KCMap.h"
#include "KCRegion.h"
#include "KCSheet.h"
#include "KCValue.h"

void TestDependencies::initTestCase()
{
    m_map = new KCMap(0 /* no KCDoc */);
    m_sheet = m_map->addNewSheet();
    m_sheet->setSheetName("Sheet1");
    m_storage = m_sheet->cellStorage();
}

void TestDependencies::testCircleRemoval()
{
    KCFormula formula(m_sheet);
    formula.setExpression("=A1");
    m_storage->setFormula(1, 1, formula); // A1

    QApplication::processEvents(); // handle Damages

    QCOMPARE(m_storage->value(1, 1), KCValue::errorCIRCLE());
    KCDependencyManager* manager = m_map->dependencyManager();
    QVERIFY(manager->d->consumers.count() == 1);
    QVERIFY(manager->d->providers.count() == 1);
    QList<KCCell> consumers = manager->d->consumers.value(m_sheet)->contains(QRect(1, 1, 1, 1));
    QCOMPARE(consumers.count(), 1);
    QCOMPARE(consumers.first(), KCCell(m_sheet, 1, 1));
    QCOMPARE(manager->d->providers.value(KCCell(m_sheet, 1, 1)), KCRegion(QRect(1, 1, 1, 1), m_sheet));

    m_storage->setFormula(1, 1, KCFormula()); // A1

    QApplication::processEvents(); // handle Damages

    QCOMPARE(m_storage->value(1, 1), KCValue());
    QVERIFY(manager->d->consumers.value(m_sheet)->contains(QRect(1, 1, 1, 1)).count() == 0);
    QVERIFY(manager->d->providers.count() == 0);
}

void TestDependencies::testCircles()
{
    KCFormula formula(m_sheet);
    formula.setExpression("=A3");
    m_storage->setFormula(1, 1, formula); // A1
    formula.setExpression("=A1");
    m_storage->setFormula(1, 2, formula); // A2
    formula.setExpression("=A2");
    m_storage->setFormula(1, 3, formula); // A3

    QApplication::processEvents(); // handle Damages

    QCOMPARE(m_storage->value(1, 1), KCValue::errorCIRCLE());
    QCOMPARE(m_storage->value(1, 2), KCValue::errorCIRCLE());
    QCOMPARE(m_storage->value(1, 3), KCValue::errorCIRCLE());
}

void TestDependencies::cleanupTestCase()
{
    delete m_map;
}

QTEST_KDEMAIN(TestDependencies, GUI)

#include "TestDependencies.moc"
