/*
 * presentation.h : uRobot UPnP Presentation Page headers.
 * Originally developped for the uRobot project.
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

#ifndef _PRESENTATION_H_
#define _PRESENTATION_H_

#define UROBOT_PRESENTATION_PAGE "/web/urobot.html"
#define PRESENTATION_PAGE_CONTENT_TYPE "text/html"
#define UROBOT_CGI "/web/urobot.cgi"
#define DEFAULT_PAGE "/index.html"

#include "urobot.h"

struct template_dict {
    char *key;
    char *value;
};

struct web_template {
    char *name;
    char *temp;
    struct template_dict *tdict;
};

struct web_content {
    struct web_template *css;
    struct web_template *script;
    struct web_template *onload;
    struct web_template *body;
};

#ifdef __cplusplus
extern "C" {
#endif
int process_cgi (struct urobot_t *ut, char *cgiargs);
int build_presentation_page (struct urobot_t *ut);
void build_css_template (struct urobot_t *ut);
void build_script_template (struct urobot_t *ut);
void build_onload_template (struct urobot_t *ut);
void build_body_template (struct urobot_t *ut);
#ifdef __cplusplus
}
#endif

#endif /* _PRESENTATION_H_ */
