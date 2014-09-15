#include <uae/uae.h>
#include <fs/config.h>
#include <fs/emu.h>
#include <fs/i18n.h>
#include <glib.h>
#include "options.h"
#include "config-graphics.h"

static bool check_card(amiga_config *c, const char **card, int *memory,
                       const char *check, const char *z2, int m2,
                       const char *z3, int m3)
{
    int z = 0;
    char check2[32];
    if (strcasecmp(*card, check) == 0) {
        /* Auto-select Zorro II / Zorro III */
        z = 1;
    }
    strcpy(check2, check);
    strcat(check2, "-z2");
    if (strcasecmp(*card, check2) == 0) {
        z = 2;
        if (!z2) {
            return false;
        }
    }
    strcpy(check2, check);
    strcat(check2, "-z3");
    if (strcasecmp(*card, check2) == 0) {
        z = 3;
        if (!z3) {
            return false;
        }
    }
    if (z == 0) {
        /* No match */
        return false;
    }
    if (z == 1) {
        if (c->allow_z3_memory) {
            z = z3 ? 3 : 2;
        }
        else {
            z = z2 ? 2 : 3;
        }
    }
    if (z == 3) {
        /* Configure Zorro III card with up to 16 MB by default */
        *card = z3;
        *memory = MIN(16, m3);
    }
    else {
        /* Configure Zorro II card with up to 4 MB by default */
        *card = z2;
        *memory = MIN(4, m2);
    }
    return true;
}

//#define CHECK_CARD(check, z2, z3) card = check_card(c, card, check, z2, z3);
#define CHECK_CARD(check, z2, m2, z3, m3) \
if (card && !found) { \
    found = check_card(c, &card, &memory, check, z2, m2, z3, m3); \
}

void fs_uae_configure_graphics_card(amiga_config *c)
{
    const char *card = NULL;
    int memory = 0;
    bool found = false;

    if (fs_config_get_const_string(OPTION_GRAPHICS_CARD)) {
        card = fs_config_get_const_string(OPTION_GRAPHICS_CARD);
    }
    else {
        int uaegfx_card = fs_config_get_boolean(OPTION_UAEGFX_CARD);
        if (uaegfx_card != FS_CONFIG_NONE) {
            fs_log("DEPRECATED: uaegfx_card is deprecated, use graphics_card "
                   "instead\n");
            if (uaegfx_card == 1) {
                if (!c->allow_z3_memory) {
                    fs_emu_warning(_("Option uaegfx.card needs a CPU with "
                                     "32-bit addressing\n"));
                }
                else {
                    card = "uaegfx-z3";
                    memory = 32;
                    found = true;
                }
            }
        }
    }

    CHECK_CARD("uaegfx", "ZorroII", 8, "ZorroIII", 512)
    CHECK_CARD("picasso-ii", "PicassoII", 2, NULL, 0)
    CHECK_CARD("picasso-ii+", "PicassoII+", 2, NULL, 0)
    CHECK_CARD("picasso-iv", "PicassoIV_Z2", 4, "PicassoIV_Z3", 4)

    if (card && !found) {
        fs_emu_warning("Unsupported graphics card: %s\n", card);
    }

    if (fs_config_get_int(OPTION_GRAPHICS_MEMORY) != FS_CONFIG_NONE) {
        memory = fs_config_get_int_clamped(OPTION_GRAPHICS_MEMORY, 0, 512);
        fs_log("CONFIG: Overriding graphics card memory: %d MB\n", memory);
    }

    if (card != NULL) {
        if (memory != 0) {
            amiga_set_option("gfxcard_type", card);
            amiga_set_int_option("gfxcard_size", memory);
        }
    }
}