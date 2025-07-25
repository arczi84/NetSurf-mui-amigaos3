/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2005-9 John-Mark Bell <jmb@netsurf-browser.org>
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
 * Form handling functions (implementation).
 */

#define _GNU_SOURCE  /* for strndup */
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "render/box.h"
#include "render/form.h"
#include "utils/log.h"
#include "utils/url.h"
#include "utils/utf8.h"
#include "utils/utils.h"


static char *form_textarea_value(struct form_control *textarea);
static char *form_acceptable_charset(struct form *form);
static char *form_encode_item(const char *item, const char *charset,
		const char *fallback);

/**
 * Create a struct form.
 *
 * \param  node    DOM node associated with form
 * \param  action  URL to submit form to, or NULL for default
 * \param  target  Target frame of form, or NULL for default
 * \param  method  method and enctype
 * \param  charset acceptable encodings for form submission, or NULL
 * \param  doc_charset  encoding of containing document, or NULL
 * \return  a new structure, or NULL on memory exhaustion
 */
struct form *form_new(void *node, const char *action, const char *target, 
		form_method method, const char *charset, 
		const char *doc_charset)
{
	struct form *form;

	form = calloc(1, sizeof *form);
	if (!form)
		return NULL;

	form->action = strdup(action != NULL ? action : "");
	if (form->action == NULL) {
		free(form);
		return NULL;
	}

	form->target = target != NULL ? strdup(target) : NULL;
	if (target != NULL && form->target == NULL) {
		free(form->action);
		free(form);
		return NULL;
	}

	form->method = method;

	form->accept_charsets = charset != NULL ? strdup(charset) : NULL;
	if (charset != NULL && form->accept_charsets == NULL) {
		free(form->target);
		free(form->action);
		free(form);
		return NULL;
	}

	form->document_charset = doc_charset != NULL ? strdup(doc_charset)
						     : NULL;
	if (doc_charset && form->document_charset == NULL) {
		free(form->accept_charsets);
		free(form->target);
		free(form->action);
		free(form);
		return NULL;
	}

	form->node = node;

	return form;
}

/**
 * Free a form, and any controls it owns.
 *
 * \param form  The form to free
 *
 * \note There may exist controls attached to box tree nodes which are not
 * associated with any form. These will leak at present. Ideally, they will
 * be cleaned up when the box tree is destroyed. As that currently happens
 * via talloc, this won't happen. These controls are distinguishable, as their
 * form field will be NULL.
 */
void form_free(struct form *form)
{
	struct form_control *c, *d;

	for (c = form->controls; c != NULL; c = d) {
		d = c->next;

		form_free_control(c);
	}

	free(form->action);
	free(form->target);
	free(form->accept_charsets);
	free(form->document_charset);

	free(form);
}

/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
struct form_control *form_new_control(void *node, form_control_type type)
{
	struct form_control *control;

	control = calloc(1, sizeof *control);
	if (control == NULL)
		return NULL;

	control->node = node;
	control->type = type;

	/* Default max length of input to something insane */
	control->maxlength = UINT_MAX;

