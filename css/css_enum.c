#include <strings.h>
#include "css/css_enum.h"

/**
 * css_unit
 */

const char * const css_unit_name[] = {
	"em",
	"ex",
	"px",
	"in",
	"cm",
	"mm",
	"pt",
	"pc"
};

css_unit css_unit_parse(const char * const s, int length)
{
	if (length == 2 && strncasecmp(s, "em", 2) == 0) return CSS_UNIT_EM;
	if (length == 2 && strncasecmp(s, "ex", 2) == 0) return CSS_UNIT_EX;
	if (length == 2 && strncasecmp(s, "px", 2) == 0) return CSS_UNIT_PX;
	if (length == 2 && strncasecmp(s, "in", 2) == 0) return CSS_UNIT_IN;
	if (length == 2 && strncasecmp(s, "cm", 2) == 0) return CSS_UNIT_CM;
	if (length == 2 && strncasecmp(s, "mm", 2) == 0) return CSS_UNIT_MM;
	if (length == 2 && strncasecmp(s, "pt", 2) == 0) return CSS_UNIT_PT;
	if (length == 2 && strncasecmp(s, "pc", 2) == 0) return CSS_UNIT_PC;
	return CSS_UNIT_UNKNOWN;
}

/**
 * css_background_attachment
 */

const char * const css_background_attachment_name[] = {
	"inherit",
	"fixed",
	"scroll"
};

css_background_attachment css_background_attachment_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_BACKGROUND_ATTACHMENT_INHERIT;
	if (length == 5 && strncasecmp(s, "fixed", 5) == 0) return CSS_BACKGROUND_ATTACHMENT_FIXED;
	if (length == 6 && strncasecmp(s, "scroll", 6) == 0) return CSS_BACKGROUND_ATTACHMENT_SCROLL;
	return CSS_BACKGROUND_ATTACHMENT_UNKNOWN;
}

/**
 * css_background_repeat
 */

const char * const css_background_repeat_name[] = {
	"inherit",
	"repeat",
	"repeat-x",
	"repeat-y",
	"no-repeat"
};

css_background_repeat css_background_repeat_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_BACKGROUND_REPEAT_INHERIT;
	if (length == 6 && strncasecmp(s, "repeat", 6) == 0) return CSS_BACKGROUND_REPEAT_REPEAT;
	if (length == 8 && strncasecmp(s, "repeat-x", 8) == 0) return CSS_BACKGROUND_REPEAT_REPEAT_X;
	if (length == 8 && strncasecmp(s, "repeat-y", 8) == 0) return CSS_BACKGROUND_REPEAT_REPEAT_Y;
	if (length == 9 && strncasecmp(s, "no-repeat", 9) == 0) return CSS_BACKGROUND_REPEAT_NO_REPEAT;
	return CSS_BACKGROUND_REPEAT_UNKNOWN;
}

/**
 * css_border_collapse
 */

const char * const css_border_collapse_name[] = {
	"inherit",
	"collapse",
	"separate"
};

css_border_collapse css_border_collapse_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_BORDER_COLLAPSE_INHERIT;
	if (length == 8 && strncasecmp(s, "collapse", 8) == 0) return CSS_BORDER_COLLAPSE_COLLAPSE;
	if (length == 8 && strncasecmp(s, "separate", 8) == 0) return CSS_BORDER_COLLAPSE_SEPARATE;
	return CSS_BORDER_COLLAPSE_UNKNOWN;
}

/**
 * css_border_style
 */

const char * const css_border_style_name[] = {
	"inherit",
	"none",
	"hidden",
	"dotted",
	"dashed",
	"solid",
	"double",
	"groove",
	"ridge",
	"inset",
	"outset"
};

