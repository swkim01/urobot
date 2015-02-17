#include <string>
#include <cstdio>
#include <cerrno>
#include <ctemplate/template.h>
#include "urobot.h"
#include "device.h"
#include "services.h"
#include "presentation.h"
#include "buffer.h"

void build_template(struct urobot_t *ut, struct web_template *webtemp)
{
  int i;
  ctemplate::StringToTemplateCache(webtemp->name, webtemp->temp, ctemplate::DO_NOT_STRIP);
  ctemplate::TemplateDictionary dict(webtemp->name);
  char *str;
  std::string temp_str;
  struct template_dict *tdict;
  tdict = webtemp->tdict;

  if (tdict != NULL)
    for (i = 0; tdict[i].key != NULL; i++)
      dict.SetValue(tdict[i].key, tdict[i].value);

  ctemplate::ExpandTemplate(webtemp->name, ctemplate::DO_NOT_STRIP, &dict, &temp_str);
  buffer_append (ut->presentation, temp_str.c_str());
}

extern "C" void
build_css_template (struct urobot_t *ut)
{
  int i, j;

  for (i=0; i<ut->devcnt; i++) {
    struct device_t *dev;
    dev = ut->dev[i];
    for (j=0; j<dev->servicecount; j++) {
      struct web_content *web;
      web = dev->service[j]->web;
      if (web && web->css)
        build_template(ut, web->css);
    }
  }
}

extern "C" void
build_script_template (struct urobot_t *ut)
{
  int i, j;

  for (i=0; i<ut->devcnt; i++) {
    struct device_t *dev;
    dev = ut->dev[i];
    for (j=0; j<dev->servicecount; j++) {
      struct web_content *web;
      web = dev->service[j]->web;
      if (web && web->script)
        build_template(ut, web->script);
    }
  }
}

extern "C" void
build_onload_template (struct urobot_t *ut)
{
  int i, j;

  for (i=0; i<ut->devcnt; i++) {
    struct device_t *dev;
    dev = ut->dev[i];
    for (j=0; j<dev->servicecount; j++) {
      struct web_content *web;
      web = dev->service[j]->web;
      if (web && web->onload)
        build_template(ut, web->onload);
    }
  }
}

extern "C" void
build_body_template (struct urobot_t *ut)
{
  int i, j;

  for (i=0; i<ut->devcnt; i++) {
    struct device_t *dev;
    dev = ut->dev[i];
    for (j=0; j<dev->servicecount; j++) {
      struct web_content *web;
      web = dev->service[j]->web;
      if (web && web->body)
        build_template(ut, web->body);
    }
  }
}
