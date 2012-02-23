/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Stephen "okra" Houston <UnixTitan@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */
#include "eio_private.h"
#include "Eio.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static void
_eio_file_mkdir(void *data, Ecore_Thread *thread)
{
   Eio_File_Mkdir *m = data;

   if (mkdir(m->path, m->mode) != 0)
     eio_file_thread_error(&m->common, thread);
}

static void
_eio_mkdir_free(Eio_File_Mkdir *m)
{
   eina_stringshare_del(m->path);
   free(m);
}

static void
_eio_file_mkdir_done(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Mkdir *m = data;

   if (m->common.done_cb)
     m->common.done_cb((void*) m->common.data, &m->common);

   _eio_mkdir_free(m);
}

static void
_eio_file_mkdir_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Mkdir *m = data;

   eio_file_error(&m->common);
   _eio_mkdir_free(m);
}

static void
_eio_file_unlink(void *data, Ecore_Thread *thread)
{
   Eio_File_Unlink *l = data;

   if (unlink(l->path) != 0)
     eio_file_thread_error(&l->common, thread);
}

static void
_eio_unlink_free(Eio_File_Unlink *l)
{
   eina_stringshare_del(l->path);
   free(l);
}

static void
_eio_file_unlink_done(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Unlink *l = data;

   if (l->common.done_cb)
     l->common.done_cb((void*) l->common.data, &l->common);

   _eio_unlink_free(l);
}

static void
_eio_file_unlink_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Unlink *l = data;

   eio_file_error(&l->common);
   _eio_unlink_free(l);
}

static void
_eio_file_stat(void *data, Ecore_Thread *thread)
{
   Eio_File_Stat *s = data;
   struct stat buf;

   if (stat(s->path, &buf) != 0)
     eio_file_thread_error(&s->common, thread);

   s->buffer.dev = buf.st_dev;
   s->buffer.ino = buf.st_ino;
   s->buffer.mode = buf.st_mode;
   s->buffer.nlink = buf.st_nlink;
   s->buffer.uid = buf.st_uid;
   s->buffer.gid = buf.st_gid;
   s->buffer.rdev = buf.st_rdev;
   s->buffer.size = buf.st_size;
   s->buffer.blksize = buf.st_blksize;
   s->buffer.blocks = buf.st_blocks;
   s->buffer.atime = buf.st_atime;
   s->buffer.mtime = buf.st_mtime;
   s->buffer.ctime = buf.st_ctime;
#ifdef _STAT_VER_LINUX
# if (defined __USE_MISC && defined st_mtime)
   s->buffer.atimensec = buf.st_atim.tv_nsec;
   s->buffer.mtimensec = buf.st_mtim.tv_nsec;
   s->buffer.ctimensec = buf.st_ctim.tv_nsec;
# else
   s->buffer.atimensec = buf.st_atimensec;
   s->buffer.mtimensec = buf.st_mtimensec;
   s->buffer.ctimensec = buf.st_ctimensec;
# endif
#else
   s->buffer.atimensec = 0;
   s->buffer.mtimensec = 0;
   s->buffer.ctimensec = 0;
#endif

}

static void
_eio_stat_free(Eio_File_Stat *s)
{
   eina_stringshare_del(s->path);
   free(s);
}

static void
_eio_file_stat_done(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Stat *s = data;

   if (s->done_cb)
     s->done_cb((void*) s->common.data, &s->common, &s->buffer);

   _eio_stat_free(s);
}

static void
_eio_file_stat_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Stat *s = data;

   eio_file_error(&s->common);
   _eio_stat_free(s);
}

static void
_eio_file_chmod(void *data, Ecore_Thread *thread)
{
   Eio_File_Chmod *ch = data;

   if (chmod(ch->path, ch->mode) != 0)
     eio_file_thread_error(&ch->common, thread);
}

static void
_eio_file_chown(void *data, Ecore_Thread *thread)
{
   Eio_File_Chown *own = data;
   char *tmp;
   uid_t owner = -1;
   gid_t group = -1;

   own->common.error = 0;

   if (own->user)
     {
        owner = strtol(own->user, &tmp, 10);

        if (*tmp != '\0')
          {
             struct passwd *pw = NULL;

             own->common.error = EIO_FILE_GETPWNAM;

             pw = getpwnam(own->user);
             if (!pw) goto on_error;

             owner = pw->pw_uid;
          }
     }

   if (own->group)
     {
        group = strtol(own->group, &tmp, 10);

        if (*tmp != '\0')
          {
             struct group *grp = NULL;

             own->common.error = EIO_FILE_GETGRNAM;

             grp = getgrnam(own->group);
             if (!grp) goto on_error;

             group = grp->gr_gid;
          }
     }

   if (owner == (uid_t) -1 && group == (gid_t) -1)
     goto on_error;

   if (chown(own->path, owner, group) != 0)
     eio_file_thread_error(&own->common, thread);

   return ;

 on_error:
   ecore_thread_cancel(thread);
   return ;
}

static void
_eio_chown_free(Eio_File_Chown *ch)
{
   if (ch->user) eina_stringshare_del(ch->user);
   if (ch->group) eina_stringshare_del(ch->group);
   eina_stringshare_del(ch->path);
   free(ch);
}

static void
_eio_file_chown_done(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Chown *ch = data;

   if (ch->common.done_cb)
     ch->common.done_cb((void*) ch->common.data, &ch->common);

   _eio_chown_free(ch);
}

