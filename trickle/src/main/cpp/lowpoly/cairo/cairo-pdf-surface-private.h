/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2004 Red Hat, Inc
 * Copyright © 2006 Red Hat, Inc
 * Copyright © 2007, 2008 Adrian Johnson
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *	Kristian Høgsberg <krh@redhat.com>
 *	Carl Worth <cworth@cworth.org>
 *	Adrian Johnson <ajohnson@redneon.com>
 */

#ifndef CAIRO_PDF_SURFACE_PRIVATE_H
#define CAIRO_PDF_SURFACE_PRIVATE_H

#include "cairo-pdf.h"

#include "cairo-surface-private.h"
#include "cairo-surface-clipper-private.h"
#include "cairo-pdf-operators-private.h"
#include "cairo-path-fixed-private.h"
#include "cairo-tag-attributes-private.h"
#include "cairo-tag-stack-private.h"

typedef struct _cairo_pdf_resource {
    unsigned int id;
} cairo_pdf_resource_t;


#define CAIRO_NUM_OPERATORS (CAIRO_OPERATOR_HSL_LUMINOSITY + 1)

typedef struct _cairo_pdf_group_resources {
    cairo_bool_t operators[CAIRO_NUM_OPERATORS];
    cairo_array_t alphas;
    cairo_array_t smasks;
    cairo_array_t patterns;
    cairo_array_t shadings;
    cairo_array_t xobjects;
    cairo_array_t fonts;
} cairo_pdf_group_resources_t;

typedef struct _cairo_pdf_source_surface_entry {
    cairo_hash_entry_t base;
    unsigned int id;
    unsigned char *unique_id;
    unsigned long unique_id_length;
    cairo_operator_t operator;

    /* If not 0, this is the recording surface region id of the source surface. */
    int region_id;

    cairo_bool_t interpolate;
    cairo_bool_t stencil_mask;
    cairo_bool_t smask;
    cairo_bool_t need_transp_group;
    cairo_pdf_resource_t surface_res;
    cairo_pdf_resource_t smask_res;

    /* True if surface will be emitted as an Image XObject. */
    cairo_bool_t emit_image;

    /* Extents of the source surface. */
    cairo_bool_t bounded;
    cairo_rectangle_int_t extents;

    /* Union of source extents required for all operations using this source */
    cairo_rectangle_int_t required_extents;
} cairo_pdf_source_surface_entry_t;

typedef struct _cairo_pdf_source_surface {
    cairo_pattern_type_t type;
    cairo_surface_t *surface;
    unsigned int region_id;
    cairo_pattern_t *raster_pattern;
    cairo_pdf_source_surface_entry_t *hash_entry;
} cairo_pdf_source_surface_t;

typedef struct _cairo_pdf_pattern {
    double width;
    double height;
    cairo_rectangle_int_t extents;
    cairo_pattern_t *pattern;
    cairo_pdf_resource_t pattern_res;
    cairo_pdf_resource_t gstate_res;
    cairo_operator_t operator;
    cairo_bool_t is_shading;

    /* Index into nodes array in cairo_pdf_surface_node_entry_t or -1 if not used */
    int region_id;

    /* PDF pattern space is the pattern matrix concatenated with the
     * initial space of the parent object. If the parent object is the
     * page, the initial space does not include the Y-axis flipping
     * matrix emitted at the start of the page content stream.  If the
     * parent object is not the page content stream, the initial space
     * will have a flipped Y-axis. The inverted_y_axis flag is true
     * when the initial space of the parent object that is drawing
     * this pattern has a flipped Y-axis.
     */
    cairo_bool_t inverted_y_axis;
} cairo_pdf_pattern_t;

typedef enum _cairo_pdf_operation {
    PDF_PAINT,
    PDF_MASK,
    PDF_FILL,
    PDF_STROKE,
    PDF_SHOW_GLYPHS
} cairo_pdf_operation_t;