css_border_style css_border_style_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_BORDER_STYLE_INHERIT;
	if (length == 4 && strncasecmp(s, "none", 4) == 0) return CSS_BORDER_STYLE_NONE;
	if (length == 6 && strncasecmp(s, "hidden", 6) == 0) return CSS_BORDER_STYLE_HIDDEN;
	if (length == 6 && strncasecmp(s, "dotted", 6) == 0) return CSS_BORDER_STYLE_DOTTED;
	if (length == 6 && strncasecmp(s, "dashed", 6) == 0) return CSS_BORDER_STYLE_DASHED;
	if (length == 5 && strncasecmp(s, "solid", 5) == 0) return CSS_BORDER_STYLE_SOLID;
	if (length == 6 && strncasecmp(s, "double", 6) == 0) return CSS_BORDER_STYLE_DOUBLE;
	if (length == 6 && strncasecmp(s, "groove", 6) == 0) return CSS_BORDER_STYLE_GROOVE;
	if (length == 5 && strncasecmp(s, "ridge", 5) == 0) return CSS_BORDER_STYLE_RIDGE;
	if (length == 5 && strncasecmp(s, "inset", 5) == 0) return CSS_BORDER_STYLE_INSET;
	if (length == 6 && strncasecmp(s, "outset", 6) == 0) return CSS_BORDER_STYLE_OUTSET;
	return CSS_BORDER_STYLE_UNKNOWN;
}

/**
 * css_caption_side
 */

const char * const css_caption_side_name[] = {
	"inherit",
	"top",
	"bottom"
};

css_caption_side css_caption_side_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_CAPTION_SIDE_INHERIT;
	if (length == 3 && strncasecmp(s, "top", 3) == 0) return CSS_CAPTION_SIDE_TOP;
	if (length == 6 && strncasecmp(s, "bottom", 6) == 0) return CSS_CAPTION_SIDE_BOTTOM;
	return CSS_CAPTION_SIDE_UNKNOWN;
}

/**
 * css_clear
 */

const char * const css_clear_name[] = {
	"inherit",
	"none",
	"both",
	"left",
	"right"
};

css_clear css_clear_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_CLEAR_INHERIT;
	if (length == 4 && strncasecmp(s, "none", 4) == 0) return CSS_CLEAR_NONE;
	if (length == 4 && strncasecmp(s, "both", 4) == 0) return CSS_CLEAR_BOTH;
	if (length == 4 && strncasecmp(s, "left", 4) == 0) return CSS_CLEAR_LEFT;
	if (length == 5 && strncasecmp(s, "right", 5) == 0) return CSS_CLEAR_RIGHT;
	return CSS_CLEAR_UNKNOWN;
}

/**
 * css_cursor
 */

const char * const css_cursor_name[] = {
	"inherit",
	"auto",
	"crosshair",
	"default",
	"pointer",
	"move",
	"e-resize",
	"ne-resize",
	"nw-resize",
	"n-resize",
	"se-resize",
	"sw-resize",
	"s-resize",
	"w-resize",
	"text",
	"wait",
	"help",
	"no-drop",
	"not-allowed",
	"progress"
};

css_cursor css_cursor_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_CURSOR_INHERIT;
	if (length == 4 && strncasecmp(s, "auto", 4) == 0) return CSS_CURSOR_AUTO;
	if (length == 9 && strncasecmp(s, "crosshair", 9) == 0) return CSS_CURSOR_CROSSHAIR;
	if (length == 7 && strncasecmp(s, "default", 7) == 0) return CSS_CURSOR_DEFAULT;
	if (length == 7 && strncasecmp(s, "pointer", 7) == 0) return CSS_CURSOR_POINTER;
	if (length == 4 && strncasecmp(s, "move", 4) == 0) return CSS_CURSOR_MOVE;
	if (length == 8 && strncasecmp(s, "e-resize", 8) == 0) return CSS_CURSOR_E_RESIZE;
	if (length == 9 && strncasecmp(s, "ne-resize", 9) == 0) return CSS_CURSOR_NE_RESIZE;
	if (length == 9 && strncasecmp(s, "nw-resize", 9) == 0) return CSS_CURSOR_NW_RESIZE;
	if (length == 8 && strncasecmp(s, "n-resize", 8) == 0) return CSS_CURSOR_N_RESIZE;
	if (length == 9 && strncasecmp(s, "se-resize", 9) == 0) return CSS_CURSOR_SE_RESIZE;
	if (length == 9 && strncasecmp(s, "sw-resize", 9) == 0) return CSS_CURSOR_SW_RESIZE;
	if (length == 8 && strncasecmp(s, "s-resize", 8) == 0) return CSS_CURSOR_S_RESIZE;
	if (length == 8 && strncasecmp(s, "w-resize", 8) == 0) return CSS_CURSOR_W_RESIZE;
	if (length == 4 && strncasecmp(s, "text", 4) == 0) return CSS_CURSOR_TEXT;
	if (length == 4 && strncasecmp(s, "wait", 4) == 0) return CSS_CURSOR_WAIT;
	if (length == 4 && strncasecmp(s, "help", 4) == 0) return CSS_CURSOR_HELP;
	if (length == 7 && strncasecmp(s, "no-drop", 7) == 0) return CSS_CURSOR_NO_DROP;
	if (length == 11 && strncasecmp(s, "not-allowed", 11) == 0) return CSS_CURSOR_NOT_ALLOWED;
	if (length == 8 && strncasecmp(s, "progress", 8) == 0) return CSS_CURSOR_PROGRESS;
	return CSS_CURSOR_UNKNOWN;
}

