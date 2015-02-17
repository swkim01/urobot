/*
 * http.c : uRobot Web Server handler.
 * Originally developped for the uRobot project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
 * Copyright (C) 2011-2015 Seong-Woo Kim <swkim01@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "urobot.h"
#include "device.h"
#include "services.h"
#include "cms.h"
#include "mcs.h"
#include "sms.h"
#include "cam.h"
#include "sts.h"
#include "http.h"
#include "utils.h"
#include "trace.h"
#include "presentation.h"
#include "joystick.h"
#include "osdep.h"

#define PROTOCOL_TYPE_PRE_SZ  11   /* for the str length of "http-get:*:" */
#define PROTOCOL_TYPE_SUFF_SZ 2    /* for the str length of ":*" */

struct web_file_t {
  char *fullpath;
  off_t pos;
  enum {
    FILE_LOCAL,
    FILE_MEMORY
  } type;
  union {
    struct {
      int fd;
      struct upnp_entry_t *entry;
    } local;
    struct {
      char *contents;
      off_t len;
    } memory;
  } detail;
};


static inline void
set_info_file (struct File_Info *info, const size_t length,
               const char *content_type)
{
  info->file_length = length;
  info->last_modified = 0;
  info->is_directory = 0;
  info->is_readable = 1;
  info->content_type = ixmlCloneDOMString (content_type);
}

