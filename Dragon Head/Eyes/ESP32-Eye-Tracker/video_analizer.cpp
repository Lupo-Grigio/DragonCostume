#include "esp_camera.h"
#include "img_converters.h"
#include "camera_index.h"
#include "Arduino.h"

#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "video_analizer.h"
#include "FaceReporting.h"


static mtmn_config_t mtmn_config = {0};

static int8_t is_enrolling = 0;

typedef struct {
        size_t size; //number of values used for filtering
        size_t index; //current value index
        size_t count; //value count
        int sum;
        int * values; //array to be filled with values
} ra_filter_t;

static ra_filter_t ra_filter;

static ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size){
    memset(filter, 0, sizeof(ra_filter_t));

    filter->values = (int *)malloc(sample_size * sizeof(int));
    if(!filter->values){
        return NULL;
    }
    memset(filter->values, 0, sample_size * sizeof(int));

    filter->size = sample_size;
    return filter;
}

static int ra_filter_run(ra_filter_t * filter, int value){
    if(!filter->values){
        return value;
    }
    filter->sum -= filter->values[filter->index];
    filter->values[filter->index] = value;
    filter->sum += filter->values[filter->index];
    filter->index++;
    filter->index = filter->index % filter->size;
    if (filter->count < filter->size) {
        filter->count++;
    }
    return filter->sum / filter->count;
}



void VideoSetup()
{
    ra_filter_init(&ra_filter, 20);
    mtmn_config.type = FAST;
    mtmn_config.min_face = 80;
    mtmn_config.pyramid = 0.707;
    mtmn_config.pyramid_times = 4;
    mtmn_config.p_threshold.score = 0.6;
    mtmn_config.p_threshold.nms = 0.7;
    mtmn_config.p_threshold.candidate_number = 20;
    mtmn_config.r_threshold.score = 0.7;
    mtmn_config.r_threshold.nms = 0.7;
    mtmn_config.r_threshold.candidate_number = 10;
    mtmn_config.o_threshold.score = 0.7;
    mtmn_config.o_threshold.nms = 0.7;
    mtmn_config.o_threshold.candidate_number = 1;

    // set up the camera, Frame Size do the flip, and the mirror, and set wB to high etc. 
    // If this code looks wierd, it is because I'm using the same format as the ESP32-CAMERA demo
    // so that if you are looking at this, coming from that demo, this all looks familiar

    sensor_t * s = esp_camera_sensor_get();
    int res = 0;
    int val = 0;

    // res = s->set_framesize(s, (framesize_t)val);
    // res = s->set_quality(s, val);
    // res = s->set_contrast(s, val);
    // res = s->set_brightness(s, val);
    // res = s->set_saturation(s, val);
    // res = s->set_gainceiling(s, (gainceiling_t)val);
    // res = s->set_colorbar(s, val);
    // res = s->set_whitebal(s, val);
    // res = s->set_gain_ctrl(s, val);
    // res = s->set_exposure_ctrl(s, val);
    res = s->set_hmirror(s, H_MIRROR);
    res = s->set_vflip(s, V_FLIP);
    // res = s->set_awb_gain(s, val);
    // res = s->set_agc_gain(s, val);
    // res = s->set_aec_value(s, val);
    // res = s->set_aec2(s, val);
    // res = s->set_dcw(s, val);
    // res = s->set_bpc(s, val);
    // res = s->set_wpc(s, val);
    // res = s->set_raw_gma(s, val);
    // res = s->set_lenc(s, val);
    // res = s->set_special_effect(s, val);
    // res = s->set_wb_mode(s, val);
    // res = s->set_ae_level(s, val);

    static char json_response[1024];
    char * p = json_response;
    *p++ = '{';

    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p+=sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
    p+=sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
    p+=sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
    p+=sprintf(p, "\"awb\":%u,", s->status.awb);
    p+=sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
    p+=sprintf(p, "\"aec\":%u,", s->status.aec);
    p+=sprintf(p, "\"aec2\":%u,", s->status.aec2);
    p+=sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p+=sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p+=sprintf(p, "\"agc\":%u,", s->status.agc);
    p+=sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p+=sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
    p+=sprintf(p, "\"bpc\":%u,", s->status.bpc);
    p+=sprintf(p, "\"wpc\":%u,", s->status.wpc);
    p+=sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
    p+=sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p+=sprintf(p, "\"vflip\":%u,", s->status.vflip);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p+=sprintf(p, "\"dcw\":%u,", s->status.dcw);
    p+=sprintf(p, "\"colorbar\":%u,", s->status.colorbar);
    *p++ = '}';
    *p++ = 0;
    Serial.printf("\n Camera Configuration : %s\n",p);
    

}

