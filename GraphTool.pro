#-------------------------------------------------
#
# Project created by QtCreator 2013-07-31T14:34:23
#
#-------------------------------------------------

gcc:QMAKE_CXXFLAGS += -std=c++11 -g

QT       += core gui opengl openglextensions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GraphTool
TEMPLATE = app


SOURCES += \
    source/rendering/camera.cpp \
    source/rendering/material.cpp \
    source/rendering/opengldebugmessagemodel.cpp \
    source/rendering/openglwindow.cpp \
    source/rendering/primitives/cube.cpp \
    source/rendering/primitives/cylinder.cpp \
    source/rendering/primitives/quad.cpp \
    source/rendering/primitives/sphere.cpp \
    source/rendering/sampler.cpp \
    source/rendering/texture.cpp \
    source/rendering/transition.cpp \
    source/graph/genericgraphmodel.cpp \
    source/graph/graph.cpp \
    source/graph/simplecomponentmanager.cpp \
    source/layout/centreinglayout.cpp \
    source/layout/collision.cpp \
    source/layout/eadeslayout.cpp \
    source/layout/layout.cpp \
    source/layout/randomlayout.cpp \
    source/layout/scalinglayout.cpp \
    source/layout/spatialoctree.cpp \
    source/main.cpp \
    source/mainwindow.cpp \
    source/maths/boundingbox.cpp \
    source/maths/boundingsphere.cpp \
    source/maths/frustum.cpp \
    source/maths/plane.cpp \
    source/maths/ray.cpp \
    source/parsers/gmlfileparser.cpp \
    source/ui/commandmanager.cpp \
    source/ui/selectionmanager.cpp \
    source/rendering/graphcomponentscene.cpp \
    source/ui/graphcomponentinteractor.cpp \
    source/ui/graphwidget.cpp \
    source/ui/mainwidget.cpp \
    source/parsers/graphfileparser.cpp

HEADERS += \
    source/rendering/camera.h \
    source/rendering/material.h \
    source/rendering/opengldebugmessagemodel.h \
    source/rendering/openglwindow.h \
    source/rendering/primitives/cube.h \
    source/rendering/primitives/cylinder.h \
    source/rendering/primitives/quad.h \
    source/rendering/primitives/sphere.h \
    source/rendering/sampler.h \
    source/rendering/scene.h \
    source/rendering/texture.h \
    source/rendering/transition.h \
    source/graph/componentmanager.h \
    source/graph/genericgraphmodel.h \
    source/graph/grapharray.h \
    source/graph/graph.h \
    source/graph/graphmodel.h \
    source/graph/simplecomponentmanager.h \
    source/layout/centreinglayout.h \
    source/layout/collision.h \
    source/layout/eadeslayout.h \
    source/layout/layout.h \
    source/layout/randomlayout.h \
    source/layout/scalinglayout.h \
    source/layout/sequencelayout.h \
    source/layout/spatialoctree.h \
    source/mainwindow.h \
    source/maths/boundingbox.h \
    source/maths/boundingsphere.h \
    source/maths/constants.h \
    source/maths/frustum.h \
    source/maths/interpolation.h \
    source/maths/line.h \
    source/maths/plane.h \
    source/maths/ray.h \
    source/parsers/gmlfileparser.h \
    source/parsers/graphfileparser.h \
    source/ui/commandmanager.h \
    source/ui/selectionmanager.h \
    source/utils.h \
    source/rendering/graphcomponentscene.h \
    source/ui/graphcomponentinteractor.h \
    source/ui/interactor.h \
    source/ui/graphwidget.h \
    source/ui/mainwidget.h

FORMS    += source/mainwindow.ui

OTHER_FILES += \
    source/rendering/shaders/2d.vert \
    source/rendering/shaders/ads.frag \
    source/rendering/shaders/debuglines.frag \
    source/rendering/shaders/debuglines.vert \
    source/rendering/shaders/instancededges.vert \
    source/rendering/shaders/instancedmarkers.vert \
    source/rendering/shaders/instancednodes.vert \
    source/rendering/shaders/marker.frag \
    source/rendering/shaders/screen.frag \
    source/rendering/shaders/screen.vert \
    source/rendering/shaders/selection.frag \
    source/rendering/shaders/selectionMarker.frag

RESOURCES += \
    source/resources.qrc
