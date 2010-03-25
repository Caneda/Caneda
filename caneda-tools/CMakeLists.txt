SET( canedatools_sources global.cpp units.cpp propertygrid.cpp helpdialog.cpp)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

SET( canedatools_moc_headers propertygrid.h )

QT4_WRAP_CPP( canedatools_moc_sources ${canedatools_moc_headers} )

ADD_LIBRARY( canedatools ${canedatools_sources} ${canedatools_moc_sources} )
