/*
 * This file is part of Hubbub.
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2007-8 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <assert.h>
#include <string.h>

#include <parserutils/charset/mibenum.h>
#include <parserutils/input/inputstream.h>

#include <hubbub/parser.h>

#include "charset/detect.h"
#include "tokeniser/tokeniser.h"
#include "treebuilder/treebuilder.h"
#include "utils/parserutilserror.h"

/**
 * Hubbub parser object
 */
struct hubbub_parser {
	parserutils_inputstream *stream;	/**< Input stream instance */
	hubbub_tokeniser *tok;		/**< Tokeniser instance */
	hubbub_treebuilder *tb;		/**< Treebuilder instance */

	hubbub_allocator_fn alloc;	/**< Memory (de)allocation function */
	void *pw;			/**< Client data */
};

/**
 * Create a hubbub parser
 *
 * \param enc      Source document encoding, or NULL to autodetect
 * \param fix_enc  Permit fixing up of encoding if it's frequently misused
 * \param alloc    Memory (de)allocation function
 * \param pw       Pointer to client-specific private data (may be NULL)
 * \param parser   Pointer to location to receive parser instance
 * \return HUBBUB_OK on success,
 *         HUBBUB_BADPARM on bad parameters,
 *         HUBBUB_NOMEM on memory exhaustion,
 *         HUBBUB_BADENCODING if ::enc is unsupported
 */
hubbub_error hubbub_parser_create(const char *enc, bool fix_enc,
		hubbub_allocator_fn alloc, void *pw, hubbub_parser **parser)
{
	parserutils_error perror;
	hubbub_error error;
	hubbub_parser *p;

	if (alloc == NULL || parser == NULL)
		return HUBBUB_BADPARM;

	p = alloc(NULL, sizeof(hubbub_parser), pw);
	if (p == NULL)
		return HUBBUB_NOMEM;

	/* If we have an encoding and we're permitted to fix up likely broken
	 * ones, then attempt to do so. */
	if (enc != NULL && fix_enc == true) {
		uint16_t mibenum = parserutils_charset_mibenum_from_name(enc, 
				strlen(enc));

		if (mibenum != 0) {
			hubbub_charset_fix_charset(&mibenum);

			enc = parserutils_charset_mibenum_to_name(mibenum);
		}
	}

	perror = parserutils_inputstream_create(enc,
		enc != NULL ? HUBBUB_CHARSET_CONFIDENT : HUBBUB_CHARSET_UNKNOWN,
		hubbub_charset_extract, alloc, pw, &p->stream);
	if (perror != PARSERUTILS_OK) {
		alloc(p, 0, pw);
		return hubbub_error_from_parserutils_error(perror);
	}

	error = hubbub_tokeniser_create(p->stream, alloc, pw, &p->tok);
	if (error != HUBBUB_OK) {
		parserutils_inputstream_destroy(p->stream);
		alloc(p, 0, pw);
		return error;
	}

	error = hubbub_treebuilder_create(p->tok, alloc, pw, &p->tb);
	if (error != HUBBUB_OK) {
		hubbub_tokeniser_destroy(p->tok);
		parserutils_inputstream_destroy(p->stream);
		alloc(p, 0, pw);
		return error;
	}

	p->alloc = alloc;
	p->pw = pw;

	*parser = p;

	return HUBBUB_OK;
}

/**
 * Destroy a hubbub parser
 *
 * \param parser  Parser instance to destroy
 * \return HUBBUB_OK on success, appropriate error otherwise
 */
hubbub_error hubbub_parser_destroy(hubbub_parser *parser)
{
	if (parser == NULL)
		return HUBBUB_BADPARM;

	hubbub_treebuilder_destroy(parser->tb);

	hubbub_tokeniser_destroy(parser->tok);

	parserutils_inputstream_destroy(parser->stream);

	parser->alloc(parser, 0, parser->pw);

	return HUBBUB_OK;
}

/**
 * Configure a hubbub parser
 *
 * \param parser  Parser instance to configure
 * \param type    Option to set
 * \param params  Option-specific parameters
 * \return HUBBUB_OK on success, appropriate error otherwise
 */
