#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "etshal.h"
#include "c_types.h"
#include "user_interface.h"
#include "gpio.h"
#include "ps2.h"

#define DEFAULT_CLOCK_PIN 5
#define DEFAULT_DATA_PIN 4

#define BUFFER_SIZE 45
static uint8_t buffer[BUFFER_SIZE];
static uint8_t head = 0, tail = 0;
static int ClockPin = DEFAULT_CLOCK_PIN;
static int DataPin = DEFAULT_DATA_PIN;
static int count = 0;

int ClockMask = 0x00;

static uint8_t get_scan_code();
static bool available();
static char read();
static char peek();
static void clear();

STATIC mp_obj_t ps2_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
  enum { ARG_clock, ARG_data };
  static const mp_arg_t allowed_args[] = {
    { MP_QSTR_clock, MP_ARG_INT, {.u_int = DEFAULT_CLOCK_PIN } },
    { MP_QSTR_data, MP_ARG_INT, {.u_int = DEFAULT_DATA_PIN} },
  };
  mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
  mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
  ClockPin = args[ARG_clock].u_int;
  ClockMask = 1 << ClockPin;
  DataPin = args[ARG_data].u_int;

  ETS_GPIO_INTR_DISABLE();
  // 2 = GPIO_PIN_INTR_NEGEDGE
  GPIO_REG_WRITE(GPIO_PIN_ADDR(ClockPin), (GPIO_REG_READ(GPIO_PIN_ADDR(ClockPin)) & ~GPIO_PIN_INT_TYPE_MASK) | GPIO_PIN_INT_TYPE_SET(2));
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, ClockMask);
  ETS_GPIO_INTR_ENABLE();

  memset(buffer, 0, sizeof buffer);
  head = 0;
  tail = 0;
  count = 0;

  return mp_const_none;
}

STATIC mp_obj_t ps2_get(mp_obj_t what)
{
  int i = mp_obj_get_int(what);
  return mp_obj_new_int(buffer[i & BUFFER_SIZE]);
}

STATIC mp_obj_t ps2_head(void)
{
  return mp_obj_new_int(head);
}

STATIC mp_obj_t ps2_tail(void)
{
  return mp_obj_new_int(tail);
}

STATIC mp_obj_t ps2_count(void)
{
  return mp_obj_new_int(count);
}

STATIC mp_obj_t ps2_clock(void)
{
  return mp_obj_new_int(ClockPin);
}

STATIC mp_obj_t ps2_data(void)
{
  return mp_obj_new_int(DataPin);
}

STATIC mp_obj_t ps2_get_scan_code(void)
{
  uint8_t c = get_scan_code();
  return mp_obj_new_int(c);
}

STATIC mp_obj_t ps2_available(void)
{
  bool ret = available();
  return mp_obj_new_bool(ret);
}

STATIC mp_obj_t ps2_read(void)
{
  char c = read();
  return mp_obj_new_int(c);
}

STATIC mp_obj_t ps2_peek(void)
{
  char c = peek();
  return mp_obj_new_int(c);
}

STATIC mp_obj_t ps2_clear(void)
{
  clear();
  return mp_const_none;
}

STATIC mp_obj_t ps2_readinto(size_t n_args, const mp_obj_t *args)
{
  mp_buffer_info_t bufinfo;
  mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_WRITE);
  mp_uint_t len = bufinfo.len;
  if (n_args > 2) {
    len = mp_obj_get_int(args[2]);
    if (len > bufinfo.len) {
      len = bufinfo.len;
    }
  }
  char *buf = bufinfo.buf;
  int done = 0;
  while (done < len) {
    char c = read();
    if (!c) {
      break;
    }
    buf[done] = c;
    done ++;
  }
  if (done) {
    return mp_obj_new_int(done);
  }
  return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ps2_init_obj, 0, ps2_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ps2_get_obj, ps2_get);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_head_obj, ps2_head);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_tail_obj, ps2_tail);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_count_obj, ps2_count);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_clock_obj, ps2_clock);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_data_obj, ps2_data);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_get_scan_code_obj, ps2_get_scan_code);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_available_obj, ps2_available);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_read_obj, ps2_read);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_peek_obj, ps2_peek);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_clear_obj, ps2_clear);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ps2_readinto_obj, 2, 3, ps2_readinto);

