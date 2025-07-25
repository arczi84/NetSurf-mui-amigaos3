/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2003 James Bursa <bursa@users.sourceforge.net>
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
 * Form handling functions (interface).
 */

#ifndef _NETSURF_RENDER_FORM_H_
#define _NETSURF_RENDER_FORM_H_

#include <stdbool.h>
#include "utils/config.h"

struct box;
struct form_control;
struct form_option;

/** Form submit method. */
typedef enum {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
struct form {
	void *node;			/**< Corresponding DOM node */

	char *action;			/**< Absolute URL to submit to. */
	char *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	char *accept_charsets;		/**< Charset to submit form in */
	char *document_charset;		/**< Charset of document containing form */
	struct form_control *controls;	/**< Linked list of controls. */
	struct form_control *last_control;	/**< Last control in list. */

	struct form *prev;		/**< Previous form in doc. */
};

/** Type of a struct form_control. */
typedef enum {
	GADGET_HIDDEN,
	GADGET_TEXTBOX,
	GADGET_RADIO,
	GADGET_CHECKBOX,
	GADGET_SELECT,
	GADGET_TEXTAREA,
	GADGET_IMAGE,
	GADGET_PASSWORD,
	GADGET_SUBMIT,
	GADGET_RESET,
	GADGET_FILE,
	GADGET_BUTTON
} form_control_type;

/** Form control. */
struct form_control {
	void *node;			/**< Corresponding DOM node */

	form_control_type type;		/**< Type of control */

	struct form *form;		/**< Containing form */

	char *name;			/**< Control name */
	char *value;			/**< Current value of control */
	char *initial_value;		/**< Initial value of control */
	bool disabled;			/**< Whether control is disabled */

	struct box *box;		/**< Box for control */
	/** Caret details. */
	struct box *caret_inline_container;
	struct box *caret_text_box;
	size_t caret_box_offset, caret_form_offset;
	int caret_pixel_offset;

	unsigned int length;		/**< Number of characters in control */
	unsigned int maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	union {
		struct {
			int mx, my;
		} image;
		struct {
			int num_items;
			struct form_option *items, *last_item;
			bool multiple;
			int num_selected;
			/** Currently selected item, if num_selected == 1. */
			struct form_option *current;
		} select;
	} data;

	struct form_control *prev;      /**< Previous control in this form */
	struct form_control *next;	/**< Next control in this form. */
};

/** Option in a select. */
struct form_option {
	bool selected;
	bool initial_selected;
	char *value;
	char *text; /**< NUL terminated. */
	struct form_option* next;
};

/** Successful control, as defined by HTML 4.01 17.13. */
struct form_successful_control {
        bool file;                              /**< It's a file */
	char *name;				/**< Control name. */
	char *value;				/**< Current value. */
	struct form_successful_control *next;	/**< Next in linked list. */
};

struct form *form_new(void *node, const char *action, const char *target, 
		form_method method, const char *charset, 
		const char *doc_charset);
void form_free(struct form *form);
struct form_control *form_new_control(void *node, form_control_type type);
void form_add_control(struct form *form, struct form_control *control);
void form_free_control(struct form_control *control);
bool form_add_option(struct form_control *control, char *value, char *text,
		bool selected);
bool form_successful_controls(struct form *form,
		struct form_control *submit_button,
		struct form_successful_control **successful_controls);
char *form_url_encode(struct form *form,
		struct form_successful_control *control);
void form_free_successful(struct form_successful_control *control);

#endif