	return control;
}


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
void form_add_control(struct form *form, struct form_control *control)
{
	control->form = form;

	if (form->controls != NULL) {
		assert(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = NULL;
		form->last_control = control;
	} else {
		form->controls = form->last_control = control;
	}
}


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
void form_free_control(struct form_control *control)
{
	free(control->name);
	free(control->value);
	free(control->initial_value);

	if (control->type == GADGET_SELECT) {
		struct form_option *option, *next;

		for (option = control->data.select.items; option;
				option = next) {
			next = option->next;
			free(option->text);
			free(option->value);
			free(option);
		}
	}

	free(control);
}


/**
 * Add an option to a form select control.
 *
 * \param  control   form control of type GADGET_SELECT
 * \param  value     value of option, used directly (not copied)
 * \param  text      text for option, used directly (not copied)
 * \param  selected  this option is selected
 * \return  true on success, false on memory exhaustion
 */
bool form_add_option(struct form_control *control, char *value, char *text,
		bool selected)
{
	struct form_option *option;

	assert(control);
	assert(control->type == GADGET_SELECT);

	option = calloc(1, sizeof *option);
	if (!option)
		return false;

	option->value = value;
	option->text = text;

	/* add to linked list */
	if (control->data.select.items == 0)
		control->data.select.items = option;
	else
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	if (selected && (control->data.select.num_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.num_selected++;
		control->data.select.current = option;
	}

	control->data.select.num_items++;

	return true;
}


/**
 * Identify 'successful' controls.
 *
 * All text strings in the successful controls list will be in the charset most
 * appropriate for submission. Therefore, no utf8_to_* processing should be
 * performed upon them.
 *
 * \todo The chosen charset needs to be made available such that it can be
 * included in the submission request (e.g. in the fetch's Content-Type header)
 *
 * \param  form           form to search for successful controls
 * \param  submit_button  control used to submit the form, if any
 * \param  successful_controls  updated to point to linked list of
 *                        form_successful_control, 0 if no controls
 * \return  true on success, false on memory exhaustion
 *
 * See HTML 4.01 section 17.13.2.
 */
bool form_successful_controls(struct form *form,
		struct form_control *submit_button,
		struct form_successful_control **successful_controls)
{
	struct form_control *control;
	struct form_option *option;
	struct form_successful_control sentinel, *last_success, *success_new;
	char *value = NULL;
	bool had_submit = false;
	char *charset;

	last_success = &sentinel;
	sentinel.next = 0;

	charset = form_acceptable_charset(form);
	if (!charset)
		return false;

#define ENCODE_ITEM(i) form_encode_item((i), charset, form->document_charset)

	for (control = form->controls; control; control = control->next) {
		/* ignore disabled controls */
		if (control->disabled)
			continue;

		/* ignore controls with no name */
		if (!control->name)
			continue;

		switch (control->type) {
			case GADGET_HIDDEN:
			case GADGET_TEXTBOX:
			case GADGET_PASSWORD:
				if (control->value)
					value = ENCODE_ITEM(control->value);
				else
					value = ENCODE_ITEM("");
				if (!value) {
					LOG(("failed to duplicate value"
						"'%s' for control %s",
							control->value,
							control->name));
					goto no_memory;
				}
				break;

			case GADGET_RADIO:
			case GADGET_CHECKBOX:
				/* ignore checkboxes and radio buttons which
				 * aren't selected */
				if (!control->selected)
					continue;
				if (control->value)
					value = ENCODE_ITEM(control->value);
				else
					value = ENCODE_ITEM("on");
				if (!value) {
					LOG(("failed to duplicate"
						"value '%s' for"
						"control %s",
						control->value,
						control->name));
					goto no_memory;
				}
				break;

			case GADGET_SELECT:
				/* select */
				for (option = control->data.select.items;
						option != NULL;
						option = option->next) {
					if (!option->selected)
						continue;
					success_new =
						malloc(sizeof(*success_new));
					if (!success_new) {
						LOG(("malloc failed"));
						goto no_memory;
					}
					success_new->file = false;
					success_new->name =
						ENCODE_ITEM(control->name);
					success_new->value =
						ENCODE_ITEM(option->value);
					success_new->next = NULL;
					last_success->next = success_new;
					last_success = success_new;
					if (!success_new->name ||
						!success_new->value) {
						LOG(("strdup failed"));
						goto no_memory;
					}
				}

				continue;
				break;

			case GADGET_TEXTAREA:
				{
				char *v2;

				/* textarea */
				value = form_textarea_value(control);
				if (!value) {
					LOG(("failed handling textarea"));
					goto no_memory;
				}
				if (value[0] == 0) {
					free(value);
					continue;
				}

				v2 = ENCODE_ITEM(value);
				if (!v2) {
					LOG(("failed handling textarea"));
					free(value);
					goto no_memory;
				}

				free(value);
				value = v2;
				}
				break;

			case GADGET_IMAGE: {
				/* image */
				size_t len;
				char *name;

				if (control != submit_button)
					/* only the activated submit button
					 * is successful */
					continue;

				name = ENCODE_ITEM(control->name);
				if (name == NULL)
					goto no_memory;

				len = strlen(name) + 3;

				/* x */
				success_new = malloc(sizeof(*success_new));
				if (!success_new) {
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				success_new->file = false;
				success_new->name = malloc(len);
				success_new->value = malloc(20);
				if (!success_new->name ||
						!success_new->value) {
					free(success_new->name);
					free(success_new->value);
					free(success_new);
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				sprintf(success_new->name, "%s.x", name);
				sprintf(success_new->value, "%i",
						control->data.image.mx);
				success_new->next = 0;
				last_success->next = success_new;
				last_success = success_new;

				/* y */
				success_new = malloc(sizeof(*success_new));
				if (!success_new) {
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				success_new->file = false;
				success_new->name = malloc(len);
				success_new->value = malloc(20);
				if (!success_new->name ||
						!success_new->value) {
					free(success_new->name);
					free(success_new->value);
					free(success_new);
					free(name);
					LOG(("malloc failed"));
					goto no_memory;
				}
				sprintf(success_new->name, "%s.y", name);
				sprintf(success_new->value, "%i",
						control->data.image.my);
				success_new->next = 0;
				last_success->next = success_new;
				last_success = success_new;

				free(name);

				continue;
				break;
			}

			case GADGET_SUBMIT:
				if (!submit_button && !had_submit)
					/* no submit button specified, so
					 * use first declared in form */
					had_submit = true;
				else if (control != submit_button)
					/* only the activated submit button
					 * is successful */
					continue;
				if (control->value)
					value = ENCODE_ITEM(control->value);
				else
					value = ENCODE_ITEM("");
				if (!value) {
					LOG(("failed to duplicate value"
						"'%s' for control %s",
							control->value,
							control->name));
					goto no_memory;
				}
				break;

			case GADGET_RESET:
				/* ignore reset */
				continue;
				break;

			case GADGET_FILE:
				/* file */
				/* Handling of blank file entries is
				 * implementation defined - we're perfectly
				 * within our rights to treat it as an
				 * unsuccessful control. Unfortunately, every
				 * other browser submits the field with
				 * a blank filename and no content. So,
				 * that's what we have to do, too.
				 */
				success_new = malloc(sizeof(*success_new));
				if (!success_new) {
					LOG(("malloc failed"));
					goto no_memory;
				}
				success_new->file = true;
				success_new->name = ENCODE_ITEM(control->name);
				success_new->value = 
						ENCODE_ITEM(control->value ?
						control->value : "");
				success_new->next = 0;
				last_success->next = success_new;
				last_success = success_new;
				if (!success_new->name ||
						!success_new->value) {
					LOG(("strdup failed"));
					goto no_memory;
				}

				continue;
				break;

			case GADGET_BUTTON:
				/* Ignore it */
				continue;
				break;

			default:
				assert(0);
				break;
		}

		success_new = malloc(sizeof(*success_new));
		if (!success_new) {
			LOG(("malloc failed"));
			goto no_memory;
		}
		success_new->file = false;
		success_new->name = ENCODE_ITEM(control->name);
		success_new->value = value;
		success_new->next = NULL;
		last_success->next = success_new;
		last_success = success_new;
		if (!success_new->name) {
			LOG(("failed to duplicate name '%s'",
					control->name));
			goto no_memory;
		}
	}

	*successful_controls = sentinel.next;
	return true;

no_memory:
	warn_user("NoMemory", 0);
	form_free_successful(sentinel.next);
	return false;

#undef ENCODE_ITEM
}


/**
 * Find the value for a textarea control.
 *
 * \param  textarea  control of type GADGET_TEXTAREA
 * \return  the value as a UTF-8 string on heap, or 0 on memory exhaustion
 */
char *form_textarea_value(struct form_control *textarea)
{
	unsigned int len = 0;
	char *value, *s;
	struct box *text_box;

	/* find required length */
	for (text_box = textarea->box->children->children; text_box;
			text_box = text_box->next) {
		if (text_box->type == BOX_TEXT)
			len += text_box->length + 1;
		else /* BOX_BR */
			len += 2;
	}

	/* construct value */
	s = value = malloc(len + 1);
	if (!s)
		return 0;
	for (text_box = textarea->box->children->children; text_box;
			text_box = text_box->next) {
		if (text_box->type == BOX_TEXT) {
			strncpy(s, text_box->text, text_box->length);
			s += text_box->length;
			if (text_box->next && text_box->next->type != BOX_BR)
				/* only add space if this isn't
				 * the last box on a line (or in the area) */
				*s++ = ' ';
		} else { /* BOX_BR */
			*s++ = '\r';
			*s++ = '\n';
		}
	}
	*s = 0;

	return value;
}


/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param  form  form to which successful controls relate
 * \param  control  linked list of form_successful_control
 * \return  URL-encoded form, or 0 on memory exhaustion
 */

char *form_url_encode(struct form *form,
		struct form_successful_control *control)
{
	char *name, *value;
	char *s = malloc(1), *s2;
	unsigned int len = 0, len1;
	url_func_result url_err;

	if (!s)
		return 0;
	s[0] = 0;

	for (; control; control = control->next) {
		url_err = url_escape(control->name, 0, true, NULL, &name);
		if (url_err == URL_FUNC_NOMEM) {
			free(s);
			return 0;
		}

		assert(url_err == URL_FUNC_OK);

		url_err = url_escape(control->value, 0, true, NULL, &value);
		if (url_err == URL_FUNC_NOMEM) {
			free(name);
			free(s);
			return 0;
		}

		assert(url_err == URL_FUNC_OK);

		len1 = len + strlen(name) + strlen(value) + 2;
		s2 = realloc(s, len1 + 1);
		if (!s2) {
			free(value);
			free(name);
			free(s);
			return 0;
		}
		s = s2;
		sprintf(s + len, "%s=%s&", name, value);
		len = len1;
		free(name);
		free(value);
	}

	if (len)
		s[len - 1] = 0;
	return s;
}


/**
 * Free a linked list of form_successful_control.
 *
 * \param control Pointer to head of list to free
 */

void form_free_successful(struct form_successful_control *control)
{
	struct form_successful_control *next;
	for (; control; control = next) {
		next = control->next;
		free(control->name);
		free(control->value);
		free(control);
	}
}

/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
char *form_acceptable_charset(struct form *form)
{
	char *temp, *c;

	if (!form)
		return NULL;

	if (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		if (form->document_charset)
			/* document charset present, so use it */
			return strdup(form->document_charset);
		else
			/* no document charset, so default to 8859-1 */
			return strdup("ISO-8859-1");
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	if (!temp)
		return NULL;

	/* make it upper case */
	for (c = temp; *c; c++)
		*c = toupper(*c);

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	if (c) {
		free(temp);
		return strdup("UTF-8");
	}

	/* dispense with temporary copy */
	free(temp);

	/* according to RFC2070, the accept-charsets attribute of the
	 * form element contains a space and/or comma separated list */
	c = form->accept_charsets;

	/* What would be an improvement would be to choose an encoding
	 * acceptable to the server which covers as much of the input
	 * values as possible. Additionally, we need to handle the case
	 * where none of the acceptable encodings cover all the textual
	 * input values.
	 * For now, we just extract the first element of the charset list
	 */
	while (*c && !isspace(*c)) {
		if (*c == ',')
			break;
		c++;
	}

	return strndup(form->accept_charsets, c - form->accept_charsets);
}

/**
 * Convert a string from UTF-8 to the specified charset
 * As a final fallback, this will attempt to convert to ISO-8859-1.
 *
 * \todo Return charset used?
 *
 * \param item String to convert
 * \param charset Destination charset
 * \param fallback Fallback charset (may be NULL),
 *                 used iff converting to charset fails
 * \return Pointer to converted string (on heap, caller frees), or NULL
 */
char *form_encode_item(const char *item, const char *charset,
		const char *fallback)
{
	utf8_convert_ret err;
	char *ret = NULL;
	char cset[256];

	if (!item || !charset)
		return NULL;

	snprintf(cset, sizeof cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	if (err == UTF8_CONVERT_BADENC) {
		/* charset not understood, try without transliteration */
		snprintf(cset, sizeof cset, "%s", charset);
		err = utf8_to_enc(item, cset, 0, &ret);

		if (err == UTF8_CONVERT_BADENC) {
			/* nope, try fallback charset (if any) */
			if (fallback) {
				snprintf(cset, sizeof cset, 
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				if (err == UTF8_CONVERT_BADENC) {
					/* and without transliteration */
					snprintf(cset, sizeof cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			if (err == UTF8_CONVERT_BADENC) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				if (err == UTF8_CONVERT_BADENC) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	if (err == UTF8_CONVERT_NOMEM) {
		return NULL;
	}

	return ret;
}

