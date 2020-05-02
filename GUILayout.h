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
	if (nk_begin(ctx, "STATISTICS", nk_rect(10, 10, 300, 150),
		NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		static const float lbratio[] = { 0.6f, 0.1f, 0.3f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, 45, 3, lbratio);
		nk_label(ctx, "Particle Count: ", NK_TEXT_LEFT);
		nk_spacing(ctx, 1);
		// username & points
		nk_label(ctx, p_count_chars, NK_TEXT_CENTERED);

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