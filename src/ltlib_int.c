
//#include <stdarg.h>
#include <pthread.h>
#include "pref_global.h"
#include "utils.h"
#include "pref.h"
#include "cal.h"
#include "tracking.h"
#include "ltlib_int.h"

static pthread_t cal_thread;
static ltr_new_frame_callback_t ltr_new_frame_cbk = NULL;
static void *ltr_new_frame_cbk_param = NULL;

static bool publish_frames = true;
static int active_frame = 0;
static unsigned int current_w = 0;
static unsigned int current_h = 0;
static struct mmap_s mmap;
static int data_size = 0;
static int extra_data = 3 * sizeof(uint32_t);

static int inc_frame_index(int current)
{
  return (current + 1) % FRAME_BUFFERS;
}

static void publish_frame(struct frame_type *frame)
{
//   if(frame->bitmap == NULL){
//     printf("No bitmap to copy!\n");
//
//     return;
//   }
  if((frame->width != current_w) || (frame->height != current_h)){
    active_frame = 0;
    if((frame->width * frame->height) > (current_w * current_h)){
      ltr_int_unmap_file(&mmap);
      data_size = FRAME_BUFFERS * frame->width * frame->height + extra_data;
      char *fname = ltr_int_get_default_file_name("frames.dat");
      if(!ltr_int_mmap_file(fname, data_size, &mmap)){
        free(fname);
        return;
      }
      free(fname);
    }
    current_w = frame->width;
    current_h = frame->height;
    *(uint8_t*)mmap.data = 0;
  }
  uint32_t *data = (uint32_t*)mmap.data;
  uint32_t *flag = data;
  uint32_t *w = data + 1;
  uint32_t *h = data + 2;
  uint8_t *buf_base = ((uint8_t*)mmap.data) + extra_data;
  int frame_size = frame->width * frame->height;
  //advance the flag to point to the current buffer
  int current_index = inc_frame_index(*flag);
  uint8_t *current_buf = buf_base + (current_index) * frame_size;

  *w = frame->width;
  *h = frame->height;

  if(frame->bitmap == current_buf){
    int next_index = inc_frame_index(current_index);
    frame->bitmap = buf_base + next_index * frame_size;
    *flag = current_index;
    memset(frame->bitmap, 0, frame->width * frame->height);
  }else if(frame->bitmap == NULL){
    *flag = 0;
    memset(buf_base, 0, frame->width * frame->height);
    frame->bitmap = buf_base + frame_size;
    memset(frame->bitmap, 0, frame->width * frame->height);
  }else{
    //someone else has supplied the frame
    memcpy(current_buf, frame->bitmap, current_w * current_h);
    *flag = current_index;
  }
}

void ltr_int_publish_frames_cmd(void){
  ltr_int_log_message("Received request to publish frames\n");
  publish_frames = true;
}

static int frame_callback(struct camera_control_block *ccb, struct frame_type *frame)
{
  (void)ccb;
  ltr_int_update_pose(frame);
  if(publish_frames){
    publish_frame(frame);
  }
  if(ltr_new_frame_cbk != NULL){
    ltr_new_frame_cbk(frame, ltr_new_frame_cbk_param);
  }
  return 0;
}

static struct camera_control_block ccb;

static void *cal_thread_fun(void *param)
{
  (void)param;
  if(ltr_int_get_device(&ccb)){
    ccb.diag = false;
    ltr_int_cal_run(&ccb, frame_callback);
    free(ccb.device.device_id);
  }else{
    ltr_int_log_message("Couldn't get the device!\n");
  }
  //pthread_detach(pthread_self());
  return NULL;
}

int ltr_int_init(void)
{
  if(!ltr_int_read_prefs(NULL, false)){
    ltr_int_log_message("Couldn't load preferences!\n");
    return -1;
  }
  if(!ltr_int_init_tracking()){
    ltr_int_log_message("Couldn't initialize tracking!\n");
    return -1;
  }
  pthread_create(&cal_thread, NULL, cal_thread_fun, NULL);
  return 0;
}

int ltr_int_get_camera_update(linuxtrack_full_pose_t *pose)
{
  return ltr_int_tracking_get_pose(pose);
}

void ltr_int_register_cbk(ltr_new_frame_callback_t new_frame_cbk, void *param1,
                          ltr_status_update_callback_t status_change_cbk, void *param2)
{
  ltr_new_frame_cbk = new_frame_cbk;
  ltr_new_frame_cbk_param = param1;
  ltr_int_set_status_change_cbk(status_change_cbk, param2);
}

int ltr_int_suspend(void)
{
  return ltr_int_cal_suspend();
}

int ltr_int_wakeup(void)
{
  return ltr_int_cal_wakeup();
}

int ltr_int_shutdown(void)
{
  ltr_int_log_message("Shutting down tracking...\n");
  int res = ltr_int_cal_shutdown();
  pthread_join(cal_thread, NULL);
  return res;
}

void ltr_int_recenter(void)
{
  ltr_int_recenter_tracking();
}

linuxtrack_state_type ltr_int_get_tracking_state(void)
{
  return ltr_int_cal_get_state();
}

