# ==================================================================================
# Caneda project
PROJECT( Caneda )

SET( PACKAGE_VERSION "0.4.0" )
SET( PACKAGE_STRING "caneda 0.4.0" )

# ==================================================================================
# Minimum libraries required

CMAKE_MINIMUM_REQUIRED( VERSION 3.16 )

SET( QT_MIN_VERSION 6.2.0 )
FIND_PACKAGE( Qt6Widgets ${QT_MIN_VERSION} REQUIRED )
FIND_PACKAGE( Qt6Svg ${QT_MIN_VERSION} REQUIRED )
FIND_PACKAGE( Qt6PrintSupport ${QT_MIN_VERSION} REQUIRED )

# Optional: only needed when (re)building translations. See the i18n subdirectory.
FIND_PACKAGE( Qt6LinguistTools ${QT_MIN_VERSION} )

# For Qwt
SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/" )
SET( QWT_MIN_VERSION 6.1.2 )
FIND_PACKAGE( Qwt ${QWT_MIN_VERSION} REQUIRED )

# ==================================================================================
# Configure runtime directories

SET( BASEDIR     "share/caneda/" )
SET( BINARYDIR   "bin/" )
SET( DESKTOPDIR  "share/applications/" )
SET( ICONDIR     "share/icons/" )
SET( IMAGEDIR    "share/caneda/images/" )
SET( MIMEDIR     "share/mime/packages/" )
SET( LANGUAGEDIR "share/caneda/i18n/" )
SET( LIBRARYDIR  "share/caneda/libraries/" )

# ==================================================================================
# Configure files and compilation options

SET( CMAKE_AUTOMOC ON )
SET( CMAKE_AUTOUIC ON )
SET( CMAKE_INCLUDE_CURRENT_DIR ON )
SET( CMAKE_BUILD_TYPE Debug )

CONFIGURE_FILE( ${Caneda_SOURCE_DIR}/config.h.cmake ${Caneda_BINARY_DIR}/config.h )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")

# ==================================================================================
# Include sources directories

ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( images )
#ADD_SUBDIRECTORY( i18n )
ADD_SUBDIRECTORY( libraries )

# ==================================================================================
# Licence and other files

SET( MISC README.md COPYING )
INSTALL( FILES ${MISC} DESTINATION ${BASEDIR} )

SET( DESKTOPFILES caneda.desktop )
INSTALL( FILES ${DESKTOPFILES} DESTINATION ${DESKTOPDIR} )

SET( MIMEFILES caneda.xml )
INSTALL( FILES ${MIMEFILES} DESTINATION ${MIMEDIR} )
