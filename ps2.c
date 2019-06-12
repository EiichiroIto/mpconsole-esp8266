#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "etshal.h"
#include "c_types.h"
#include "user_interface.h"
#include "gpio.h"

#define DEFAULT_CLOCK_PIN 5
#define DEFAULT_DATA_PIN 4

#define BUFFER_SIZE 45
static uint8_t buffer[BUFFER_SIZE];
static uint8_t head = 0, tail = 0;
static int ClockPin = DEFAULT_CLOCK_PIN;
static int DataPin = DEFAULT_DATA_PIN;
static uint8_t bitcount = 0;
static uint8_t incoming = 0;
static uint32_t prev_ms = 0;
static int count = 0;

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
  DataPin = args[ARG_data].u_int;

  memset(buffer, 0, sizeof buffer);
  head = 0;
  tail = 0;
  bitcount = 0;
  incoming = 0;
  prev_ms = 0;
  count = 0;

  ETS_GPIO_INTR_DISABLE();
  // 2 = GPIO_PIN_INTR_NEGEDGE
  GPIO_REG_WRITE(GPIO_PIN_ADDR(ClockPin), (GPIO_REG_READ(GPIO_PIN_ADDR(ClockPin)) & ~GPIO_PIN_INT_TYPE_MASK) | GPIO_PIN_INT_TYPE_SET(2));
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << ClockPin);
  ETS_GPIO_INTR_ENABLE();
  return mp_const_none;
}

void ps2_callback_body()
{
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
  uint8_t c;
  uint8_t i;

  i = tail;
  if (i == head) {
    return mp_obj_new_int(0);
  }
  i ++;
  if (i >= BUFFER_SIZE) {
    i = 0;
  }
  c = buffer[i];
  tail = i;
  return mp_obj_new_int(c);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ps2_init_obj, 0, ps2_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ps2_get_obj, ps2_get);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_head_obj, ps2_head);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_tail_obj, ps2_tail);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_count_obj, ps2_count);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_clock_obj, ps2_clock);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_data_obj, ps2_data);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ps2_get_scan_code_obj, ps2_get_scan_code);

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
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_ps2_globals,
    ps2_globals_table
);

const mp_obj_module_t mp_module_ps2 = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_ps2_globals,
};
