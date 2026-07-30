/*
Kuwagata For OBS Studio by bbadewa

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>

Hopefully, if you're opening the source code, this program is at least...
marginally useful to you!

And if not, well, I hope it's good enough to open a PR to fix.
*/

#include <obs-module.h>
#include <plugin-support.h>
#include"obs-frontend-api.h"
#include "Dock.h"
#include "Backend.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	BackendInit();
	obs_log(LOG_INFO, "Kuwagata4OBS version %s loaded successfully", PLUGIN_VERSION);
	obs_frontend_add_event_callback(DockCallback, nullptr);
	return true;
}

void obs_module_unload(void)
{

	obs_log(LOG_INFO, "plugin unloaded");
}
