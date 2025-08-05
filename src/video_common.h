/*
 * video_common.h
 */


#ifndef __LIBOSW_VIDEO_COMMON_H__
#define __LIBOSW_VIDEO_COMMON_H__

#include "libosw/types.h"

#define OSW_VIDEO_ATTR_TITLE		0
#define OSW_VIDEO_ATTR_FRAME		1

typedef struct VideoState_t {
	u64 target_us;
	u32 flags;

	u32 output_w;
	u32 output_h;
	u32 frame_w;
	u32 frame_h;
	u32 fill_mode;
	u32 filter_mode;
	u32 res_mode;
	u32 max_w;
	u32 max_h;

	u32 frame_output_x;
	u32 frame_output_y;
	u32 frame_output_w;
	u32 frame_output_h;
} VideoState;

extern VideoState vstate;

u32  __osw_VideoInit(u32 flags);
void __osw_VideoExit(void);
void __osw_VideoAttrSet(u32 attr, void *val);
void __osw_VideoPoll(void);
void __osw_VideoSwapBuffers(void);
void __osw_UpdateOutputDims(void);

#endif /*__LIBOSW_VIDEO_COMMON_H__*/
