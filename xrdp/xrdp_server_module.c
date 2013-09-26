/**
 * xrdp: A Remote Desktop Protocol server.
 *
 * Copyright (C) Jay Sorg 2004-2013
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * module interface
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/stat.h>

#include "xrdp.h"

int freerds_client_inbound_begin_update(rdsModule* mod, XRDP_MSG_BEGIN_UPDATE* msg)
{
	libxrdp_orders_begin_paint(mod->session);
	return 0;
}

int freerds_client_inbound_end_update(rdsModule* mod, XRDP_MSG_END_UPDATE* msg)
{
	libxrdp_orders_end_paint(mod->session);
	return 0;
}

int freerds_client_inbound_beep(rdsModule* mod, XRDP_MSG_BEEP* msg)
{
	libxrdp_send_bell(mod->session);
	return 0;
}

int freerds_client_inbound_is_terminated(rdsModule* mod)
{
	return g_is_term();
}

int freerds_client_inbound_opaque_rect(rdsModule* mod, XRDP_MSG_OPAQUE_RECT* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_screen_blt(rdsModule* mod, XRDP_MSG_SCREEN_BLT* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_paint_rect(rdsModule* mod, XRDP_MSG_PAINT_RECT* msg)
{
	int bpp;
	int inFlightFrames;
	SURFACE_FRAME* frame;

	bpp = msg->framebuffer->fbBitsPerPixel;

	if (mod->session->codecMode)
	{
		inFlightFrames = ListDictionary_Count(mod->session->FrameList);

		if (inFlightFrames > mod->session->settings->FrameAcknowledge)
			mod->fps = (100 / (inFlightFrames + 1) * mod->MaxFps) / 100;
		else
			mod->fps = mod->MaxFps;

		if (mod->fps < 1)
			mod->fps = 1;

		frame = (SURFACE_FRAME*) malloc(sizeof(SURFACE_FRAME));

		frame->frameId = ++mod->session->frameId;
		ListDictionary_Add(mod->session->FrameList, (void*) (size_t) frame->frameId, frame);

		libxrdp_orders_send_frame_marker(mod->session, SURFACECMD_FRAMEACTION_BEGIN, frame->frameId);
		libxrdp_send_surface_bits(mod->session, bpp, msg);
		libxrdp_orders_send_frame_marker(mod->session, SURFACECMD_FRAMEACTION_END, frame->frameId);
	}
	else
	{
		libxrdp_send_bitmap_update(mod->session, bpp, msg);
	}

	return 0;
}

int freerds_client_inbound_patblt(rdsModule* mod, XRDP_MSG_PATBLT* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_dstblt(rdsModule* mod, XRDP_MSG_DSTBLT* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_set_pointer(rdsModule* mod, XRDP_MSG_SET_POINTER* msg)
{
	libxrdp_set_pointer(mod->session, msg);
	return 0;
}

int freerds_client_inbound_set_palette(rdsModule* mod, XRDP_MSG_SET_PALETTE* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_set_clipping_region(rdsModule* mod, XRDP_MSG_SET_CLIPPING_REGION* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_line_to(rdsModule* mod, XRDP_MSG_LINE_TO* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_cache_glyph(rdsModule* mod, XRDP_MSG_CACHE_GLYPH* msg)
{
	return libxrdp_orders_send_font(mod->session, msg);
}

int freerds_client_inbound_glyph_index(rdsModule* mod, XRDP_MSG_GLYPH_INDEX* msg)
{
	/* TODO */

	return 0;
}

