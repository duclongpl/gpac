/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Compositor sub-project
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#ifndef _VISUAL_MANAGER_H_
#define _VISUAL_MANAGER_H_

#include "drawable.h"

/*all 2D related functions and macro are locate there*/
#include "visual_manager_2d.h"

/*all 3D related functions and macro are locate there*/
#include "visual_manager_3d.h"


enum 
{
	GF_3D_STEREO_NONE = 0,
	GF_3D_STEREO_TOP = 1,
	GF_3D_STEREO_SIDE = 2,
	/*all modes above GF_3D_STEREO_SIDE require shaders*/

	/*each pixel correspond to a different view*/
	GF_3D_STEREO_COLUMNS = 3,
	GF_3D_STEREO_ROWS = 4,
	/*special case of sub-pixel interleaving for 2 views*/
	GF_3D_STEREO_ANAGLYPH = 5,
	/*custom interleaving using GLSL shaders*/
	GF_3D_STEREO_CUSTOM = 6,
};

enum 
{
	GF_3D_CAMERA_STRAIGHT = 0,
	GF_3D_CAMERA_OFFAXIS,
	GF_3D_CAMERA_LINEAR,
	GF_3D_CAMERA_CIRCULAR,
};

struct _visual_manager
{
	GF_Compositor *compositor;
	Bool direct_flush;

#ifndef GPAC_DISABLE_3D
	/*3D type for the visual:
	0: visual is 2D
	1: visual is 2D with 3D acceleration (2D camera)
	2: visual is 3D MPEG-4 (with 3D camera)
	3: visual is 3D X3D (with 3D camera)
	*/
	u32 type_3d;
#endif


#ifndef GPAC_DISABLE_VRML
	/*background stack*/
	GF_List *back_stack;
	/*viewport stack*/
	GF_List *view_stack;
#endif


	/*size in pixels*/
	u32 width, height;

	/*
	 *	Visual Manager part for 2D drawing and dirty rect
	 */

	/*the one and only dirty rect collector for this visual manager*/
	GF_RectArray to_redraw;
#ifdef TRACK_OPAQUE_REGIONS
	u32 draw_node_index;
#endif

	/*display list (list of drawable context). The first context with no drawable attached to 
	it (ctx->drawable==NULL) marks the end of the display list*/
	DrawableContext *context, *cur_context;

	/*keeps track of nodes drawn last frame*/
	struct _drawable_store *prev_nodes, *last_prev_entry;

	/*pixel area in BIFS coords - eg area to fill with background*/
	GF_IRect surf_rect;
	/*top clipper (may be different than surf_rect when a viewport is active)*/
	GF_IRect top_clipper;

	Bool last_had_back;

	/*signals that the hardware surface is attached to buffer/device/stencil*/
	Bool is_attached;
	Bool center_coords;
	Bool has_modif;
	Bool has_overlays;
	Bool has_text_edit;

	/*gets access to graphics handle (either OS-specific or raw memory)*/
	GF_Err (*GetSurfaceAccess)(GF_VisualManager *);
	/*release graphics handle*/
	void (*ReleaseSurfaceAccess)(GF_VisualManager *);

	/*clear given rect or all visual if no rect specified - clear color depends on visual's type:
		BackColor for background nodes
		0x00000000 for composite, 
		compositor clear color otherwise
	*/
	void (*ClearSurface)(GF_VisualManager *visual, GF_IRect *rc, u32 BackColor);
	/*draws specified texture as flat bitmap*/
	Bool (*DrawBitmap)(GF_VisualManager *visual, GF_TraverseState *tr_state, DrawableContext *ctx, GF_ColorKey *col_key);

	/*raster surface interface*/
	GF_SURFACE raster_surface;
	/*raster brush interface*/
	GF_STENCIL raster_brush;

	/*node owning this visual manager (composite textures) - NULL for root visual*/
	GF_Node *offscreen;

	/*value of the flag to use to signal any geometry changes*/
	u32 bounds_tracker_modif_flag;

	u32 num_nodes_prev_frame, num_nodes_current_frame;

	/*list of video overlays sorted from first to last*/
	struct _video_overlay *overlays;

#ifndef GPAC_DISABLE_3D
	/*
	 *	Visual Manager part for 3D drawing 
	 */

#ifndef GPAC_DISABLE_VRML
	/*navigation stack*/
	GF_List *navigation_stack;
	/*fog stack*/
	GF_List *fog_stack;
#endif

	/*the one and only camera associated with the visual*/
	GF_Camera camera;

	/*list of transparent nodes to draw after TRAVERSE_SORT pass*/
	GF_List *alpha_nodes_to_draw;

	/*lighting stuff*/
	u32 num_lights;
	u32 max_lights;
	/*cliping stuff*/
	u32 num_clips;
	u32 max_clips;


	u32 nb_views, current_view, autostereo_type, camera_layout;
	Bool reverse_views;

	u32 *gl_textures;
	u32 auto_stereo_width, auto_stereo_height;
	GF_Mesh *autostereo_mesh;
	u32 glsl_program;
	u32 glsl_vertex;
	u32 glsl_fragment;
#endif

#ifdef GF_SR_USE_DEPTH
	Fixed depth_vp_position, depth_vp_range;
#endif

};

/*constructor/destructor*/
GF_VisualManager *visual_new(GF_Compositor *compositor);
void visual_del(GF_VisualManager *visual);

/*draw cycle for the visual*/
Bool visual_draw_frame(GF_VisualManager *visual, GF_Node *root, GF_TraverseState *tr_state, Bool is_root_visual);

/*executes scene event (picks node if needed) - returns FALSE if no scene event handler has been called*/
Bool visual_execute_event(GF_VisualManager *visual, GF_TraverseState *tr_state, GF_Event *ev, GF_ChildNodeItem *children);

Bool visual_get_size_info(GF_TraverseState *tr_state, Fixed *surf_width, Fixed *surf_height);

/*reset all appearance dirty state and visual registration info*/
void visual_clean_contexts(GF_VisualManager *visual);


void visual_reset_graphics(GF_VisualManager *visual);

#endif	/*_VISUAL_MANAGER_H_*/

