/* This file is part of the KDE project
   Copyright 2007 Ariya Hidayat <ariya@kde.org>
   Copyright 2007 Sascha Pfau <MrPeacock@gmail.com>

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

#ifndef KSPREAD_TEST_MATH_FUNCTIONS
#define KSPREAD_TEST_MATH_FUNCTIONS

#include <QtGui>
#include <QtTest/QtTest>

#include <Value.h>

namespace KSpread
{

class TestMathFunctions: public QObject
{
Q_OBJECT

private slots:
  void testABS();
  void testACOS();
  void testACOSH();
  void testACOT();
//   void testACOTH(); // to be implemented
  void testASIN();
  void testASINH();
  void testATAN();
  void testATAN2();
  void testATANH();
  void testBESSELI();
  void testBESSELJ();
  void testBESSELK();
  void testBESSELY();
  void testCOMBIN();
//   void testCOMBINA(); // to be implemented
  void testCONVERT();
  void testCEIL();
  void testCEILING();
//   void testCOT();     // to be implemented
//   void testCOTH();    // to be implemented
  void testDEGREES();
  void testDELTA();
//   void testERF();  -> TestEngineering
//   void testERFC(); -> TestEngineering
  void testEVEN();
  void testEXP();
  void testFACT();
  void testFACTDOUBLE();
  void testFIB();
  void testGAMMA();
  void testGAMMALN();
  void testGCD();
  void testGESTEP();
  void testLCM();
  void testLN();
  void testLOG();
  void testLOG10();
  void testMDETERM();
  void testMINVERSE();
  void testMMULT();
  void testMOD();
//   void testMULTINOMINAL();
  void testMUNIT();
  void testODD();
//   void testPI(); -> TestEngineering
  void testPOWER();
  void testPRODUCT();
  void testQUOTIENT();
  void testRADIANS();
  void testRAND();
  void testRANDBETWEEN();
  void testSERIESSUM();
  void testSIGN();
  void testSQRT();
  void testSQRTPI();
  void testSUBTOTAL();
  void testSUMA();
//   void testSUMIF();
  void testSUMSQ();

private:
  Value evaluate(const QString&);
};

} // namespace KSpread

#endif // KSPREAD_TEST_MATH_FUNCTIONS
