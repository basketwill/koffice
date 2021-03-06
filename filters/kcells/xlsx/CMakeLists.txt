add_definitions(-DKDE_DEFAULT_DEBUG_AREA=30527)

include_directories( ${KOMAIN_INCLUDES} ${CMAKE_SOURCE_DIR}/filters/libmsooxml )

########### next target ###############

set(xlsximport_PART_SRCS
    ColorConversions.cpp
    XlsxImport.cpp
    XlsxXmlCommonReader.cpp
    XlsxXmlDocumentReader.cpp
    XlsxXmlWorksheetReader.cpp
    XlsxXmlSharedStringsReader.cpp
    XlsxXmlStylesReader.cpp
    XlsxXmlDrawingReader.cpp
    XlsxXmlChartReader.cpp
    XlsxXmlCommentsReader.cpp
    NumberFormatParser.cpp
    ChartExport.cpp
    FormulaParser.cpp
)

kde4_add_plugin(xlsximport ${xlsximport_PART_SRCS})

target_link_libraries(
    xlsximport
    msooxml
    komain
    kcellscommon
)

install(TARGETS xlsximport DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES
  kcells_xlsx_import.desktop
#  kcells_xlsx_export.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)

########## unit tests ###################

set(TestFormulaParser_SRCS
    FormulaParser.cpp
    TestFormulaParser.cpp
)

kde4_add_unit_test(TestFormulaParser TESTNAME kcells-xlsx-FormulaParser ${TestFormulaParser_SRCS})
target_link_libraries(TestFormulaParser msooxml kcellscommon ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
