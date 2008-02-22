SET( qucstools_sources global.cpp units.cpp propertygrid.cpp helpdialog.cpp)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

SET( qucstools_moc_headers propertygrid.h )

QT4_WRAP_CPP( qucstools_moc_sources ${qucstools_moc_headers} )

ADD_LIBRARY( qucstools ${qucstools_sources} ${qucstools_moc_sources} )