typedef struct _cairo_pdf_smask_group {
    double width;
    double height;
    cairo_rectangle_int_t extents;
    cairo_pdf_resource_t group_res;
    cairo_pdf_operation_t operation;
    cairo_pattern_t *source;
    cairo_pdf_resource_t source_res;
    cairo_pattern_t *mask;
    cairo_path_fixed_t path;
    cairo_fill_rule_t fill_rule;
    cairo_stroke_style_t style;
    cairo_matrix_t ctm;
    cairo_matrix_t ctm_inverse;
    char *utf8;
    int utf8_len;
    cairo_glyph_t *glyphs;
    int num_glyphs;
    cairo_text_cluster_t *clusters;
    int num_clusters;
    cairo_bool_t cluster_flags;
    cairo_scaled_font_t *scaled_font;
} cairo_pdf_smask_group_t;

typedef struct _cairo_pdf_jbig2_global {
    unsigned char *id;
    unsigned long id_length;
    cairo_pdf_resource_t res;
    cairo_bool_t emitted;
} cairo_pdf_jbig2_global_t;

typedef struct _cairo_pdf_page_info {
    double width;
    double height;
    cairo_pdf_resource_t page_res;
    cairo_pdf_resource_t content;
    cairo_pdf_resource_t resources;
    cairo_pdf_resource_t thumbnail;
    cairo_array_t annots; /* <cairo_pdf_resource_t> */
    int struct_parents;
} cairo_pdf_page_info_t;


/* cairo-pdf-interchange.c types */

typedef struct _pdf_tag_extents {
    cairo_rectangle_int_t extents;
    cairo_bool_t valid;
} cairo_pdf_tag_extents_t;

typedef struct _pdf_page_mcid {
    int order;
    int page;
    cairo_pdf_resource_t xobject_res; /* 0 if not in an XObject */
    int mcid;
    struct _cairo_pdf_struct_tree_node *child_node;
} cairo_pdf_page_mcid_t;

/* The non PDF_NODE_STRUCT types are excluded from the struct tree embedded in
 * the PDF. */
typedef enum _cairo_pdf_tree_node_type {
    PDF_NODE_STRUCT,
    PDF_NODE_CONTENT,
    PDF_NODE_CONTENT_REF,
    PDF_NODE_ARTIFACT,
} cairo_pdf_tree_node_type_t;

typedef struct _cairo_pdf_struct_tree_node {
    cairo_hash_entry_t hash;
    cairo_pdf_tree_node_type_t type;
    char *name;
    cairo_pdf_resource_t res;
    struct _cairo_pdf_struct_tree_node *parent;
    cairo_list_t children;
    cairo_array_t mcid; /* array of cairo_pdf_page_mcid_t */
    struct _cairo_pdf_annotation *annot;
    cairo_pdf_tag_extents_t extents;

    union {
        cairo_content_attrs_t content; /* type == PDF_NODE_CONTENT */
        cairo_content_ref_attrs_t content_ref; /* type == PDF_NODE_CONTENT_REF */
    } attributes;

    cairo_list_t link; /* linked list of parent's children */
} cairo_pdf_struct_tree_node_t;

typedef struct _cairo_pdf_command_entry {
    cairo_hash_entry_t base;
    unsigned int recording_id;
    unsigned int command_id;
    cairo_pdf_struct_tree_node_t *node;
} cairo_pdf_command_entry_t;

typedef struct _cairo_recording_surface_stack_entry {
    cairo_bool_t ignore_surface;
    cairo_pdf_struct_tree_node_t *current_node;
} cairo_recording_surface_stack_entry_t;

typedef struct _cairo_pdf_content_tag {
    cairo_hash_entry_t base;
    cairo_pdf_struct_tree_node_t *node;
} cairo_pdf_content_tag_t;

typedef struct _cairo_pdf_annotation {
    cairo_pdf_struct_tree_node_t *node; /* node containing the annotation */
    cairo_link_attrs_t link_attrs;
    cairo_pdf_resource_t res;
} cairo_pdf_annotation_t;

