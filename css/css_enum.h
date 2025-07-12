extern const char * const css_unit_name[];
typedef enum {
	CSS_UNIT_EM,
	CSS_UNIT_EX,
	CSS_UNIT_PX,
	CSS_UNIT_IN,
	CSS_UNIT_CM,
	CSS_UNIT_MM,
	CSS_UNIT_PT,
	CSS_UNIT_PC,
	CSS_UNIT_UNKNOWN,
	CSS_UNIT_NOT_SET
} css_unit;
css_unit css_unit_parse(const char * const s, int length);

extern const char * const css_background_attachment_name[];
typedef enum {
	CSS_BACKGROUND_ATTACHMENT_INHERIT,
	CSS_BACKGROUND_ATTACHMENT_FIXED,
	CSS_BACKGROUND_ATTACHMENT_SCROLL,
	CSS_BACKGROUND_ATTACHMENT_UNKNOWN,
	CSS_BACKGROUND_ATTACHMENT_NOT_SET
} css_background_attachment;
css_background_attachment css_background_attachment_parse(const char * const s, int length);

extern const char * const css_background_repeat_name[];
typedef enum {
	CSS_BACKGROUND_REPEAT_INHERIT,
	CSS_BACKGROUND_REPEAT_REPEAT,
	CSS_BACKGROUND_REPEAT_REPEAT_X,
	CSS_BACKGROUND_REPEAT_REPEAT_Y,
	CSS_BACKGROUND_REPEAT_NO_REPEAT,
	CSS_BACKGROUND_REPEAT_UNKNOWN,
	CSS_BACKGROUND_REPEAT_NOT_SET
} css_background_repeat;
css_background_repeat css_background_repeat_parse(const char * const s, int length);

extern const char * const css_border_collapse_name[];
typedef enum {
	CSS_BORDER_COLLAPSE_INHERIT,
	CSS_BORDER_COLLAPSE_COLLAPSE,
	CSS_BORDER_COLLAPSE_SEPARATE,
	CSS_BORDER_COLLAPSE_UNKNOWN,
	CSS_BORDER_COLLAPSE_NOT_SET
} css_border_collapse;
css_border_collapse css_border_collapse_parse(const char * const s, int length);

extern const char * const css_border_style_name[];
typedef enum {
	CSS_BORDER_STYLE_INHERIT,
	CSS_BORDER_STYLE_NONE,
	CSS_BORDER_STYLE_HIDDEN,
	CSS_BORDER_STYLE_DOTTED,
	CSS_BORDER_STYLE_DASHED,
	CSS_BORDER_STYLE_SOLID,
	CSS_BORDER_STYLE_DOUBLE,
	CSS_BORDER_STYLE_GROOVE,
	CSS_BORDER_STYLE_RIDGE,
	CSS_BORDER_STYLE_INSET,
	CSS_BORDER_STYLE_OUTSET,
	CSS_BORDER_STYLE_UNKNOWN,
	CSS_BORDER_STYLE_NOT_SET
} css_border_style;
css_border_style css_border_style_parse(const char * const s, int length);

extern const char * const css_caption_side_name[];
typedef enum {
	CSS_CAPTION_SIDE_INHERIT,
	CSS_CAPTION_SIDE_TOP,
	CSS_CAPTION_SIDE_BOTTOM,
	CSS_CAPTION_SIDE_UNKNOWN,
	CSS_CAPTION_SIDE_NOT_SET
} css_caption_side;
css_caption_side css_caption_side_parse(const char * const s, int length);

extern const char * const css_clear_name[];
typedef enum {
	CSS_CLEAR_INHERIT,
	CSS_CLEAR_NONE,
	CSS_CLEAR_BOTH,
	CSS_CLEAR_LEFT,
	CSS_CLEAR_RIGHT,
	CSS_CLEAR_UNKNOWN,
	CSS_CLEAR_NOT_SET
} css_clear;
css_clear css_clear_parse(const char * const s, int length);