/**
 * css_direction
 */

const char * const css_direction_name[] = {
	"inherit",
	"ltr",
	"rtl"
};

css_direction css_direction_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_DIRECTION_INHERIT;
	if (length == 3 && strncasecmp(s, "ltr", 3) == 0) return CSS_DIRECTION_LTR;
	if (length == 3 && strncasecmp(s, "rtl", 3) == 0) return CSS_DIRECTION_RTL;
	return CSS_DIRECTION_UNKNOWN;
}

/**
 * css_display
 */

const char * const css_display_name[] = {
	"inherit",
	"inline",
	"block",
	"list-item",
	"run-in",
	"inline-block",
	"table",
	"inline-table",
	"table-row-group",
	"table-header-group",
	"table-footer-group",
	"table-row",
	"table-column-group",
	"table-column",
	"table-cell",
	"table-caption",
	"none"
};

css_display css_display_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_DISPLAY_INHERIT;
	if (length == 6 && strncasecmp(s, "inline", 6) == 0) return CSS_DISPLAY_INLINE;
	if (length == 5 && strncasecmp(s, "block", 5) == 0) return CSS_DISPLAY_BLOCK;
	if (length == 9 && strncasecmp(s, "list-item", 9) == 0) return CSS_DISPLAY_LIST_ITEM;
	if (length == 6 && strncasecmp(s, "run-in", 6) == 0) return CSS_DISPLAY_RUN_IN;
	if (length == 12 && strncasecmp(s, "inline-block", 12) == 0) return CSS_DISPLAY_INLINE_BLOCK;
	if (length == 5 && strncasecmp(s, "table", 5) == 0) return CSS_DISPLAY_TABLE;
	if (length == 12 && strncasecmp(s, "inline-table", 12) == 0) return CSS_DISPLAY_INLINE_TABLE;
	if (length == 15 && strncasecmp(s, "table-row-group", 15) == 0) return CSS_DISPLAY_TABLE_ROW_GROUP;
	if (length == 18 && strncasecmp(s, "table-header-group", 18) == 0) return CSS_DISPLAY_TABLE_HEADER_GROUP;
	if (length == 18 && strncasecmp(s, "table-footer-group", 18) == 0) return CSS_DISPLAY_TABLE_FOOTER_GROUP;
	if (length == 9 && strncasecmp(s, "table-row", 9) == 0) return CSS_DISPLAY_TABLE_ROW;
	if (length == 18 && strncasecmp(s, "table-column-group", 18) == 0) return CSS_DISPLAY_TABLE_COLUMN_GROUP;
	if (length == 12 && strncasecmp(s, "table-column", 12) == 0) return CSS_DISPLAY_TABLE_COLUMN;
	if (length == 10 && strncasecmp(s, "table-cell", 10) == 0) return CSS_DISPLAY_TABLE_CELL;
	if (length == 13 && strncasecmp(s, "table-caption", 13) == 0) return CSS_DISPLAY_TABLE_CAPTION;
	if (length == 4 && strncasecmp(s, "none", 4) == 0) return CSS_DISPLAY_NONE;
	return CSS_DISPLAY_UNKNOWN;
}

/**
 * css_empty_cells
 */

const char * const css_empty_cells_name[] = {
	"inherit",
	"show",
	"hide"
};

