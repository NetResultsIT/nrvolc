#ifndef NRVOLC_LIB_GLOBAL_H
#define NRVOLC_LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NRVOLC_LIB_LIBRARY)
#  define NRVOLC_LIB_EXPORT Q_DECL_EXPORT
#else
#  define NRVOLC_LIB_EXPORT Q_DECL_IMPORT
#endif

#endif // LIB_GLOBAL_H
