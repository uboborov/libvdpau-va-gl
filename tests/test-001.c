// Create two output surfaces (B8G8R8A8) of 4x4, fill first with opaque black
// and second with black and one red dot (opaque too).
// Render second into first. Check that red dot do not gets smoothed.

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vdpau/vdpau.h>
#include "vdpau-init.h"


int main(void)
{
    VdpDevice device;
    VdpStatus st = vdpau_init_functions(&device);
    assert (VDP_STATUS_OK == st);

    VdpOutputSurface out_surface_1;
    VdpOutputSurface out_surface_2;

    ASSERT_OK(vdp_output_surface_create(device, VDP_RGBA_FORMAT_B8G8R8A8, 4, 4, &out_surface_1));
    ASSERT_OK(vdp_output_surface_create(device, VDP_RGBA_FORMAT_B8G8R8A8, 4, 4, &out_surface_2));

    uint32_t black_box[] = {
        0xff000000, 0xff000000, 0xff000000, 0xff000000,
        0xff000000, 0xff000000, 0xff000000, 0xff000000,
        0xff000000, 0xff000000, 0xff000000, 0xff000000,
        0xff000000, 0xff000000, 0xff000000, 0xff000000
    };

    uint32_t one_red_dot[] = {
        0xff000000, 0xff000000, 0xff000000, 0xff000000,
        0xff000000, 0xffff0000, 0xff000000, 0xff000000,
        0xff000000, 0xff000000, 0xff000000, 0xff000000,
        0xff000000, 0xff000000, 0xff000000, 0xff000000
    };

    const void * const source_data_1[] = {black_box};
    const void * const source_data_2[] = {one_red_dot};
    uint32_t source_pitches[] = { 4 * 4 };

    // upload data
    ASSERT_OK(vdp_output_surface_put_bits_native(out_surface_1, source_data_1, source_pitches, NULL));
    ASSERT_OK(vdp_output_surface_put_bits_native(out_surface_2, source_data_2, source_pitches, NULL));

    // render
    VdpOutputSurfaceRenderBlendState blend_state = {
        .struct_version = VDP_OUTPUT_SURFACE_RENDER_BLEND_STATE_VERSION,
        .blend_factor_source_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE,
        .blend_factor_source_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE,
        .blend_factor_destination_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ZERO,
        .blend_factor_source_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE,
        .blend_equation_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD,
        .blend_equation_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD,
        .blend_constant = {0, 0, 0, 0}
    };

    ASSERT_OK(vdp_output_surface_render_output_surface(out_surface_1, NULL, out_surface_2, NULL,
                NULL, &blend_state, VDP_OUTPUT_SURFACE_RENDER_ROTATE_0));

    // get data back
    uint32_t receive_buf[16];
    void * const dest_data[] = {receive_buf};
    ASSERT_OK(vdp_output_surface_get_bits_native(out_surface_1, NULL, dest_data, source_pitches));

    for (int k = 0; k < 16; k ++) {
        printf("%x ", receive_buf[k]);
        if (3 == k % 4) printf("\n");
    }
    printf("----------\n");
    for (int k = 0; k < 16; k ++) {
        printf("%x ", one_red_dot[k]);
        if (3 == k % 4) printf("\n");
    }

    // compare recieve_buf with one_red_dot
    if (memcpy(receive_buf, one_red_dot, 4*4*4)) {
        // not equal
        return 1;
    }

    printf("pass.\n");
    return 0;
}