extern const char * const css_cursor_name[];
typedef enum {
	CSS_CURSOR_INHERIT,
	CSS_CURSOR_AUTO,
	CSS_CURSOR_CROSSHAIR,
	CSS_CURSOR_DEFAULT,
	CSS_CURSOR_POINTER,
	CSS_CURSOR_MOVE,
	CSS_CURSOR_E_RESIZE,
	CSS_CURSOR_NE_RESIZE,
	CSS_CURSOR_NW_RESIZE,
	CSS_CURSOR_N_RESIZE,
	CSS_CURSOR_SE_RESIZE,
	CSS_CURSOR_SW_RESIZE,
	CSS_CURSOR_S_RESIZE,
	CSS_CURSOR_W_RESIZE,
	CSS_CURSOR_TEXT,
	CSS_CURSOR_WAIT,
	CSS_CURSOR_HELP,
	CSS_CURSOR_NO_DROP,
	CSS_CURSOR_NOT_ALLOWED,
	CSS_CURSOR_PROGRESS,
	CSS_CURSOR_UNKNOWN,
	CSS_CURSOR_NOT_SET
} css_cursor;
css_cursor css_cursor_parse(const char * const s, int length);

extern const char * const css_direction_name[];
typedef enum {
	CSS_DIRECTION_INHERIT,
	CSS_DIRECTION_LTR,
	CSS_DIRECTION_RTL,
	CSS_DIRECTION_UNKNOWN,
	CSS_DIRECTION_NOT_SET
} css_direction;
css_direction css_direction_parse(const char * const s, int length);

extern const char * const css_display_name[];
typedef enum {
	CSS_DISPLAY_INHERIT,
	CSS_DISPLAY_INLINE,
	CSS_DISPLAY_BLOCK,
	CSS_DISPLAY_LIST_ITEM,
	CSS_DISPLAY_RUN_IN,
	CSS_DISPLAY_INLINE_BLOCK,
	CSS_DISPLAY_TABLE,
	CSS_DISPLAY_INLINE_TABLE,
	CSS_DISPLAY_TABLE_ROW_GROUP,
	CSS_DISPLAY_TABLE_HEADER_GROUP,
	CSS_DISPLAY_TABLE_FOOTER_GROUP,
	CSS_DISPLAY_TABLE_ROW,
	CSS_DISPLAY_TABLE_COLUMN_GROUP,
	CSS_DISPLAY_TABLE_COLUMN,
	CSS_DISPLAY_TABLE_CELL,
	CSS_DISPLAY_TABLE_CAPTION,
	CSS_DISPLAY_NONE,
	CSS_DISPLAY_UNKNOWN,
	CSS_DISPLAY_NOT_SET
} css_display;
css_display css_display_parse(const char * const s, int length);

extern const char * const css_empty_cells_name[];
typedef enum {
	CSS_EMPTY_CELLS_INHERIT,
	CSS_EMPTY_CELLS_SHOW,
	CSS_EMPTY_CELLS_HIDE,
	CSS_EMPTY_CELLS_UNKNOWN,
	CSS_EMPTY_CELLS_NOT_SET
} css_empty_cells;
css_empty_cells css_empty_cells_parse(const char * const s, int length);

extern const char * const css_float_name[];
typedef enum {
	CSS_FLOAT_INHERIT,
	CSS_FLOAT_NONE,
	CSS_FLOAT_LEFT,
	CSS_FLOAT_RIGHT,
	CSS_FLOAT_UNKNOWN,
	CSS_FLOAT_NOT_SET
} css_float;
css_float css_float_parse(const char * const s, int length);

extern const char * const css_font_family_name[];
typedef enum {
	CSS_FONT_FAMILY_INHERIT,
	CSS_FONT_FAMILY_SANS_SERIF,
	CSS_FONT_FAMILY_SERIF,
	CSS_FONT_FAMILY_MONOSPACE,
	CSS_FONT_FAMILY_CURSIVE,
	CSS_FONT_FAMILY_FANTASY,
	CSS_FONT_FAMILY_UNKNOWN,
	CSS_FONT_FAMILY_NOT_SET
} css_font_family;
css_font_family css_font_family_parse(const char * const s, int length);

extern const char * const css_font_style_name[];
typedef enum {
	CSS_FONT_STYLE_INHERIT,
	CSS_FONT_STYLE_NORMAL,
	CSS_FONT_STYLE_ITALIC,
	CSS_FONT_STYLE_OBLIQUE,
	CSS_FONT_STYLE_UNKNOWN,
	CSS_FONT_STYLE_NOT_SET
} css_font_style;
css_font_style css_font_style_parse(const char * const s, int length);