int freerds_client_inbound_shared_framebuffer(rdsModule* mod, XRDP_MSG_SHARED_FRAMEBUFFER* msg)
{
	mod->framebuffer.fbWidth = msg->width;
	mod->framebuffer.fbHeight = msg->height;
	mod->framebuffer.fbScanline = msg->scanline;
	mod->framebuffer.fbSegmentId = msg->segmentId;
	mod->framebuffer.fbBitsPerPixel = msg->bitsPerPixel;
	mod->framebuffer.fbBytesPerPixel = msg->bytesPerPixel;

	printf("received shared framebuffer message: mod->framebuffer.fbAttached: %d msg->attach: %d\n",
			mod->framebuffer.fbAttached, msg->attach);

	if (!mod->framebuffer.fbAttached && msg->attach)
	{
		mod->framebuffer.fbSharedMemory = (BYTE*) shmat(mod->framebuffer.fbSegmentId, 0, 0);
		mod->framebuffer.fbAttached = TRUE;

		printf("attached segment %d to %p\n",
				mod->framebuffer.fbSegmentId, mod->framebuffer.fbSharedMemory);

		mod->framebuffer.image = (void*) pixman_image_create_bits(PIXMAN_x8r8g8b8,
				mod->framebuffer.fbWidth, mod->framebuffer.fbHeight,
				(uint32_t*) mod->framebuffer.fbSharedMemory, mod->framebuffer.fbScanline);
	}

	if (mod->framebuffer.fbAttached && !msg->attach)
	{
		shmdt(mod->framebuffer.fbSharedMemory);
		mod->framebuffer.fbAttached = FALSE;
		mod->framebuffer.fbSharedMemory = 0;
	}

	return 0;
}

int freerds_client_inbound_reset(rdsModule* mod, XRDP_MSG_RESET* msg)
{
	if (libxrdp_reset(mod->session, msg) != 0)
		return 0;

	return 0;
}

int freerds_client_inbound_create_offscreen_surface(rdsModule* mod, XRDP_MSG_CREATE_OFFSCREEN_SURFACE* msg)
{
	return 0;
}

int freerds_client_inbound_switch_offscreen_surface(rdsModule* mod, XRDP_MSG_SWITCH_OFFSCREEN_SURFACE* msg)
{
	return 0;
}

int freerds_client_inbound_delete_offscreen_surface(rdsModule* mod, XRDP_MSG_DELETE_OFFSCREEN_SURFACE* msg)
{
	return 0;
}

int freerds_client_inbound_paint_offscreen_surface(rdsModule* mod, XRDP_MSG_PAINT_OFFSCREEN_SURFACE* msg)
{
	return 0;
}

int freerds_client_inbound_window_new_update(rdsModule* mod, XRDP_MSG_WINDOW_NEW_UPDATE* msg)
{
	return libxrdp_window_new_update(mod->session, msg);
}

int freerds_client_inbound_window_delete(rdsModule* mod, XRDP_MSG_WINDOW_DELETE* msg)
{
	return libxrdp_window_delete(mod->session, msg);
}

int freerds_client_inbound_module_init(rdsModule* mod)
{
	if (mod->server)
	{
		mod->server->BeginUpdate = freerds_client_inbound_begin_update;
		mod->server->EndUpdate = freerds_client_inbound_end_update;
		mod->server->Beep = freerds_client_inbound_beep;
		mod->server->IsTerminated = freerds_client_inbound_is_terminated;
		mod->server->OpaqueRect = freerds_client_inbound_opaque_rect;
		mod->server->ScreenBlt = freerds_client_inbound_screen_blt;
		mod->server->PaintRect = freerds_client_inbound_paint_rect;
		mod->server->PatBlt = freerds_client_inbound_patblt;
		mod->server->DstBlt = freerds_client_inbound_dstblt;
		mod->server->SetPointer = freerds_client_inbound_set_pointer;
		mod->server->SetPalette = freerds_client_inbound_set_palette;
		mod->server->SetClippingRegion = freerds_client_inbound_set_clipping_region;
		mod->server->LineTo = freerds_client_inbound_line_to;
		mod->server->CacheGlyph = freerds_client_inbound_cache_glyph;
		mod->server->GlyphIndex = freerds_client_inbound_glyph_index;
		mod->server->SharedFramebuffer = freerds_client_inbound_shared_framebuffer;
		mod->server->Reset = freerds_client_inbound_reset;
		mod->server->CreateOffscreenSurface = freerds_client_inbound_create_offscreen_surface;
		mod->server->SwitchOffscreenSurface = freerds_client_inbound_switch_offscreen_surface;
		mod->server->DeleteOffscreenSurface = freerds_client_inbound_delete_offscreen_surface;
		mod->server->PaintOffscreenSurface = freerds_client_inbound_paint_offscreen_surface;
		mod->server->WindowNewUpdate = freerds_client_inbound_window_new_update;
		mod->server->WindowDelete = freerds_client_inbound_window_delete;
	}

	xrdp_message_server_module_init(mod);

	return 0;
}