css_empty_cells css_empty_cells_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_EMPTY_CELLS_INHERIT;
	if (length == 4 && strncasecmp(s, "show", 4) == 0) return CSS_EMPTY_CELLS_SHOW;
	if (length == 4 && strncasecmp(s, "hide", 4) == 0) return CSS_EMPTY_CELLS_HIDE;
	return CSS_EMPTY_CELLS_UNKNOWN;
}

/**
 * css_float
 */

const char * const css_float_name[] = {
	"inherit",
	"none",
	"left",
	"right"
};

css_float css_float_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_FLOAT_INHERIT;
	if (length == 4 && strncasecmp(s, "none", 4) == 0) return CSS_FLOAT_NONE;
	if (length == 4 && strncasecmp(s, "left", 4) == 0) return CSS_FLOAT_LEFT;
	if (length == 5 && strncasecmp(s, "right", 5) == 0) return CSS_FLOAT_RIGHT;
	return CSS_FLOAT_UNKNOWN;
}

/**
 * css_font_family
 */

const char * const css_font_family_name[] = {
	"inherit",
	"sans-serif",
	"serif",
	"monospace",
	"cursive",
	"fantasy"
};

css_font_family css_font_family_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_FONT_FAMILY_INHERIT;
	if (length == 10 && strncasecmp(s, "sans-serif", 10) == 0) return CSS_FONT_FAMILY_SANS_SERIF;
	if (length == 5 && strncasecmp(s, "serif", 5) == 0) return CSS_FONT_FAMILY_SERIF;
	if (length == 9 && strncasecmp(s, "monospace", 9) == 0) return CSS_FONT_FAMILY_MONOSPACE;
	if (length == 7 && strncasecmp(s, "cursive", 7) == 0) return CSS_FONT_FAMILY_CURSIVE;
	if (length == 7 && strncasecmp(s, "fantasy", 7) == 0) return CSS_FONT_FAMILY_FANTASY;
	return CSS_FONT_FAMILY_UNKNOWN;
}

/**
 * css_font_style
 */

const char * const css_font_style_name[] = {
	"inherit",
	"normal",
	"italic",
	"oblique"
};

css_font_style css_font_style_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_FONT_STYLE_INHERIT;
	if (length == 6 && strncasecmp(s, "normal", 6) == 0) return CSS_FONT_STYLE_NORMAL;
	if (length == 6 && strncasecmp(s, "italic", 6) == 0) return CSS_FONT_STYLE_ITALIC;
	if (length == 7 && strncasecmp(s, "oblique", 7) == 0) return CSS_FONT_STYLE_OBLIQUE;
	return CSS_FONT_STYLE_UNKNOWN;
}

/**
 * css_font_variant
 */

const char * const css_font_variant_name[] = {
	"inherit",
	"normal",
	"small-caps"
};

css_font_variant css_font_variant_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_FONT_VARIANT_INHERIT;
	if (length == 6 && strncasecmp(s, "normal", 6) == 0) return CSS_FONT_VARIANT_NORMAL;
	if (length == 10 && strncasecmp(s, "small-caps", 10) == 0) return CSS_FONT_VARIANT_SMALL_CAPS;
	return CSS_FONT_VARIANT_UNKNOWN;
}

/**
 * css_font_weight
 */

const char * const css_font_weight_name[] = {
	"inherit",
	"normal",
	"bold",
	"bolder",
	"lighter",
	"100",
	"200",
	"300",
	"400",
	"500",
	"600",
	"700",
	"800",
	"900"
};