typedef struct _cairo_pdf_named_dest {
    cairo_hash_entry_t base;
    cairo_pdf_tag_extents_t extents;
    cairo_dest_attrs_t attrs;
    int page;
} cairo_pdf_named_dest_t;

typedef struct _cairo_pdf_outline_entry {
    char *name;
    cairo_link_attrs_t link_attrs;
    cairo_pdf_outline_flags_t flags;
    cairo_pdf_resource_t res;
    struct _cairo_pdf_outline_entry *parent;
    struct _cairo_pdf_outline_entry *first_child;
    struct _cairo_pdf_outline_entry *last_child;
    struct _cairo_pdf_outline_entry *next;
    struct _cairo_pdf_outline_entry *prev;
    int count;
} cairo_pdf_outline_entry_t;

typedef struct _cairo_pdf_forward_link {
    cairo_pdf_resource_t res;
    char *dest;
    int page;
    cairo_bool_t has_pos;
    cairo_point_double_t pos;
} cairo_pdf_forward_link_t;

struct docinfo {
    char *title;
    char *author;
    char *subject;
    char *keywords;
    char *creator;
    char *create_date;
    char *mod_date;
};

struct metadata {
    char *name;
    char *value;
};

typedef enum _cairo_pdf_operation_flags_t {
    PDF_NONE = 0,
    PDF_CONTENT,
    PDF_BEGIN,
    PDF_END,
    PDF_GROUP,
} cairo_pdf_operation_flags_t;

typedef struct _pdf_command_list {
    cairo_array_t commands;
    struct _pdf_command_list *parent;
} cairo_pdf_command_list_t;

typedef struct _pdf_operation {
    cairo_pdf_command_list_t *group;
    cairo_pdf_struct_tree_node_t *node;
    unsigned int command_id;
    int mcid_index;
    cairo_pdf_operation_flags_t flags;
} cairo_pdf_command_t;

typedef struct _pdf_recording_surface_commands {
    cairo_surface_t *recording_surface;
    cairo_pdf_command_list_t *command_list;
    unsigned int region_id;
} cairo_pdf_recording_surface_commands_t;

typedef struct _cairo_pdf_interchange {
    cairo_tag_stack_t analysis_tag_stack;
    cairo_tag_stack_t render_tag_stack;
    cairo_pdf_struct_tree_node_t *struct_root;

    /* Current position in the tree during the analysis stage and across
     * pages as each page adds to the tree */
    cairo_pdf_struct_tree_node_t *current_analyze_node;

    /* Currently open tag content containing content. NULL if no content tag open.
     * A content containg tag may be open across pages */
    cairo_pdf_struct_tree_node_t *current_render_node;
    cairo_pdf_struct_tree_node_t *next_page_render_node;

    cairo_array_t recording_surface_stack; /* cairo_recording_surface_stack_entry_t */
    cairo_pdf_resource_t current_recording_surface_res;
    cairo_hash_table_t *command_to_node_map; /* <cairo_pdf_surface_node_entry_t> */
    cairo_bool_t ignore_current_surface;
    cairo_hash_table_t *content_tag_map; /* <char*,cairo_pdf_content_tag_t> */

    cairo_array_t parent_tree; /* <cairo_pdf_resource_t> */
    cairo_array_t annots; /* array of pointers to cairo_pdf_annotation_t */
    cairo_pdf_resource_t content_parent_res;
    cairo_pdf_resource_t parent_tree_res;

    /* mcid to tree node for current page or group */
    cairo_array_t mcid_to_tree; /* <cairo_pdf_struct_tree_node_t *> */

    cairo_array_t page_commands; /* <cairo_pdf_command_list_t> */
    cairo_pdf_command_list_t *current_commands; /* <cairo_pdf_command_list_t> */
    cairo_array_t recording_surface_commands;  /* <cairo_pdf_recording_surface_commands_t> */

    cairo_list_t extents_list;
    cairo_hash_table_t *named_dests;
    int num_dests;
    cairo_pdf_named_dest_t **sorted_dests;
    cairo_pdf_resource_t dests_res;
    int annot_page;
    cairo_array_t outline; /* array of pointers to cairo_pdf_outline_entry_t; */
    struct docinfo docinfo;
    cairo_array_t custom_metadata; /* array of struct metadata */
    cairo_bool_t content_emitted;
    cairo_bool_t marked_content_open;
    unsigned int recording_id;
    unsigned int command_id;
    cairo_bool_t render_next_command_has_content;
    int mcid_order;

} cairo_pdf_interchange_t;