extern const char * const css_font_variant_name[];
typedef enum {
	CSS_FONT_VARIANT_INHERIT,
	CSS_FONT_VARIANT_NORMAL,
	CSS_FONT_VARIANT_SMALL_CAPS,
	CSS_FONT_VARIANT_UNKNOWN,
	CSS_FONT_VARIANT_NOT_SET
} css_font_variant;
css_font_variant css_font_variant_parse(const char * const s, int length);

extern const char * const css_font_weight_name[];
typedef enum {
	CSS_FONT_WEIGHT_INHERIT,
	CSS_FONT_WEIGHT_NORMAL,
	CSS_FONT_WEIGHT_BOLD,
	CSS_FONT_WEIGHT_BOLDER,
	CSS_FONT_WEIGHT_LIGHTER,
	CSS_FONT_WEIGHT_100,
	CSS_FONT_WEIGHT_200,
	CSS_FONT_WEIGHT_300,
	CSS_FONT_WEIGHT_400,
	CSS_FONT_WEIGHT_500,
	CSS_FONT_WEIGHT_600,
	CSS_FONT_WEIGHT_700,
	CSS_FONT_WEIGHT_800,
	CSS_FONT_WEIGHT_900,
	CSS_FONT_WEIGHT_UNKNOWN,
	CSS_FONT_WEIGHT_NOT_SET
} css_font_weight;
css_font_weight css_font_weight_parse(const char * const s, int length);

extern const char * const css_list_style_position_name[];
typedef enum {
	CSS_LIST_STYLE_POSITION_INHERIT,
	CSS_LIST_STYLE_POSITION_OUTSIDE,
	CSS_LIST_STYLE_POSITION_INSIDE,
	CSS_LIST_STYLE_POSITION_UNKNOWN,
	CSS_LIST_STYLE_POSITION_NOT_SET
} css_list_style_position;
css_list_style_position css_list_style_position_parse(const char * const s, int length);

extern const char * const css_list_style_type_name[];
typedef enum {
	CSS_LIST_STYLE_TYPE_INHERIT,
	CSS_LIST_STYLE_TYPE_DISC,
	CSS_LIST_STYLE_TYPE_CIRCLE,
	CSS_LIST_STYLE_TYPE_SQUARE,
	CSS_LIST_STYLE_TYPE_DECIMAL,
	CSS_LIST_STYLE_TYPE_LOWER_ALPHA,
	CSS_LIST_STYLE_TYPE_LOWER_ROMAN,
	CSS_LIST_STYLE_TYPE_UPPER_ALPHA,
	CSS_LIST_STYLE_TYPE_UPPER_ROMAN,
	CSS_LIST_STYLE_TYPE_NONE,
	CSS_LIST_STYLE_TYPE_UNKNOWN,
	CSS_LIST_STYLE_TYPE_NOT_SET
} css_list_style_type;
css_list_style_type css_list_style_type_parse(const char * const s, int length);

extern const char * const css_overflow_name[];
typedef enum {
	CSS_OVERFLOW_INHERIT,
	CSS_OVERFLOW_VISIBLE,
	CSS_OVERFLOW_HIDDEN,
	CSS_OVERFLOW_SCROLL,
	CSS_OVERFLOW_AUTO,
	CSS_OVERFLOW_UNKNOWN,
	CSS_OVERFLOW_NOT_SET
} css_overflow;
css_overflow css_overflow_parse(const char * const s, int length);

extern const char * const css_page_break_after_name[];
typedef enum {
	CSS_PAGE_BREAK_AFTER_INHERIT,
	CSS_PAGE_BREAK_AFTER_AUTO,
	CSS_PAGE_BREAK_AFTER_ALWAYS,
	CSS_PAGE_BREAK_AFTER_AVOID,
	CSS_PAGE_BREAK_AFTER_LEFT,
	CSS_PAGE_BREAK_AFTER_RIGHT,
	CSS_PAGE_BREAK_AFTER_UNKNOWN,
	CSS_PAGE_BREAK_AFTER_NOT_SET
} css_page_break_after;
css_page_break_after css_page_break_after_parse(const char * const s, int length);

extern const char * const css_page_break_before_name[];
typedef enum {
	CSS_PAGE_BREAK_BEFORE_INHERIT,
	CSS_PAGE_BREAK_BEFORE_AUTO,
	CSS_PAGE_BREAK_BEFORE_ALWAYS,
	CSS_PAGE_BREAK_BEFORE_AVOID,
	CSS_PAGE_BREAK_BEFORE_LEFT,
	CSS_PAGE_BREAK_BEFORE_RIGHT,
	CSS_PAGE_BREAK_BEFORE_UNKNOWN,
	CSS_PAGE_BREAK_BEFORE_NOT_SET
} css_page_break_before;
css_page_break_before css_page_break_before_parse(const char * const s, int length);