hubbub_error hubbub_parser_setopt(hubbub_parser *parser,
		hubbub_parser_opttype type,
		hubbub_parser_optparams *params)
{
	hubbub_error result = HUBBUB_OK;

	if (parser == NULL || params == NULL)
		return HUBBUB_BADPARM;

	switch (type) {
	case HUBBUB_PARSER_TOKEN_HANDLER:
		if (parser->tb != NULL) {
			/* Client is defining their own token handler, 
			 * so we must destroy the default treebuilder */
			hubbub_treebuilder_destroy(parser->tb);
			parser->tb = NULL;
		}
		result = hubbub_tokeniser_setopt(parser->tok,
				HUBBUB_TOKENISER_TOKEN_HANDLER,
				(hubbub_tokeniser_optparams *) params);
		break;
	case HUBBUB_PARSER_ERROR_HANDLER:
		/* The error handler does not cascade, so tell both the
		 * treebuilder (if extant) and the tokeniser. */
		if (parser->tb != NULL) {
			result = hubbub_treebuilder_setopt(parser->tb,
					HUBBUB_TREEBUILDER_ERROR_HANDLER,
					(hubbub_treebuilder_optparams *) params);
		}
		if (result == HUBBUB_OK) {
			result = hubbub_tokeniser_setopt(parser->tok,
					HUBBUB_TOKENISER_ERROR_HANDLER,
					(hubbub_tokeniser_optparams *) params);
		}
		break;
	case HUBBUB_PARSER_CONTENT_MODEL:
		result = hubbub_tokeniser_setopt(parser->tok,
				HUBBUB_TOKENISER_CONTENT_MODEL,
				(hubbub_tokeniser_optparams *) params);
		break;
	case HUBBUB_PARSER_TREE_HANDLER:
		if (parser->tb != NULL) {
			result = hubbub_treebuilder_setopt(parser->tb,
					HUBBUB_TREEBUILDER_TREE_HANDLER,
					(hubbub_treebuilder_optparams *) params);
		}
		break;
	case HUBBUB_PARSER_DOCUMENT_NODE:
		if (parser->tb != NULL) {
			result = hubbub_treebuilder_setopt(parser->tb,
					HUBBUB_TREEBUILDER_DOCUMENT_NODE,
					(hubbub_treebuilder_optparams *) params);
		}
		break;
	case HUBBUB_PARSER_ENABLE_SCRIPTING:
		if (parser->tb != NULL) {
			result = hubbub_treebuilder_setopt(parser->tb,
					HUBBUB_TREEBUILDER_ENABLE_SCRIPTING,
					(hubbub_treebuilder_optparams *) params);
		}
		break;
	default:
		result = HUBBUB_INVALID;
	}

	return result;
}

/**
 * Pass a chunk of data to a hubbub parser for parsing
 *
 * \param parser  Parser instance to use
 * \param data    Data to parse (encoded in the input charset)
 * \param len     Length, in bytes, of data
 * \return HUBBUB_OK on success, appropriate error otherwise
 */
hubbub_error hubbub_parser_parse_chunk(hubbub_parser *parser,
		const uint8_t *data, size_t len)
{
	parserutils_error perror;
	hubbub_error error;

	if (parser == NULL || data == NULL)
		return HUBBUB_BADPARM;

	perror = parserutils_inputstream_append(parser->stream, data, len);
	if (perror != PARSERUTILS_OK)
		return hubbub_error_from_parserutils_error(perror);

	error = hubbub_tokeniser_run(parser->tok);
	if (error == HUBBUB_BADENCODING) {
		/* Ok, we autodetected an encoding that we don't actually
		 * support. We've not actually processed any data at this
		 * point so fall back to Windows-1252 and hope for the best
		 */
		perror = parserutils_inputstream_change_charset(parser->stream,
				"Windows-1252", HUBBUB_CHARSET_TENTATIVE);
		/* Under no circumstances should we get here if we've managed
		 * to process data. If there is a way, I want to know about it
		 */
		assert(perror != PARSERUTILS_INVALID);
		if (perror != PARSERUTILS_OK)
			return hubbub_error_from_parserutils_error(perror);

		/* Retry the tokenisation */
		error = hubbub_tokeniser_run(parser->tok);
	}

	if (error != HUBBUB_OK)
		return error;

	return HUBBUB_OK;
}

#if 0
/**
 * Pass a chunk of extraneous data to a hubbub parser for parsing
 *
 * \param parser  Parser instance to use
 * \param data    Data to parse (encoded in UTF-8)
 * \param len     Length, in byte, of data
 * \return HUBBUB_OK on success, appropriate error otherwise
 */
hubbub_error hubbub_parser_parse_extraneous_chunk(hubbub_parser *parser,
		const uint8_t *data, size_t len)
{
	hubbub_error error;

	/** \todo In some cases, we don't actually want script-inserted
	 * data to be parsed until later. We'll need some way of flagging
	 * this through the public API, and the inputstream API will need
	 * some way of marking the insertion point so that, when the
	 * tokeniser is run, only the inserted chunk is parsed. */

	if (parser == NULL || data == NULL)
		return HUBBUB_BADPARM;

	error = parserutils_inputstream_insert(parser->stream, data, len);
	if (error != HUBBUB_OK)
		return error;

	error = hubbub_tokeniser_run(parser->tok);
	if (error != HUBBUB_OK)
		return error;

	return HUBBUB_OK;
}
#endif

/**
 * Inform the parser that the last chunk of data has been parsed
 *
 * \param parser  Parser to inform
 * \return HUBBUB_OK on success, appropriate error otherwise
 */
hubbub_error hubbub_parser_completed(hubbub_parser *parser)
{
	parserutils_error perror;
	hubbub_error error;

	if (parser == NULL)
		return HUBBUB_BADPARM;

	perror = parserutils_inputstream_append(parser->stream, NULL, 0);
	if (perror != HUBBUB_OK)
		return !HUBBUB_OK;

	error = hubbub_tokeniser_run(parser->tok);
	if (error != HUBBUB_OK)
		return error;

	return HUBBUB_OK;
}

/**
 * Read the document charset
 *
 * \param parser  Parser instance to query
 * \param source  Pointer to location to receive charset source
 * \return Pointer to charset name (constant; do not free), or NULL if unknown
 */
const char *hubbub_parser_read_charset(hubbub_parser *parser,
		hubbub_charset_source *source)
{
	if (parser == NULL || source == NULL)
		return NULL;

	return parserutils_inputstream_read_charset(parser->stream, source);
}

