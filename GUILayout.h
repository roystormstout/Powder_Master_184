#pragma once
#include "NuklearInit.h"
#include <string>
#include <vector>
#define EVADE_INDEX 0
#define PROJ_INDEX 1
#define OMNI_SKILL_INDEX 2
#define DIR_SKILL_INDEX 3

#define KILLED_TEXT_NUM 4

#define CHEATING_GOLD 45
#define CHEATING_POINTS 3

using namespace std;

// order leaderboard by kills, most first
static void ui_statstics(struct nk_context *ctx, struct media *media,
	guiStatus status ) {

	struct nk_style* s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(1.0, 1.0, 1.0, 1.0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(1.0, 1.0, 1.0, 1.0)));
	nk_style_set_font(ctx, &(media->font_22->handle));

	string p_count_str = std::to_string(status.curr_parts);
	const char* p_count_chars = p_count_str.c_str();
	string fps_str = std::to_string(status.fps);
	const char* fps_chars = fps_str.c_str();
	if (nk_begin(ctx, "STATISTICS", nk_rect(10, 10, 300, 150),
		NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		static const float lbratio[] = { 0.6f, 0.1f, 0.3f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, 90, 3, lbratio);
		if (nk_group_begin(ctx, "icons", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 30, 180, 1);
			nk_label(ctx, "FPS: ", NK_TEXT_LEFT);
			nk_layout_row_static(ctx, 30, 180, 1);
			nk_label(ctx, "Particle Count: ", NK_TEXT_LEFT);
		}
		nk_group_end(ctx);
		nk_spacing(ctx, 1);
		if (nk_group_begin(ctx, "nums", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 30, 80, 1);
			nk_label(ctx, fps_chars, NK_TEXT_CENTERED);
			nk_layout_row_static(ctx, 30, 80, 1);
			nk_label(ctx, p_count_chars, NK_TEXT_CENTERED);
		}
		nk_group_end(ctx);

	}
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
	nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
}


static void
main_layout(struct nk_context* ctx, struct media* media, int width, int height, guiStatus s) {
	ui_statstics(ctx, media, s);
}