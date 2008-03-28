INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(XML_SRCS QRelaxNGvalidator.cpp  QXmlStreamReaderExt.cpp  
             QXsltTransformer.cpp validators.cpp xmlutilities.cpp)
SET(XML_MOC_HDRS QRelaxNGvalidator.h  QXmlStreamReaderExt.h  
                 QXsltTransformer.h validators.h)

#QT4_WRAP_CPP( XML_MOC_SRCS ${XML_MOC_HDRS} )

ADD_LIBRARY(xmlutilities ${XML_SRCS} ${XML_MOC_SRCS} )