css_font_weight css_font_weight_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_FONT_WEIGHT_INHERIT;
	if (length == 6 && strncasecmp(s, "normal", 6) == 0) return CSS_FONT_WEIGHT_NORMAL;
	if (length == 4 && strncasecmp(s, "bold", 4) == 0) return CSS_FONT_WEIGHT_BOLD;
	if (length == 6 && strncasecmp(s, "bolder", 6) == 0) return CSS_FONT_WEIGHT_BOLDER;
	if (length == 7 && strncasecmp(s, "lighter", 7) == 0) return CSS_FONT_WEIGHT_LIGHTER;
	if (length == 3 && strncasecmp(s, "100", 3) == 0) return CSS_FONT_WEIGHT_100;
	if (length == 3 && strncasecmp(s, "200", 3) == 0) return CSS_FONT_WEIGHT_200;
	if (length == 3 && strncasecmp(s, "300", 3) == 0) return CSS_FONT_WEIGHT_300;
	if (length == 3 && strncasecmp(s, "400", 3) == 0) return CSS_FONT_WEIGHT_400;
	if (length == 3 && strncasecmp(s, "500", 3) == 0) return CSS_FONT_WEIGHT_500;
	if (length == 3 && strncasecmp(s, "600", 3) == 0) return CSS_FONT_WEIGHT_600;
	if (length == 3 && strncasecmp(s, "700", 3) == 0) return CSS_FONT_WEIGHT_700;
	if (length == 3 && strncasecmp(s, "800", 3) == 0) return CSS_FONT_WEIGHT_800;
	if (length == 3 && strncasecmp(s, "900", 3) == 0) return CSS_FONT_WEIGHT_900;
	return CSS_FONT_WEIGHT_UNKNOWN;
}

/**
 * css_list_style_position
 */

const char * const css_list_style_position_name[] = {
	"inherit",
	"outside",
	"inside"
};

css_list_style_position css_list_style_position_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_LIST_STYLE_POSITION_INHERIT;
	if (length == 7 && strncasecmp(s, "outside", 7) == 0) return CSS_LIST_STYLE_POSITION_OUTSIDE;
	if (length == 6 && strncasecmp(s, "inside", 6) == 0) return CSS_LIST_STYLE_POSITION_INSIDE;
	return CSS_LIST_STYLE_POSITION_UNKNOWN;
}

/**
 * css_list_style_type
 */

const char * const css_list_style_type_name[] = {
	"inherit",
	"disc",
	"circle",
	"square",
	"decimal",
	"lower-alpha",
	"lower-roman",
	"upper-alpha",
	"upper-roman",
	"none"
};

css_list_style_type css_list_style_type_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_LIST_STYLE_TYPE_INHERIT;
	if (length == 4 && strncasecmp(s, "disc", 4) == 0) return CSS_LIST_STYLE_TYPE_DISC;
	if (length == 6 && strncasecmp(s, "circle", 6) == 0) return CSS_LIST_STYLE_TYPE_CIRCLE;
	if (length == 6 && strncasecmp(s, "square", 6) == 0) return CSS_LIST_STYLE_TYPE_SQUARE;
	if (length == 7 && strncasecmp(s, "decimal", 7) == 0) return CSS_LIST_STYLE_TYPE_DECIMAL;
	if (length == 11 && strncasecmp(s, "lower-alpha", 11) == 0) return CSS_LIST_STYLE_TYPE_LOWER_ALPHA;
	if (length == 11 && strncasecmp(s, "lower-roman", 11) == 0) return CSS_LIST_STYLE_TYPE_LOWER_ROMAN;
	if (length == 11 && strncasecmp(s, "upper-alpha", 11) == 0) return CSS_LIST_STYLE_TYPE_UPPER_ALPHA;
	if (length == 11 && strncasecmp(s, "upper-roman", 11) == 0) return CSS_LIST_STYLE_TYPE_UPPER_ROMAN;
	if (length == 4 && strncasecmp(s, "none", 4) == 0) return CSS_LIST_STYLE_TYPE_NONE;
	return CSS_LIST_STYLE_TYPE_UNKNOWN;
}

/**
 * css_overflow
 */

const char * const css_overflow_name[] = {
	"inherit",
	"visible",
	"hidden",
	"scroll",
	"auto"
};

css_overflow css_overflow_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_OVERFLOW_INHERIT;
	if (length == 7 && strncasecmp(s, "visible", 7) == 0) return CSS_OVERFLOW_VISIBLE;
	if (length == 6 && strncasecmp(s, "hidden", 6) == 0) return CSS_OVERFLOW_HIDDEN;
	if (length == 6 && strncasecmp(s, "scroll", 6) == 0) return CSS_OVERFLOW_SCROLL;
	if (length == 4 && strncasecmp(s, "auto", 4) == 0) return CSS_OVERFLOW_AUTO;
	return CSS_OVERFLOW_UNKNOWN;
}

