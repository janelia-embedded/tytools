/*
 * ty, a collection of GUI and command-line tools to manage Teensy devices
 *
 * Distributed under the MIT license (see LICENSE.txt or http://opensource.org/licenses/MIT)
 * Copyright (c) 2015 Niels Martignène <niels.martignene@gmail.com>
 */

#ifndef TY_BOARD_H
#define TY_BOARD_H

#include "common.h"

TY_C_BEGIN

struct ty_descriptor_set;
struct tyd_device;
struct tyb_monitor;
struct tyb_firmware;
struct tyd_handle;
struct ty_task;

typedef struct tyb_monitor tyb_monitor;

typedef struct tyb_board tyb_board;
typedef struct tyb_board_interface tyb_board_interface;

typedef struct tyb_board_family tyb_board_family;
typedef struct tyb_board_model tyb_board_model;

// Keep in sync with capability_names in board.c
typedef enum tyb_board_capability {
    TYB_BOARD_CAPABILITY_RUN,
    TYB_BOARD_CAPABILITY_UPLOAD,
    TYB_BOARD_CAPABILITY_RESET,
    TYB_BOARD_CAPABILITY_REBOOT,
    TYB_BOARD_CAPABILITY_SERIAL,

    TYB_BOARD_CAPABILITY_COUNT
} tyb_board_capability;

typedef enum tyb_board_state {
    TYB_BOARD_STATE_DROPPED,
    TYB_BOARD_STATE_MISSING,
    TYB_BOARD_STATE_ONLINE
} tyb_board_state;

enum {
    TYB_UPLOAD_WAIT = 1,
    TYB_UPLOAD_NORESET = 2,
    TYB_UPLOAD_NOCHECK = 4
};

#define TYB_UPLOAD_MAX_FIRMWARES 256

typedef int tyb_board_family_list_models_func(const tyb_board_model *model, void *udata);
typedef int tyb_board_list_interfaces_func(tyb_board_interface *iface, void *udata);
typedef int tyb_board_upload_progress_func(const tyb_board *board, const struct tyb_firmware *fw, size_t uploaded, void *udata);

TY_PUBLIC extern const tyb_board_family *tyb_board_families[];

TY_PUBLIC const char *tyb_board_family_get_name(const tyb_board_family *family);
TY_PUBLIC int tyb_board_family_list_models(const tyb_board_family *family, tyb_board_family_list_models_func *f, void *udata);

TY_PUBLIC bool tyb_board_model_is_real(const tyb_board_model *model);
TY_PUBLIC bool tyb_board_model_test_firmware(const tyb_board_model *model, const struct tyb_firmware *fw,
                                             const tyb_board_model **rguesses, unsigned int *rcount);

TY_PUBLIC const char *tyb_board_model_get_name(const tyb_board_model *model);
TY_PUBLIC const char *tyb_board_model_get_mcu(const tyb_board_model *model);
TY_PUBLIC size_t tyb_board_model_get_code_size(const tyb_board_model *model);

TY_PUBLIC const char *tyb_board_capability_get_name(tyb_board_capability cap);

TY_PUBLIC tyb_board *tyb_board_ref(tyb_board *board);
TY_PUBLIC void tyb_board_unref(tyb_board *board);

TY_PUBLIC bool tyb_board_matches_tag(tyb_board *board, const char *id);

TY_PUBLIC void tyb_board_set_udata(tyb_board *board, void *udata);
TY_PUBLIC void *tyb_board_get_udata(const tyb_board *board);

TY_PUBLIC struct tyb_monitor *tyb_board_get_manager(const tyb_board *board);

TY_PUBLIC tyb_board_state tyb_board_get_state(const tyb_board *board);

TY_PUBLIC const char *tyb_board_get_id(const tyb_board *board);
TY_PUBLIC int tyb_board_set_tag(tyb_board *board, const char *tag);
TY_PUBLIC const char *tyb_board_get_tag(const tyb_board *board);

TY_PUBLIC const char *tyb_board_get_location(const tyb_board *board);
TY_PUBLIC uint64_t tyb_board_get_serial_number(const tyb_board *board);

TY_PUBLIC const tyb_board_model *tyb_board_get_model(const tyb_board *board);
TY_PUBLIC const char *tyb_board_get_model_name(const tyb_board *board);

TY_PUBLIC int tyb_board_list_interfaces(tyb_board *board, tyb_board_list_interfaces_func *f, void *udata);
TY_PUBLIC int tyb_board_open_interface(tyb_board *board, tyb_board_capability cap, tyb_board_interface **riface);

TY_PUBLIC int tyb_board_get_capabilities(const tyb_board *board);
static inline bool tyb_board_has_capability(const tyb_board *board, tyb_board_capability cap)
{
    return tyb_board_get_capabilities(board) & (1 << cap);
}

TY_PUBLIC int tyb_board_wait_for(tyb_board *board, tyb_board_capability capability, int timeout);

TY_PUBLIC int tyb_board_serial_set_attributes(tyb_board *board, uint32_t rate, int flags);
TY_PUBLIC ssize_t tyb_board_serial_read(tyb_board *board, char *buf, size_t size, int timeout);
TY_PUBLIC ssize_t tyb_board_serial_write(tyb_board *board, const char *buf, size_t size);

TY_PUBLIC int tyb_board_upload(tyb_board *board, struct tyb_firmware *fw, tyb_board_upload_progress_func *pf, void *udata);
TY_PUBLIC int tyb_board_reset(tyb_board *board);
TY_PUBLIC int tyb_board_reboot(tyb_board *board);

TY_PUBLIC tyb_board_interface *tyb_board_interface_ref(tyb_board_interface *iface);
TY_PUBLIC void tyb_board_interface_unref(tyb_board_interface *iface);
TY_PUBLIC int tyb_board_interface_open(tyb_board_interface *iface);
TY_PUBLIC void tyb_board_interface_close(tyb_board_interface *iface);

TY_PUBLIC const char *tyb_board_interface_get_name(const tyb_board_interface *iface);
TY_PUBLIC int tyb_board_interface_get_capabilities(const tyb_board_interface *iface);

TY_PUBLIC uint8_t tyb_board_interface_get_interface_number(const tyb_board_interface *iface);
TY_PUBLIC const char *tyb_board_interface_get_path(const tyb_board_interface *iface);

TY_PUBLIC struct tyd_device *tyb_board_interface_get_device(const tyb_board_interface *iface);
TY_PUBLIC struct tyd_handle *tyb_board_interface_get_handle(const tyb_board_interface *iface);
TY_PUBLIC void tyb_board_interface_get_descriptors(const tyb_board_interface *iface, struct ty_descriptor_set *set, int id);

TY_PUBLIC int tyb_upload(tyb_board *board, struct tyb_firmware **fws, unsigned int fws_count,
                         int flags, struct ty_task **rtask);
TY_PUBLIC int tyb_reset(tyb_board *board, struct ty_task **rtask);
TY_PUBLIC int tyb_reboot(tyb_board *board, struct ty_task **rtask);

TY_C_END

#endif
