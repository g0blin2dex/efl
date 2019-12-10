#ifndef EVAS_ECTOR_GL_H
#define EVAS_ECTOR_GL_H

#undef EAPI

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#include "evas_ector_buffer.eo.h"
#include "evas_ector_gl_buffer.eo.h"
#include "evas_ector_gl_image_buffer.eo.h"

void tizen_vd_ecache_init();
void tizen_vd_ecache_term();

#undef EAPI
#define EAPI

#endif /* ! EVAS_ECTOR_GL_H */