/**
 * css_page_break_after
 */

const char * const css_page_break_after_name[] = {
	"inherit",
	"auto",
	"always",
	"avoid",
	"left",
	"right"
};

css_page_break_after css_page_break_after_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_PAGE_BREAK_AFTER_INHERIT;
	if (length == 4 && strncasecmp(s, "auto", 4) == 0) return CSS_PAGE_BREAK_AFTER_AUTO;
	if (length == 6 && strncasecmp(s, "always", 6) == 0) return CSS_PAGE_BREAK_AFTER_ALWAYS;
	if (length == 5 && strncasecmp(s, "avoid", 5) == 0) return CSS_PAGE_BREAK_AFTER_AVOID;
	if (length == 4 && strncasecmp(s, "left", 4) == 0) return CSS_PAGE_BREAK_AFTER_LEFT;
	if (length == 5 && strncasecmp(s, "right", 5) == 0) return CSS_PAGE_BREAK_AFTER_RIGHT;
	return CSS_PAGE_BREAK_AFTER_UNKNOWN;
}

/**
 * css_page_break_before
 */

const char * const css_page_break_before_name[] = {
	"inherit",
	"auto",
	"always",
	"avoid",
	"left",
	"right"
};

css_page_break_before css_page_break_before_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_PAGE_BREAK_BEFORE_INHERIT;
	if (length == 4 && strncasecmp(s, "auto", 4) == 0) return CSS_PAGE_BREAK_BEFORE_AUTO;
	if (length == 6 && strncasecmp(s, "always", 6) == 0) return CSS_PAGE_BREAK_BEFORE_ALWAYS;
	if (length == 5 && strncasecmp(s, "avoid", 5) == 0) return CSS_PAGE_BREAK_BEFORE_AVOID;
	if (length == 4 && strncasecmp(s, "left", 4) == 0) return CSS_PAGE_BREAK_BEFORE_LEFT;
	if (length == 5 && strncasecmp(s, "right", 5) == 0) return CSS_PAGE_BREAK_BEFORE_RIGHT;
	return CSS_PAGE_BREAK_BEFORE_UNKNOWN;
}

/**
 * css_page_break_inside
 */

const char * const css_page_break_inside_name[] = {
	"inherit",
	"avoid",
	"auto"
};

css_page_break_inside css_page_break_inside_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_PAGE_BREAK_INSIDE_INHERIT;
	if (length == 5 && strncasecmp(s, "avoid", 5) == 0) return CSS_PAGE_BREAK_INSIDE_AVOID;
	if (length == 4 && strncasecmp(s, "auto", 4) == 0) return CSS_PAGE_BREAK_INSIDE_AUTO;
	return CSS_PAGE_BREAK_INSIDE_UNKNOWN;
}

/**
 * css_position
 */

const char * const css_position_name[] = {
	"inherit",
	"static",
	"relative",
	"absolute",
	"fixed"
};

css_position css_position_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_POSITION_INHERIT;
	if (length == 6 && strncasecmp(s, "static", 6) == 0) return CSS_POSITION_STATIC;
	if (length == 8 && strncasecmp(s, "relative", 8) == 0) return CSS_POSITION_RELATIVE;
	if (length == 8 && strncasecmp(s, "absolute", 8) == 0) return CSS_POSITION_ABSOLUTE;
	if (length == 5 && strncasecmp(s, "fixed", 5) == 0) return CSS_POSITION_FIXED;
	return CSS_POSITION_UNKNOWN;
}

/**
 * css_table_layout
 */

const char * const css_table_layout_name[] = {
	"inherit",
	"auto",
	"fixed"
};

css_table_layout css_table_layout_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_TABLE_LAYOUT_INHERIT;
	if (length == 4 && strncasecmp(s, "auto", 4) == 0) return CSS_TABLE_LAYOUT_AUTO;
	if (length == 5 && strncasecmp(s, "fixed", 5) == 0) return CSS_TABLE_LAYOUT_FIXED;
	return CSS_TABLE_LAYOUT_UNKNOWN;
}

/**
 * css_text_align
 */

