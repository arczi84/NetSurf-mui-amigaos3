/*
 * Copyright 2008 John Tytgat <joty@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * Export a content as a PDF file (implementation).
 */

#ifdef WITH_PDF_EXPORT

#include <stdbool.h>

#include "content/content.h"
#include "desktop/print.h"
#include "desktop/save_pdf/font_haru.h"
#include "desktop/save_pdf/pdf_plotters.h"
#include "mui/methodstack.h"
#include "mui/mui.h"
#include "mui/netsurf.h"
#include "mui/save_pdf.h"
#include "utils/log.h"
#include "utils/config.h"

/**
 * Export a content as a PDF file.
 *
 * \param  c     content to export
 * \param  path  path to save PDF as
 * \return  true on success, false on error and error reported
 */
bool save_as_pdf(struct content *c, const char *path)
{
	struct print_settings *psettings;
	bool retval;

	retval = false;
	
	psettings = print_make_settings(PRINT_DEFAULT, path, &haru_nsfont);

	if (psettings)
	{
		if (print_basic_run(c, &pdf_printer, psettings))
			retval = true;
	}

	return retval;
}

void PDF_Password(char **owner_pass, char **user_pass, char *path)
{
	*owner_pass = NULL;
	methodstack_push_sync(application, 2, MM_Application_AskPasswordPDF, owner_pass, user_pass);
}
#endif
