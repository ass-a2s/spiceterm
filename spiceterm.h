#include <glib.h>
#include <spice.h>

#define IBUFSIZE 1024
#define MAX_ESC_PARAMS 16

typedef struct TextAttributes {
    unsigned int fgcol:4;
    unsigned int bgcol:4;
    unsigned int bold:1;
    unsigned int uline:1;
    unsigned int blink:1;
    unsigned int invers:1;
    unsigned int unvisible:1;
} TextAttributes;

typedef struct TextCell {
    gunichar2 ch;
    TextAttributes attrib;
} TextCell;

#define COMMANDS_SIZE 1024
#define MAX_HEIGHT 2048
#define MAX_WIDTH 2048

typedef struct SpiceScreen SpiceScreen;

struct SpiceScreen {
    SpiceCoreInterface *core;
    SpiceServer *server;

    QXLInstance qxl_instance;
    QXLWorker *qxl_worker;

    uint8_t primary_surface[MAX_HEIGHT * MAX_WIDTH * 4];
    int primary_height;
    int primary_width;

    SpiceTimer *conn_timeout_timer;
    SpiceWatch *mwatch; /* watch master pty */

    int cursor_notify;

    // Current mode (set by create_primary)
    int width;
    int height;

    GCond* command_cond;
    GMutex* command_mutex;

    int commands_end;
    int commands_start;
    struct QXLCommandExt* commands[COMMANDS_SIZE];

    // callbacks
    void (*on_client_connected)(SpiceScreen *spice_screen);
    void (*on_client_disconnected)(SpiceScreen *spice_screen);
};

SpiceScreen* spice_screen_new(SpiceCoreInterface* core, guint timeout);

void spice_screen_draw_char(SpiceScreen *spice_screen, int x, int y, gunichar2 ch, TextAttributes attrib);
void spice_screen_scroll(SpiceScreen *spice_screen, int x1, int y1, int x2, int y2, int src_x, int src_y);
void spice_screen_clear(SpiceScreen *spice_screen, int x1, int y1, int x2, int y2);
uint32_t spice_screen_get_width(void);
uint32_t spice_screen_get_height(void);

typedef struct spiceTerm {
    int maxx;
    int maxy;

    int width;
    int height;

    int total_height;
    int scroll_height;
    int y_base;
    int y_displ;
    int altbuf:1;

    unsigned int utf8:1; // utf8 mode
    long utf_char;       // used by utf8 parser
    int utf_count;       // used by utf8 parser

    TextAttributes default_attrib;

    TextCell *cells;
    TextCell *altcells;

    SpiceScreen *screen;
    SpiceKbdInstance keyboard_sin;
    SpiceCharDeviceInstance vdagent_sin;

    // cursor
    TextAttributes cur_attrib;
    TextAttributes cur_attrib_saved;
    int tty_state; // 0 - normal, 1 - ESC, 2 - CSI
    int cx; // cursor x position
    int cy; // cursor y position
    int cx_saved; // saved cursor x position
    int cy_saved; // saved cursor y position
    int esc_buf[MAX_ESC_PARAMS];
    int esc_count;
    int esc_ques;
    int esc_has_par;
    char osc_textbuf[4096];
    char osc_cmd;
    int region_top;
    int region_bottom;

    unsigned int charset:1; // G0 or G1
    unsigned int charset_saved:1; // G0 or G1
    unsigned int g0enc:2;
    unsigned int g0enc_saved:2;
    unsigned int g1enc:2;
    unsigned int g1enc_saved:2;
    unsigned int cur_enc:2;
    unsigned int cur_enc_saved:2;

    // input buffer
    char ibuf[IBUFSIZE];
    int ibuf_count;

    gunichar2 *selection;
    int selection_len;

    unsigned int mark_active:1;

    unsigned int report_mouse:1;

} spiceTerm;