static void
_eio_file_chown_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Chown *ch = data;

   eio_file_error(&ch->common);
   _eio_chown_free(ch);
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

void
eio_file_error(Eio_File *common)
{
   if (common->error_cb)
     common->error_cb((void*) common->data, common, common->error);
}

void
eio_file_thread_error(Eio_File *common, Ecore_Thread *thread)
{
   common->error = errno;
   ecore_thread_cancel(thread);
}

Eina_Bool
eio_long_file_set(Eio_File *common,
		  Eio_Done_Cb done_cb,
		  Eio_Error_Cb error_cb,
		  const void *data,
		  Ecore_Thread_Cb heavy_cb,
		  Ecore_Thread_Notify_Cb notify_cb,
		  Ecore_Thread_Cb end_cb,
		  Ecore_Thread_Cb cancel_cb)
{
   Ecore_Thread *thread;

   common->done_cb = done_cb;
   common->error_cb = error_cb;
   common->data = data;
   common->error = 0;
   common->thread = NULL;
   common->container = NULL;
   common->worker.associated = NULL;
   common->main.associated = NULL;

   /* Be aware that ecore_thread_feedback_run could call cancel_cb if something goes wrong.
      This means that common would be destroyed if thread == NULL.
    */
   thread = ecore_thread_feedback_run(heavy_cb,
                                      notify_cb,
                                      end_cb,
                                      cancel_cb,
                                      common,
                                      EINA_TRUE);
   if (thread) common->thread = thread;
   return !!thread;
}

Eina_Bool
eio_file_set(Eio_File *common,
	     Eio_Done_Cb done_cb,
	     Eio_Error_Cb error_cb,
	     const void *data,
	     Ecore_Thread_Cb job_cb,
	     Ecore_Thread_Cb end_cb,
	     Ecore_Thread_Cb cancel_cb)
{
   Ecore_Thread *thread;

   common->done_cb = done_cb;
   common->error_cb = error_cb;
   common->data = data;
   common->error = 0;
   common->thread = NULL;
   common->container = NULL;

   /* Be aware that ecore_thread_run could call cancel_cb if something goes wrong.
      This means that common would be destroyed if thread == NULL.
   */
   thread = ecore_thread_run(job_cb, end_cb, cancel_cb, common);

   if (thread) common->thread = thread;
   return !!thread;
}

void
eio_file_container_set(Eio_File *common, void *container)
{
   common->container = container;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eio_File *
eio_file_direct_stat(const char *path,
		     Eio_Stat_Cb done_cb,
		     Eio_Error_Cb error_cb,
		     const void *data)
{
   Eio_File_Stat *s = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   s = malloc(sizeof (Eio_File_Stat));
   if (!s) return NULL;

   s->path = eina_stringshare_add(path);
   s->done_cb = done_cb;

   if (!eio_file_set(&s->common,
		      NULL,
		      error_cb,
		      data,
		      _eio_file_stat,
		      _eio_file_stat_done,
		      _eio_file_stat_error))
     return NULL;

   return &s->common;
}

EAPI Eio_File *
eio_file_unlink(const char *path,
		Eio_Done_Cb done_cb,
		Eio_Error_Cb error_cb,
		const void *data)
{
   Eio_File_Unlink *l = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   l = malloc(sizeof (Eio_File_Unlink));
   if (!l) return NULL;

   l->path = eina_stringshare_add(path);

   if (!eio_file_set(&l->common,
		      done_cb,
		      error_cb,
		      data,
		      _eio_file_unlink,
		      _eio_file_unlink_done,
		      _eio_file_unlink_error))
     return NULL;

   return &l->common;
}

EAPI Eio_File *
eio_file_mkdir(const char *path,
	       mode_t mode,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
   Eio_File_Mkdir *r = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   r = malloc(sizeof (Eio_File_Mkdir));
   if (!r) return NULL;

   r->path = eina_stringshare_add(path);
   r->mode = mode;

   if (!eio_file_set(&r->common,
		     done_cb,
		     error_cb,
		      data,
		     _eio_file_mkdir,
		     _eio_file_mkdir_done,
		     _eio_file_mkdir_error))
     return NULL;

   return &r->common;
}

EAPI Eio_File *
eio_file_chmod(const char *path,
	       mode_t mode,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
   Eio_File_Mkdir *r = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   r = malloc(sizeof (Eio_File_Mkdir));
   if (!r) return NULL;

   r->path = eina_stringshare_add(path);
   r->mode = mode;

   if (!eio_file_set(&r->common,
		     done_cb,
		     error_cb,
		      data,
		     _eio_file_chmod,
		     _eio_file_mkdir_done,
		     _eio_file_mkdir_error))
     return NULL;

   return &r->common;
}

EAPI Eio_File *
eio_file_chown(const char *path,
	       const char *user,
	       const char *group,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
   Eio_File_Chown *c = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   c = malloc(sizeof (Eio_File_Chown));
   if (!c) return NULL;

   c->path = eina_stringshare_add(path);
   c->user = eina_stringshare_add(user);
   c->group = eina_stringshare_add(group);

   if (!eio_file_set(&c->common,
		     done_cb,
		     error_cb,
		      data,
		     _eio_file_chown,
		     _eio_file_chown_done,
		     _eio_file_chown_error))
     return NULL;

   return &c->common;
}