typedef struct _cairo_pdf_color_glyph {
    cairo_hash_entry_t base;
    cairo_scaled_font_t *scaled_font;
    unsigned long glyph_index;
    cairo_bool_t supported;
} cairo_pdf_color_glyph_t;

/* pdf surface data */

typedef struct _cairo_pdf_surface cairo_pdf_surface_t;

struct _cairo_pdf_surface {
    cairo_surface_t base;

    /* Prefer the name "output" here to avoid confusion over the
     * structure within a PDF document known as a "stream". */
    cairo_output_stream_t *output;

    double width;
    double height;
    cairo_rectangle_int_t surface_extents;
    cairo_bool_t surface_bounded;
    cairo_matrix_t cairo_to_pdf;
    cairo_bool_t in_xobject;

    cairo_array_t objects; /* cairo_pdf_resource_t - list of every resource in the PDF */
    cairo_array_t pages; /* <cairo_pdf_page_info_t> */
    cairo_array_t rgb_linear_functions;
    cairo_array_t alpha_linear_functions;
    cairo_array_t page_patterns; /* cairo_pdf_pattern_t */
    cairo_array_t page_surfaces; /* cairo_pdf_source_surface_t */
    cairo_array_t doc_surfaces; /* cairo_pdf_source_surface_t */
    cairo_hash_table_t *all_surfaces; /* cairo_pdf_source_surface_entry_t* */
    int duplicate_surface_number;
    cairo_array_t smask_groups;
    cairo_array_t knockout_group;
    cairo_array_t jbig2_global;
    cairo_hash_table_t *color_glyphs;

    cairo_scaled_font_subsets_t *font_subsets;
    cairo_array_t fonts;

    cairo_pdf_resource_t next_available_resource;
    cairo_pdf_resource_t pages_resource;
    cairo_pdf_resource_t struct_tree_root;

    cairo_pdf_version_t pdf_version;
    cairo_bool_t compress_streams;

    cairo_pdf_resource_t content;
    cairo_pdf_resource_t content_resources;
    cairo_pdf_group_resources_t resources;
    cairo_bool_t has_fallback_images;
    cairo_bool_t header_emitted;

    struct {
        cairo_bool_t active;
        cairo_pdf_resource_t self;
        cairo_pdf_resource_t length;
        long long start_offset;
        cairo_bool_t compressed;
        cairo_output_stream_t *old_output;
    } pdf_stream;

    struct {
        cairo_bool_t active;
        cairo_output_stream_t *stream;
        cairo_output_stream_t *mem_stream;
        cairo_output_stream_t *old_output;
        cairo_pdf_resource_t resource;
        cairo_box_double_t bbox;
        cairo_bool_t is_knockout;
    } group_stream;

    struct {
        cairo_bool_t active;
        cairo_output_stream_t *stream;
        cairo_pdf_resource_t resource;
        cairo_array_t objects;
    } object_stream;

    cairo_surface_clipper_t clipper;

    cairo_pdf_operators_t pdf_operators;
    cairo_paginated_mode_t paginated_mode;
    cairo_bool_t type3_replay;
    cairo_bool_t select_pattern_gstate_saved;

    cairo_bool_t force_fallbacks;

    cairo_operator_t current_operator;
    cairo_bool_t reset_gs_required;
    cairo_bool_t current_pattern_is_solid_color;
    cairo_bool_t current_color_is_stroke;
    double current_color_red;
    double current_color_green;
    double current_color_blue;
    double current_color_alpha;

