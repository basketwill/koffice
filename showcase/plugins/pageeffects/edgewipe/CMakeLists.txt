include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  ${CMAKE_SOURCE_DIR}/showcase/part ${CMAKE_SOURCE_DIR}/showcase/part/pageeffects )

set(showcase_pageeffect_edgewipe_SRCS
    Plugin.cpp

    miscdiagonalwipe/MiscDiagonalWipeEffectFactory.cpp
    miscdiagonalwipe/DoubleBarnDoorWipeStrategy.cpp
    miscdiagonalwipe/DoubleDiamondWipeStrategy.cpp

    fourboxwipe/FourBoxWipeEffectFactory.cpp
    fourboxwipe/CornersInWipeStrategy.cpp
    fourboxwipe/CornersOutWipeStrategy.cpp

    diagonalwipe/DiagonalWipeEffectFactory.cpp
    diagonalwipe/DiagonalWipeStrategy.cpp

    barndoorwipe/BarnDoorWipeEffectFactory.cpp
    barndoorwipe/BarnDoorWipeStrategy.cpp

    boxwipe/BoxWipeEffectFactory.cpp
    boxwipe/BoxWipeStrategy.cpp

    veewipe/VeeWipeEffectFactory.cpp
    veewipe/VeeWipeStrategy.cpp

    barnveewipe/BarnVeeWipeEffectFactory.cpp
    barnveewipe/BarnVeeWipeStrategy.cpp

    zigzagwipe/ZigZagWipeEffectFactory.cpp
    zigzagwipe/ZigZagWipeStrategy.cpp

    barnzigzagwipe/BarnZigZagWipeEffectFactory.cpp
    barnzigzagwipe/BarnZigZagWipeStrategy.cpp
    )

kde4_add_plugin(showcase_pageeffect_edgewipe ${showcase_pageeffect_edgewipe_SRCS})

target_link_libraries(showcase_pageeffect_edgewipe showcaseprivate)

install(TARGETS showcase_pageeffect_edgewipe  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES showcase_pageeffect_edgewipe.desktop DESTINATION ${SERVICES_INSTALL_DIR})


