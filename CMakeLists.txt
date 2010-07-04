# ==================================================================================
# Caneda project
PROJECT( Caneda )

SET( PACKAGE_VERSION "0.1.0" )
SET( PACKAGE_STRING "caneda 0.1.0" )

# ==================================================================================
# Configure QT options

SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTSVG TRUE )
SET( QT_USE_QTCORE TRUE )
SET( QT_USE_QTGUI TRUE )

# ==================================================================================
# Minimum libraries required

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

SET( QT_MIN_VERSION 4.6.0 )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )

# For relaxNG
FIND_PACKAGE( LibXml2 REQUIRED )
ADD_DEFINITIONS( ${LIBXML2_DEFINITIONS} )

# For libxslt
FIND_PACKAGE( LibXslt REQUIRED )
ADD_DEFINITIONS( ${LIBXSLT_DEFINITIONS} )

# ==================================================================================
# Configure runtime directories

SET( BASEDIR     "share/caneda/" )
SET( BITMAPDIR   "share/caneda/bitmaps/" )
SET( BINARYDIR   "bin/" )
SET( DOCDIR      "share/caneda/docs/" )
SET( MANDIR      "share/man/man1/" )
SET( LANGUAGEDIR "share/caneda/lang/" )
SET( LIBRARYDIR  "share/caneda/library/" )

# ==================================================================================
# Configure files and compilation options

CONFIGURE_FILE( ${Caneda_SOURCE_DIR}/config.h.cmake ${Caneda_BINARY_DIR}/config.h )
SET( CMAKE_BUILD_TYPE Debug )
ADD_DEFINITIONS( -DHAVE_CONFIG_H )

# ==================================================================================
# Include sources directories

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/caneda
    ${LIBXSLT_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR}
)

ADD_SUBDIRECTORY( caneda-tools )
ADD_SUBDIRECTORY( caneda )

ADD_SUBDIRECTORY( caneda-attenuator )
ADD_SUBDIRECTORY( caneda-filter )
ADD_SUBDIRECTORY( caneda-help )
ADD_SUBDIRECTORY( caneda-lib )
ADD_SUBDIRECTORY( caneda-transcalc )
ADD_SUBDIRECTORY( caneda-qterm )

# ==================================================================================
# Licence and other files

SET( MISC ChangeLog INSTALL PLATFORMS THANKS AUTHORS README TODO COPYING NEWS RELEASE )
INSTALL( FILES ${MISC} DESTINATION ${BASEDIR} )
