/*
 * http.h : uRobot Web Server handler header.
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

#ifndef _HTTP_H_
#define _HTTP_H_

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#if UPNP_VERSION < 10613
struct UpnpVirtualDirCallbacks virtual_dir_callbacks;
#else
int http_init(void);
#endif /* UPNP_VERSION < 10613 */

#endif /* _HTTP_H_ */
