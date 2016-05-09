#-------------------------------------------------
#
# Project created by QtCreator 2016-04-23T17:31:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RayTracer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imagecontainer.cpp \
    ray.cpp \
    raytracer.cpp \
    sphere.cpp \
    hitablelist.cpp \
    camera.cpp \
    lambertian.cpp \
    utils.cpp \
    metal.cpp \
    dielectric.cpp \
    movingsphere.cpp \
    aabbox.cpp \
    bvh.cpp \
    constanttexture.cpp \
    checkertexture.cpp \
    perlin.cpp \
    noisetexture.cpp

HEADERS  += mainwindow.h \
    imagecontainer.h \
    ray.h \
    raytracer.h \
    hitable.h \
    sphere.h \
    hitablelist.h \
    camera.h \
    material.h \
    lambertian.h \
    utils.h \
    metal.h \
    dielectric.h \
    movingsphere.h \
    aabbox.h \
    bvh.h \
    texture.h \
    constanttexture.h \
    checkertexture.h \
    perlin.h \
    noisetexture.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS+= -fopenmp -O2
QMAKE_LFLAGS +=  -fopenmp -O2