const char * const css_text_align_name[] = {
	"inherit",
	"left",
	"right",
	"center",
	"justify"
};

css_text_align css_text_align_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_TEXT_ALIGN_INHERIT;
	if (length == 4 && strncasecmp(s, "left", 4) == 0) return CSS_TEXT_ALIGN_LEFT;
	if (length == 5 && strncasecmp(s, "right", 5) == 0) return CSS_TEXT_ALIGN_RIGHT;
	if (length == 6 && strncasecmp(s, "center", 6) == 0) return CSS_TEXT_ALIGN_CENTER;
	if (length == 7 && strncasecmp(s, "justify", 7) == 0) return CSS_TEXT_ALIGN_JUSTIFY;
	return CSS_TEXT_ALIGN_UNKNOWN;
}

/**
 * css_text_transform
 */

const char * const css_text_transform_name[] = {
	"inherit",
	"none",
	"capitalize",
	"lowercase",
	"uppercase"
};

css_text_transform css_text_transform_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_TEXT_TRANSFORM_INHERIT;
	if (length == 4 && strncasecmp(s, "none", 4) == 0) return CSS_TEXT_TRANSFORM_NONE;
	if (length == 10 && strncasecmp(s, "capitalize", 10) == 0) return CSS_TEXT_TRANSFORM_CAPITALIZE;
	if (length == 9 && strncasecmp(s, "lowercase", 9) == 0) return CSS_TEXT_TRANSFORM_LOWERCASE;
	if (length == 9 && strncasecmp(s, "uppercase", 9) == 0) return CSS_TEXT_TRANSFORM_UPPERCASE;
	return CSS_TEXT_TRANSFORM_UNKNOWN;
}

/**
 * css_unicode_bidi
 */

const char * const css_unicode_bidi_name[] = {
	"inherit",
	"normal",
	"embed",
	"bidi-override"
};

css_unicode_bidi css_unicode_bidi_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_UNICODE_BIDI_INHERIT;
	if (length == 6 && strncasecmp(s, "normal", 6) == 0) return CSS_UNICODE_BIDI_NORMAL;
	if (length == 5 && strncasecmp(s, "embed", 5) == 0) return CSS_UNICODE_BIDI_EMBED;
	if (length == 13 && strncasecmp(s, "bidi-override", 13) == 0) return CSS_UNICODE_BIDI_BIDI_OVERRIDE;
	return CSS_UNICODE_BIDI_UNKNOWN;
}

/**
 * css_visibility
 */

const char * const css_visibility_name[] = {
	"inherit",
	"visible",
	"hidden",
	"collapse"
};

css_visibility css_visibility_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_VISIBILITY_INHERIT;
	if (length == 7 && strncasecmp(s, "visible", 7) == 0) return CSS_VISIBILITY_VISIBLE;
	if (length == 6 && strncasecmp(s, "hidden", 6) == 0) return CSS_VISIBILITY_HIDDEN;
	if (length == 8 && strncasecmp(s, "collapse", 8) == 0) return CSS_VISIBILITY_COLLAPSE;
	return CSS_VISIBILITY_UNKNOWN;
}

/**
 * css_white_space
 */

const char * const css_white_space_name[] = {
	"inherit",
	"normal",
	"nowrap",
	"pre",
	"pre-wrap",
	"pre-line"
};

css_white_space css_white_space_parse(const char * const s, int length)
{
	if (length == 7 && strncasecmp(s, "inherit", 7) == 0) return CSS_WHITE_SPACE_INHERIT;
	if (length == 6 && strncasecmp(s, "normal", 6) == 0) return CSS_WHITE_SPACE_NORMAL;
	if (length == 6 && strncasecmp(s, "nowrap", 6) == 0) return CSS_WHITE_SPACE_NOWRAP;
	if (length == 3 && strncasecmp(s, "pre", 3) == 0) return CSS_WHITE_SPACE_PRE;
	if (length == 8 && strncasecmp(s, "pre-wrap", 8) == 0) return CSS_WHITE_SPACE_PRE_WRAP;
	if (length == 8 && strncasecmp(s, "pre-line", 8) == 0) return CSS_WHITE_SPACE_PRE_LINE;
	return CSS_WHITE_SPACE_UNKNOWN;
}