void FindFaces()
{
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];
    dl_matrix3du_t *image_matrix = NULL;
    bool detected = false;
    int face_id = 0;
    int face_x = 0;
    int face_y = 0;
    int64_t fr_start = 0;
    int64_t fr_ready = 0;
    int64_t fr_face = 0;
    int64_t fr_recognize = 0;
    int64_t fr_encode = 0;

    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    //while(true){
        detected = false;
        face_id = 0;
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            fr_start = esp_timer_get_time();
            fr_ready = fr_start;
            fr_face = fr_start;
            fr_encode = fr_start;
            fr_recognize = fr_start;
            if(fb->width > 400){
                Serial.println("Rezolution width over 400 can't face detect");
                delay(1000);
            } else {

                image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

                if (!image_matrix) {
                    Serial.println("dl_matrix3du_alloc failed");
                    res = ESP_FAIL;
                } else {
                    if(!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)){
                        Serial.println("fmt2rgb888 failed");
                        res = ESP_FAIL;
                    } else {
                        fr_ready = esp_timer_get_time();
                        box_array_t *net_boxes = NULL;
                        
                        net_boxes = face_detect(image_matrix, &mtmn_config);
                        
                        fr_face = esp_timer_get_time();
                        fr_recognize = fr_face; // TODO: Delte this
                        if (net_boxes || fb->format != PIXFORMAT_JPEG){
                            if(net_boxes){
                                detected = true;
                                /*
                                 * I could just use the x and y coords from the box
                                 * or I could calculate the center of the box and send that
                                 * but the first landmark is (usually) one of the eyes, and 
                                 * you really want to "look" at the eyes
                                 */
                                SetFaceFoundAt((int)net_boxes->landmark[0].landmark_p[0],
                                               (int)net_boxes->landmark[0].landmark_p[1],
                                               image_matrix->w,
                                               image_matrix->h
                                               );
                                                   
                                /*face_x = (int)boxes->box[0].box_p[0];
                                face_y = (int)boxes->box[0].box_p[1];*/
                                face_x = (int)net_boxes->landmark[0].landmark_p[0];
                                face_y = (int)net_boxes->landmark[0].landmark_p[1];
                                dl_lib_free(net_boxes->score);
                                dl_lib_free(net_boxes->box);
                                dl_lib_free(net_boxes->landmark);
                                dl_lib_free(net_boxes);
                            }
                            if(!fmt2jpg(image_matrix->item, fb->width*fb->height*3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len)){
                                Serial.println("fmt2jpg failed");
                                res = ESP_FAIL;
                            }
                            esp_camera_fb_return(fb);
                            fb = NULL;
                        } else {
                             SetNoFaceFound();
                            _jpg_buf = fb->buf;
                            _jpg_buf_len = fb->len;
                            face_x = 0;
                            face_y = 0; 
                        }
                        fr_encode = esp_timer_get_time();
                    }
                    dl_matrix3du_free(image_matrix);
                }
            }
        }


        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
           // break; // if this becomes a loop function
        }
#ifdef DEBUG_FRAMERATE
        int64_t fr_end = esp_timer_get_time();

        int64_t ready_time = (fr_ready - fr_start)/1000;
        int64_t face_time = (fr_face - fr_ready)/1000;
        int64_t recognize_time = (fr_recognize - fr_face)/1000;
        int64_t encode_time = (fr_encode - fr_recognize)/1000;
        int64_t process_time = (fr_encode - fr_start)/1000;
        
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        Serial.printf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps), %u+%u+%u+%u=%u %s%d %s%d%s%d\n",
            (uint32_t)(_jpg_buf_len),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
            avg_frame_time, 1000.0 / avg_frame_time,
            (uint32_t)ready_time, (uint32_t)face_time, (uint32_t)recognize_time, (uint32_t)encode_time, (uint32_t)process_time,
            (detected)?"DETECTED ":"", face_id,
            (detected)?"First Location ":"", face_x, ",", face_y
        );
#endif
    //}

    last_frame = 0;
}
