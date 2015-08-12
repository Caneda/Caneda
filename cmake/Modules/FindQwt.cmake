# ==================================================================================
# Caneda project
#
# FindQwt.cmake file to use in cmake to find Qwt libraries. Can be replaced once
# cmake upstream implements its own FindQwt.cmake
#
# The following values will be defined:
# QWT_FOUND - Qwt was found
# QWT_INCLUDE_DIR - Qwt include directories (qwt.h)
# QWT_LIBRARY - Qwt library directory
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

FILE(GLOB QWT_SEARCH_PATHS "/opt/qwt-*" "/usr/local/qwt-*")

FIND_PATH(QWT_INCLUDE_DIR
    NAMES qwt.h
    PATHS /usr/local/include/qwt /usr/include/qwt ${QWT_SEARCH_PATHS}/include
    )

FIND_LIBRARY(QWT_LIBRARY
    NAMES qwt libqwt
    PATHS /usr/local/lib /usr/lib ${QWT_SEARCH_PATHS}/lib
    )

SET(QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR})
SET(QWT_LIBRARIES ${QWT_LIBRARY})

# Handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE
# if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Qwt DEFAULT_MSG
                                  QWT_LIBRARY QWT_INCLUDE_DIR)

MARK_AS_ADVANCED(QWT_INCLUDE_DIR QWT_LIBRARY )