STATIC const mp_map_elem_t ps2_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ps2) },
    { MP_ROM_QSTR(MP_QSTR_init), (mp_obj_t)&ps2_init_obj },
    { MP_ROM_QSTR(MP_QSTR_get), (mp_obj_t)&ps2_get_obj },
    { MP_ROM_QSTR(MP_QSTR_head), (mp_obj_t)&ps2_head_obj },
    { MP_ROM_QSTR(MP_QSTR_tail), (mp_obj_t)&ps2_tail_obj },
    { MP_ROM_QSTR(MP_QSTR_count), (mp_obj_t)&ps2_count_obj },
    { MP_ROM_QSTR(MP_QSTR_clock), (mp_obj_t)&ps2_clock_obj },
    { MP_ROM_QSTR(MP_QSTR_data), (mp_obj_t)&ps2_data_obj },
    { MP_ROM_QSTR(MP_QSTR_get_scan_code), (mp_obj_t)&ps2_get_scan_code_obj },
    { MP_ROM_QSTR(MP_QSTR_available), (mp_obj_t)&ps2_available_obj },
    { MP_ROM_QSTR(MP_QSTR_read), (mp_obj_t)&ps2_read_obj },
    { MP_ROM_QSTR(MP_QSTR_peek), (mp_obj_t)&ps2_peek_obj },
    { MP_ROM_QSTR(MP_QSTR_clear), (mp_obj_t)&ps2_clear_obj },
    { MP_ROM_QSTR(MP_QSTR_read), (mp_obj_t)&ps2_read_obj },
    { MP_ROM_QSTR(MP_QSTR_readinto), (mp_obj_t)&ps2_readinto_obj },
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_ps2_globals,
    ps2_globals_table
);

const mp_obj_module_t mp_module_ps2 = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_ps2_globals,
};

void
ps2_callback()
{
  static uint8_t bitcount = 0;
  static uint8_t incoming = 0;
  static uint32_t prev_ms = 0;
  uint32_t now_ms;
  uint8_t n;
  uint8_t val;

  count ++;
  val = GPIO_INPUT_GET(DataPin);
  now_ms = system_get_time() / 1000;
  if (now_ms - prev_ms > 250) {
    bitcount = 0;
    incoming = 0;
  }
  prev_ms = now_ms;
  n = bitcount - 1;
  if (n <= 7) {
    incoming |= (val << n);
  }
  bitcount++;
  if (bitcount == 11) {
    uint8_t i = head + 1;
    if (i >= BUFFER_SIZE) {
      i = 0;
    }
    if (i != tail) {
      buffer[i] = incoming;
      head = i;
    }
    bitcount = 0;
    incoming = 0;
  }
}

static uint8_t
get_scan_code()
{
  uint8_t c;
  uint8_t i;

  i = tail;
  if (i == head) {
    return 0;
  }
  i ++;
  if (i >= BUFFER_SIZE) {
    i = 0;
  }
  c = buffer[i];
  tail = i;
  return c;
}

#define PS2_TAB				9
#define PS2_ENTER			13
#define PS2_BACKSPACE			127
#define PS2_ESC				27
#define PS2_INSERT			0
#define PS2_DELETE			127
#define PS2_HOME			0
#define PS2_END				0
#define PS2_PAGEUP			25
#define PS2_PAGEDOWN			26
#define PS2_UPARROW			11
#define PS2_LEFTARROW			8
#define PS2_DOWNARROW			10
#define PS2_RIGHTARROW			21
#define PS2_F1				0
#define PS2_F2				0
#define PS2_F3				0
#define PS2_F4				0
#define PS2_F5				0
#define PS2_F6				0
#define PS2_F7				0
#define PS2_F8				0
#define PS2_F9				0
#define PS2_F10				0
#define PS2_F11				0
#define PS2_F12				0
#define PS2_SCROLL			0

#define PS2_KEYMAP_SIZE 136

typedef struct {
  uint8_t noshift[PS2_KEYMAP_SIZE];
  uint8_t shift[PS2_KEYMAP_SIZE];
  unsigned int uses_altgr;
  uint8_t altgr[PS2_KEYMAP_SIZE];
} PS2Keymap_t;