static int
http_get_info (const char *filename, struct File_Info *info)
{
  extern struct urobot_t *ut;
  
  printf ("http_get_info, filename : %s\n", filename);
  if (!filename || !info)
    return -1;

  log_verbose ("http_get_info, filename : %s\n", filename);
  if (!strcmp (filename, CMS_LOCATION))
  {
    set_info_file (info, CMS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
    return 0;
  }

  if (!strcmp (filename, MCS_LOCATION))
  {
    set_info_file (info, MCS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
    return 0;
  }

  if (!strcmp (filename, SMS_LOCATION))
  {
    set_info_file (info, SMS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
    return 0;
  }

  if (!strcmp (filename, CAM_LOCATION))
  {
    set_info_file (info, CAM_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
    return 0;
  }

  if (!strcmp (filename, STS_LOCATION))
  {
    set_info_file (info, STS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
    return 0;
  }

  if (ut->use_presentation && !strcmp (filename, JOYSTICK_LOCATION))
  {
    set_info_file (info, JOYSTICK_DESCRIPTION_LEN, JOYSTICK_PAGE_CONTENT_TYPE);
    return 0;
  }

  if (ut->use_presentation && !strcmp (filename, UROBOT_PRESENTATION_PAGE))
  {
    if (build_presentation_page (ut) < 0)
      return -1;

    set_info_file (info, ut->presentation->len, PRESENTATION_PAGE_CONTENT_TYPE);
    return 0;
  }

  if (ut->use_presentation && !strncmp (filename, UROBOT_CGI, strlen (UROBOT_CGI)))
  {
    pthread_mutex_lock (&ut->web_mutex);
    int result = process_cgi (ut, (char *) (filename + strlen (UROBOT_CGI) + 1));
    if (result < 0) {
      pthread_mutex_unlock (&ut->web_mutex);
      return -1;
    }

    if (result == 1)
      set_info_file (info, ut->presentation->len, SERVICE_CONTENT_TYPE);
    else
      set_info_file (info, ut->presentation->len, PRESENTATION_PAGE_CONTENT_TYPE);
    pthread_mutex_unlock (&ut->web_mutex);
    return 0;
  }

  return 0;
}

static UpnpWebFileHandle
get_file_memory (const char *fullpath, const char *description,
                 const size_t length)
{
  struct web_file_t *file;

  file = malloc (sizeof (struct web_file_t));
  file->fullpath = strdup (fullpath);
  file->pos = 0;
  file->type = FILE_MEMORY;
  file->detail.memory.contents = strdup (description);
  file->detail.memory.len = length;

  return ((UpnpWebFileHandle) file);
}

static UpnpWebFileHandle
http_open (const char *filename, enum UpnpOpenFileMode mode)
{
  extern struct urobot_t *ut;

  printf ("http_open, filename : %s\n", filename);
  if (!filename)
    return NULL;

  log_verbose ("http_open, filename : %s\n", filename);

  if (mode != UPNP_READ)
    return NULL;

  if (!strcmp (filename, CMS_LOCATION))
    return get_file_memory (CMS_LOCATION, CMS_DESCRIPTION, CMS_DESCRIPTION_LEN);

  if (!strcmp (filename, MCS_LOCATION))
    return get_file_memory (MCS_LOCATION, MCS_DESCRIPTION, MCS_DESCRIPTION_LEN);

  if (!strcmp (filename, SMS_LOCATION))
    return get_file_memory (SMS_LOCATION, SMS_DESCRIPTION, SMS_DESCRIPTION_LEN);

  if (!strcmp (filename, CAM_LOCATION))
    return get_file_memory (CAM_LOCATION, CAM_DESCRIPTION, CAM_DESCRIPTION_LEN);

  if (!strcmp (filename, STS_LOCATION))
    return get_file_memory (STS_LOCATION, STS_DESCRIPTION, STS_DESCRIPTION_LEN);

  if (ut->use_presentation && !strcmp (filename, JOYSTICK_LOCATION))
    return get_file_memory (JOYSTICK_LOCATION, JOYSTICK_DESCRIPTION, JOYSTICK_DESCRIPTION_LEN);

  if (ut->use_presentation && ( !strcmp (filename, UROBOT_PRESENTATION_PAGE)
      || !strncmp (filename, UROBOT_CGI, strlen (UROBOT_CGI)))) {
    UpnpWebFileHandle file;

    pthread_mutex_lock (&ut->web_mutex);
    file = get_file_memory (UROBOT_PRESENTATION_PAGE, ut->presentation->buf,
                            ut->presentation->len);
    pthread_mutex_unlock (&ut->web_mutex);
    return file;
  }

  return ((UpnpWebFileHandle) 0);
}

static int
http_read (UpnpWebFileHandle fh, char *buf, size_t buflen)
{
  struct web_file_t *file = (struct web_file_t *) fh;
  ssize_t len = -1;

  printf ("http_read\n");
  if (!file)
    return -1;

  log_verbose ("http_read\n");
  switch (file->type)
  {
  case FILE_LOCAL:
    log_verbose ("Read local file.\n");
    len = read (file->detail.local.fd, buf, buflen);
    break;
  case FILE_MEMORY:
    log_verbose ("Read file from memory.\n");
    len = (size_t) MIN (buflen, file->detail.memory.len - file->pos);
    memcpy (buf, file->detail.memory.contents + file->pos, (size_t) len);
    break;
  default:
    log_verbose ("Unknown file type.\n");
    break;
  }

  if (len >= 0)
    file->pos += len;

  log_verbose ("Read %zd bytes.\n", len);

  return len;
}

static int
http_write (UpnpWebFileHandle fh __attribute__((unused)),
            char *buf __attribute__((unused)),
            size_t buflen __attribute__((unused)))
{
  log_verbose ("http write\n");

  return 0;
}

static int
http_seek (UpnpWebFileHandle fh, off_t offset, int origin)
{
  struct web_file_t *file = (struct web_file_t *) fh;
  off_t newpos = -1;

  log_verbose ("http_seek\n");

  if (!file)
    return -1;

  switch (origin)
  {
  case SEEK_SET:
    log_verbose ("Attempting to seek to %lld (was at %lld) in %s\n",
                offset, file->pos, file->fullpath);
    newpos = offset;
    break;
  case SEEK_CUR:
    log_verbose ("Attempting to seek by %lld from %lld in %s\n",
                offset, file->pos, file->fullpath);
    newpos = file->pos + offset;
    break;
  case SEEK_END:
    log_verbose ("Attempting to seek by %lld from end (was at %lld) in %s\n",
                offset, file->pos, file->fullpath);

    if (file->type == FILE_LOCAL)
    {
      struct stat sb;
      if (stat (file->fullpath, &sb) < 0)
      {
        log_verbose ("%s: cannot stat: %s\n",
                    file->fullpath, strerror (errno));
        return -1;
      }
      newpos = sb.st_size + offset;
    }
    else if (file->type == FILE_MEMORY)
      newpos = file->detail.memory.len + offset;
    break;
  }

  switch (file->type)
  {
  case FILE_LOCAL:
    /* Just make sure we cannot seek before start of file. */
    if (newpos < 0)
    {
      log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (EINVAL));
      return -1;
    }

    /* Don't seek with origin as specified above, as file may have
       changed in size since our last stat. */
    if (lseek (file->detail.local.fd, newpos, SEEK_SET) == -1)
    {
      log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (errno));
      return -1;
    }
    break;
  case FILE_MEMORY:
    if (newpos < 0 || newpos > file->detail.memory.len)
    {
      log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (EINVAL));
      return -1;
    }
    break;
  }

  file->pos = newpos;

  return 0;
}

static int
http_close (UpnpWebFileHandle fh)
{
  struct web_file_t *file = (struct web_file_t *) fh;

  log_verbose ("http_close\n");

  if (!file)
    return -1;

  switch (file->type)
  {
  case FILE_LOCAL:
    close (file->detail.local.fd);
    break;
  case FILE_MEMORY:
    /* no close operation */
    if (file->detail.memory.contents)
      free (file->detail.memory.contents);
    break;
  default:
    log_verbose ("Unknown file type.\n");
    break;
  }

  if (file->fullpath)
    free (file->fullpath);
  free (file);

  return 0;
}

#if UPNP_VERSION < 10613
struct UpnpVirtualDirCallbacks virtual_dir_callbacks =
  {
    http_get_info,
    http_open,
    http_read,
    http_write,
    http_seek,
    http_close
  };
#else
int http_init(void)
{
    int res;
    res = UpnpVirtualDir_set_GetInfoCallback(http_get_info);
    if (res != UPNP_E_SUCCESS) return res;
    res = UpnpVirtualDir_set_OpenCallback(http_open);
    if (res != UPNP_E_SUCCESS) return res;
    res = UpnpVirtualDir_set_ReadCallback(http_read);
    if (res != UPNP_E_SUCCESS) return res;
    res = UpnpVirtualDir_set_WriteCallback(http_write);
    if (res != UPNP_E_SUCCESS) return res;
    res = UpnpVirtualDir_set_SeekCallback(http_seek);
    if (res != UPNP_E_SUCCESS) return res;
    res = UpnpVirtualDir_set_CloseCallback(http_close);
    return res;
}
#endif /* UPNP_VERSION < 10613 */
