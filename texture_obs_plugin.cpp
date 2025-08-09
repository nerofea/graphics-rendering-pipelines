// obs plug in way
gs_texture_t* tex = gs_texture_create(width, height, GS_RGBA, 1, 
                                        (const uint8_t**)&imgBuffer,
                                        GY_DYNAMIC);

gs_effect_t* effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
gs_effect_get_texture(gs_effect_get_param_by_name(effect, "image"), tex);

while (gs_effect_loop(effect, "Draw")) {
    gs_draw_sprite(tex, 0, width, height);
}