const PS2Keymap_t PS2Keymap_US = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
   0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '`', 0,
   0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
   0, 0, 'z', 's', 'a', 'w', '2', 0,
   0, 'c', 'x', 'd', 'e', '4', '3', 0,
   0, ' ', 'v', 'f', 't', 'r', '5', 0,
   0, 'n', 'b', 'h', 'g', 'y', '6', 0,
   0, 0, 'm', 'j', 'u', '7', '8', 0,
   0, ',', 'k', 'i', 'o', '0', '9', 0,
   0, '.', '/', 'l', ';', 'p', '-', 0,
   0, 0, '\'', 0, '[', '=', 0, 0,
   0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, '\\', 0, 0,
   0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
   0, '1', 0, '4', '7', 0, 0, 0,
   '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
   PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
   0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
   0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '~', 0,
   0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
   0, 0, 'Z', 'S', 'A', 'W', '@', 0,
   0, 'C', 'X', 'D', 'E', '$', '#', 0,
   0, ' ', 'V', 'F', 'T', 'R', '%', 0,
   0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
   0, 0, 'M', 'J', 'U', '&', '*', 0,
   0, '<', 'K', 'I', 'O', ')', '(', 0,
   0, '>', '?', 'L', ':', 'P', '_', 0,
   0, 0, '"', 0, '{', '+', 0, 0,
   0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '}', 0, '|', 0, 0,
   0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
   0, '1', 0, '4', '7', 0, 0, 0,
   '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
   PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
   0, 0, 0, PS2_F7 },
  0
};

const PS2Keymap_t *keymap = &PS2Keymap_US;
static uint8_t CharBuffer=0;

#define BREAK     0x01
#define MODIFIER  0x02
#define SHIFT_L   0x04
#define SHIFT_R   0x08
#define ALTGR     0x10

static char
get_iso8859_code()
{
  static uint8_t state = 0;
  uint8_t s;
  char c;

  while (1) {
    s = get_scan_code();
    if (!s) {
      return 0;
    }
    if (s == 0xF0) {
      state |= BREAK;
    } else if (s == 0xE0) {
      state |= MODIFIER;
    } else {
      if (state & BREAK) {
	if (s == 0x12) {
	  state &= ~SHIFT_L;
	} else if (s == 0x59) {
	  state &= ~SHIFT_R;
	} else if (s == 0x11 && (state & MODIFIER)) {
	  state &= ~ALTGR;
	}
	state &= ~(BREAK | MODIFIER);
	continue;
      }
      if (s == 0x12) {
	state |= SHIFT_L;
	continue;
      } else if (s == 0x59) {
	state |= SHIFT_R;
	continue;
      } else if (s == 0x11 && (state & MODIFIER)) {
	state |= ALTGR;
      }
      c = 0;
      if (state & MODIFIER) {
	switch (s) {
	case 0x70: c = PS2_INSERT;      break;
	case 0x6C: c = PS2_HOME;        break;
	case 0x7D: c = PS2_PAGEUP;      break;
	case 0x71: c = PS2_DELETE;      break;
	case 0x69: c = PS2_END;         break;
	case 0x7A: c = PS2_PAGEDOWN;    break;
	case 0x75: c = PS2_UPARROW;     break;
	case 0x6B: c = PS2_LEFTARROW;   break;
	case 0x72: c = PS2_DOWNARROW;   break;
	case 0x74: c = PS2_RIGHTARROW;  break;
	case 0x4A: c = '/';             break;
	case 0x5A: c = PS2_ENTER;       break;
	default: break;
	}
      } else if ((state & ALTGR) && keymap->uses_altgr) {
	if (s < PS2_KEYMAP_SIZE)
	  c = keymap->altgr[s];
      } else if (state & (SHIFT_L | SHIFT_R)) {
	if (s < PS2_KEYMAP_SIZE)
	  c = keymap->shift[s];
      } else {
	if (s < PS2_KEYMAP_SIZE)
	  c = keymap->noshift[s];
      }
      state &= ~(BREAK | MODIFIER);
      if (c) {
	return c;
      }
    }
  }
}

static bool
available()
{
  if (CharBuffer) {
    return true;
  }
  CharBuffer = get_iso8859_code();
  return CharBuffer != 0;
}

static char
read()
{
  if (CharBuffer) {
    char c = CharBuffer;
    CharBuffer = 0;
    return c;
  }
  return get_iso8859_code();
}

static char
peek()
{
  if (CharBuffer) {
    return CharBuffer;
  }
  CharBuffer = get_iso8859_code();
  return CharBuffer;
}

static void
clear()
{
  CharBuffer = 0;
}