    cairo_pdf_interchange_t interchange;
    int page_parent_tree; /* -1 if not used */
    cairo_array_t page_annots;
    cairo_array_t forward_links;
    cairo_bool_t tagged;
    char *current_page_label;
    cairo_array_t page_labels;
    cairo_pdf_resource_t outlines_dict_res;
    cairo_pdf_resource_t names_dict_res;
    cairo_pdf_resource_t docinfo_res;
    cairo_pdf_resource_t page_labels_res;

    int thumbnail_width;
    int thumbnail_height;
    cairo_image_surface_t *thumbnail_image;

    cairo_surface_t *paginated_surface;

    cairo_bool_t debug;
};

cairo_private cairo_pdf_resource_t
_cairo_pdf_surface_new_object(cairo_pdf_surface_t *surface);

cairo_private void
_cairo_pdf_surface_update_object(cairo_pdf_surface_t *surface,
                                 cairo_pdf_resource_t resource);

cairo_private cairo_int_status_t
_cairo_utf8_to_pdf_string(const char *utf8, char **str_out);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_init(cairo_pdf_surface_t *surface);

cairo_private void
_cairo_pdf_interchange_fini(cairo_pdf_surface_t *surface);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_begin_page_content(cairo_pdf_surface_t *surface);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_end_page_content(cairo_pdf_surface_t *surface);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_tag_begin(cairo_pdf_surface_t *surface,
                                 const char *name,
                                 const char *attributes);

cairo_private cairo_int_status_t
_cairo_pdf_surface_object_begin(cairo_pdf_surface_t *surface,
                                cairo_pdf_resource_t resource);

cairo_private void
_cairo_pdf_surface_object_end(cairo_pdf_surface_t *surface);

cairo_private cairo_bool_t
_cairo_pdf_interchange_struct_tree_requires_recording_surface(
        cairo_pdf_surface_t *surface,
        const cairo_surface_pattern_t *recording_surface,
        cairo_analysis_source_t source_type);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_recording_source_surface_begin(
        cairo_pdf_surface_t *surface,
        const cairo_surface_pattern_t *recording_surface_pattern,
        unsigned int region_id,
        cairo_analysis_source_t source_type);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_recording_source_surface_end(
        cairo_pdf_surface_t *surface,
        const cairo_surface_pattern_t *recording_surface_pattern,
        unsigned int region_id,
        cairo_analysis_source_t source_type);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_emit_recording_surface_begin(
        cairo_pdf_surface_t *surface,
        cairo_surface_t *recording_surface,
        int region_id,
        cairo_pdf_resource_t surface_resource,
        int *struct_parents);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_emit_recording_surface_end(
        cairo_pdf_surface_t *surface,
        cairo_surface_t *recording_surface);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_tag_end(cairo_pdf_surface_t *surface,
                               const char *name);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_command_id(cairo_pdf_surface_t *surface,
                                  unsigned int recording_id,
                                  unsigned int command_id);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_add_operation_extents(cairo_pdf_surface_t *surface,
                                             const cairo_rectangle_int_t *extents);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_add_content(cairo_pdf_surface_t *surface);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_write_page_objects(cairo_pdf_surface_t *surface);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_write_document_objects(cairo_pdf_surface_t *surface);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_add_outline(cairo_pdf_surface_t *surface,
                                   int parent_id,
                                   const char *name,
                                   const char *dest,
                                   cairo_pdf_outline_flags_t flags,
                                   int *id);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_set_metadata(cairo_pdf_surface_t *surface,
                                    cairo_pdf_metadata_t metadata,
                                    const char *utf8);

cairo_private cairo_int_status_t
_cairo_pdf_interchange_set_custom_metadata(cairo_pdf_surface_t *surface,
                                           const char *name,
                                           const char *value);

#endif /* CAIRO_PDF_SURFACE_PRIVATE_H */
