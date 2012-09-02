# ==================================================================================
# Caneda project
PROJECT( Caneda )

SET( PACKAGE_VERSION "0.1.0" )
SET( PACKAGE_STRING "caneda 0.1.0" )

# ==================================================================================
# Configure QT options

SET( QT_USE_QTSVG TRUE )

# ==================================================================================
# Minimum libraries required

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

SET( QT_MIN_VERSION "4.7.0" )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )

# For Qwt
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
FIND_PACKAGE( Qwt REQUIRED )

# ==================================================================================
# Configure runtime directories

SET( BASEDIR     "share/caneda/" )
SET( BITMAPDIR   "share/caneda/images/" )
SET( BINARYDIR   "bin/" )
SET( DOCDIR      "share/caneda/docs/" )
SET( MANDIR      "share/man/man1/" )
SET( LANGUAGEDIR "share/caneda/i18n/" )
SET( LIBRARYDIR  "share/caneda/libraries/" )

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
    ${CMAKE_SOURCE_DIR}/src
    ${QWT_INCLUDE_DIR}
)

ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( docs )
ADD_SUBDIRECTORY( images )
ADD_SUBDIRECTORY( i18n )
ADD_SUBDIRECTORY( libraries )

# ==================================================================================
# Licence and other files

SET( MISC ChangeLog INSTALL AUTHORS COPYING RELEASE )
INSTALL( FILES ${MISC} DESTINATION ${BASEDIR} )