extern const char * const css_page_break_inside_name[];
typedef enum {
	CSS_PAGE_BREAK_INSIDE_INHERIT,
	CSS_PAGE_BREAK_INSIDE_AVOID,
	CSS_PAGE_BREAK_INSIDE_AUTO,
	CSS_PAGE_BREAK_INSIDE_UNKNOWN,
	CSS_PAGE_BREAK_INSIDE_NOT_SET
} css_page_break_inside;
css_page_break_inside css_page_break_inside_parse(const char * const s, int length);

extern const char * const css_position_name[];
typedef enum {
	CSS_POSITION_INHERIT,
	CSS_POSITION_STATIC,
	CSS_POSITION_RELATIVE,
	CSS_POSITION_ABSOLUTE,
	CSS_POSITION_FIXED,
	CSS_POSITION_UNKNOWN,
	CSS_POSITION_NOT_SET
} css_position;
css_position css_position_parse(const char * const s, int length);

extern const char * const css_table_layout_name[];
typedef enum {
	CSS_TABLE_LAYOUT_INHERIT,
	CSS_TABLE_LAYOUT_AUTO,
	CSS_TABLE_LAYOUT_FIXED,
	CSS_TABLE_LAYOUT_UNKNOWN,
	CSS_TABLE_LAYOUT_NOT_SET
} css_table_layout;
css_table_layout css_table_layout_parse(const char * const s, int length);

extern const char * const css_text_align_name[];
typedef enum {
	CSS_TEXT_ALIGN_INHERIT,
	CSS_TEXT_ALIGN_LEFT,
	CSS_TEXT_ALIGN_RIGHT,
	CSS_TEXT_ALIGN_CENTER,
	CSS_TEXT_ALIGN_JUSTIFY,
	CSS_TEXT_ALIGN_UNKNOWN,
	CSS_TEXT_ALIGN_NOT_SET
} css_text_align;
css_text_align css_text_align_parse(const char * const s, int length);

extern const char * const css_text_transform_name[];
typedef enum {
	CSS_TEXT_TRANSFORM_INHERIT,
	CSS_TEXT_TRANSFORM_NONE,
	CSS_TEXT_TRANSFORM_CAPITALIZE,
	CSS_TEXT_TRANSFORM_LOWERCASE,
	CSS_TEXT_TRANSFORM_UPPERCASE,
	CSS_TEXT_TRANSFORM_UNKNOWN,
	CSS_TEXT_TRANSFORM_NOT_SET
} css_text_transform;
css_text_transform css_text_transform_parse(const char * const s, int length);

extern const char * const css_unicode_bidi_name[];
typedef enum {
	CSS_UNICODE_BIDI_INHERIT,
	CSS_UNICODE_BIDI_NORMAL,
	CSS_UNICODE_BIDI_EMBED,
	CSS_UNICODE_BIDI_BIDI_OVERRIDE,
	CSS_UNICODE_BIDI_UNKNOWN,
	CSS_UNICODE_BIDI_NOT_SET
} css_unicode_bidi;
css_unicode_bidi css_unicode_bidi_parse(const char * const s, int length);

extern const char * const css_visibility_name[];
typedef enum {
	CSS_VISIBILITY_INHERIT,
	CSS_VISIBILITY_VISIBLE,
	CSS_VISIBILITY_HIDDEN,
	CSS_VISIBILITY_COLLAPSE,
	CSS_VISIBILITY_UNKNOWN,
	CSS_VISIBILITY_NOT_SET
} css_visibility;
css_visibility css_visibility_parse(const char * const s, int length);

extern const char * const css_white_space_name[];
typedef enum {
	CSS_WHITE_SPACE_INHERIT,
	CSS_WHITE_SPACE_NORMAL,
	CSS_WHITE_SPACE_NOWRAP,
	CSS_WHITE_SPACE_PRE,
	CSS_WHITE_SPACE_PRE_WRAP,
	CSS_WHITE_SPACE_PRE_LINE,
	CSS_WHITE_SPACE_UNKNOWN,
	CSS_WHITE_SPACE_NOT_SET
} css_white_space;
css_white_space css_white_space_parse(const char * const s, int length);

