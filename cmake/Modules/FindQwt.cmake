# ==================================================================================
# Caneda project
#
# FindQwt.cmake file to use in cmake to find Qwt libraries. Can be replaced once
# cmake upstream implements its own FindQwt.cmake
#
# The module defines the following variables:
#  QWT_FOUND - Qwt was found
#  QWT_INCLUDE_DIR - Qwt include directories (qwt.h)
#  QWT_LIBRARY - Qwt library directory
#  QWT_MAJOR_VERSION - major version
#  QWT_MINOR_VERSION - minor version
#  QWT_PATCH_VERSION - patch version
#  QWT_VERSION_STRING - version (ex. 5.2.1)
#
# ==================================================================================
# Copyright (C) 2012-2015 by Pablo Daniel Pareja Obregon
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this package; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
# Boston, MA 02110-1301, USA.
# ==================================================================================

###########################################
# Find Qwt include and library directories
###########################################
FILE( GLOB QWT_SEARCH_PATHS "/opt/qwt-*" "/usr/local/qwt-*" "C:/Qwt-*" )

FIND_PATH( QWT_INCLUDE_DIR NAMES qwt.h PATHS
    /usr/include
    /usr/include/qwt
    /usr/local/include
    /usr/local/include/qwt
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
    ${QWT_SEARCH_PATHS}/include
)

FIND_LIBRARY( QWT_LIBRARY NAMES qwt libqwt qwt-qt5 libqwt-qt5 PATHS
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
    ${QWT_SEARCH_PATHS}/lib
)

SET( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )
SET( QWT_LIBRARIES ${QWT_LIBRARY} )

###########################################
# Find Qwt installed version
###########################################
SET( _VERSION_FILE ${QWT_INCLUDE_DIR}/qwt_global.h )
IF( EXISTS ${_VERSION_FILE} )
    FILE( STRINGS ${_VERSION_FILE} _VERSION_LINE REGEX "define[ ]+QWT_VERSION_STR" )
    IF( _VERSION_LINE )
        STRING( REGEX REPLACE ".*define[ ]+QWT_VERSION_STR[ ]+\"(.*)\".*" "\\1" QWT_VERSION_STRING "${_VERSION_LINE}" )
        STRING( REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1" QWT_MAJOR_VERSION "${QWT_VERSION_STRING}" )
        STRING( REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\2" QWT_MINOR_VERSION "${QWT_VERSION_STRING}" )
        STRING( REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\3" QWT_PATCH_VERSION "${QWT_VERSION_STRING}" )
    ENDIF()
ENDIF()

SET( _QWT_VERSION_MATCH TRUE )
IF( Qwt_FIND_VERSION AND QWT_VERSION_STRING )
    IF( Qwt_FIND_VERSION_EXACT )
        IF( NOT Qwt_FIND_VERSION VERSION_EQUAL QWT_VERSION_STRING )
            SET( _QWT_VERSION_MATCH FALSE )
        ENDIF()
    ELSE()
        IF( QWT_VERSION_STRING VERSION_LESS Qwt_FIND_VERSION )
            SET( _QWT_VERSION_MATCH FALSE )
        ENDIF()
    ENDIF()
ENDIF()

###########################################
# Set QWT_FOUND to TRUE and QWT_VERSION
###########################################
# Handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE
# if all listed variables are TRUE
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( Qwt DEFAULT_MSG
                                   QWT_LIBRARY QWT_INCLUDE_DIR _QWT_VERSION_MATCH
)

MARK_AS_ADVANCED(
    QWT_INCLUDE_DIR
    QWT_LIBRARY
    QWT_MAJOR_VERSION
    QWT_MINOR_VERSION
    QWT_PATCH_VERSION
    QWT_VERSION_STRING
)
