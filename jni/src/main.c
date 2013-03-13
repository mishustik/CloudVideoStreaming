#include <jni.h>

#include <stdio.h>

#include "SDL.h"
#include <SDL_thread.h>
#include <SDL_video.h>

#include <android/log.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswscale/swscale_internal.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>

#include <assert.h>


#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG, "sdldemo", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "sdldemo", __VA_ARGS__)

int running = 1;

int SDL_main(int argc, char *argv[]) {

	const char* templ = "SDLThread";
	AVFrame *pFrame, *pFrame_YUV420P;
	AVPacket packet;
	uint8_t *buffer_YUV420P;

	LOGI("hi there yoyoyyo");

	LOGI("hi1");
	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	LOGI("hi2");
	int err;

	LOGI("hi3");
	// Init SDL with video support
	err = SDL_Init(SDL_INIT_VIDEO);
	if (err < 0) {
		LOGI("Unable to init SDL: %s\n", SDL_GetError());
		return -1;
	}

	LOGI("hi4");
	// Open video file
	const char* filename = "/sdcard/Download/Received_video.mp4";
	AVFormatContext* format_context = NULL;
	err = avformat_open_input(&format_context, filename, NULL, NULL);
	if (err < 0) {
		LOGI("ffmpeg: Unable to open input file\n");
		return -1;
	}

	LOGI("hi5");
	// Retrieve stream information
	err = avformat_find_stream_info(format_context, NULL);
	if (err < 0) {
		fprintf(stderr, "ffmpeg: Unable to find stream info\n");
		return -1;
	}
	LOGI("hi6");
	// Dump information about file onto standard error
	av_dump_format(format_context, 0, "/sdcard/Download/Received_video.mp4", 0);
	LOGI("hi7");
	// Find the first video stream
	int video_stream;
	for (video_stream = 0; video_stream < format_context->nb_streams; ++video_stream) {
		if (format_context->streams[video_stream]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			break;
		}
	}
	if (video_stream == format_context->nb_streams) {
		LOGI("ffmpeg: Unable to find video stream\n");
		return -1;
	}
	LOGI("hi8");
	AVCodecContext* codec_context = format_context->streams[video_stream]->codec;
	AVCodec* codec = avcodec_find_decoder(codec_context->codec_id);
	err = avcodec_open2(codec_context, codec, NULL);
	if (err < 0) {
		LOGI("ffmpeg: Unable to open codec\n");
		return -1;
	}
	LOGI("hi9");

	pFrame = avcodec_alloc_frame();
	LOGI("hi10");

	pFrame_YUV420P = avcodec_alloc_frame();
	LOGI("hi11");

	if(pFrame_YUV420P == NULL) {
		LOGI("Could not allocate pFrame_YUV420P");
		return -1;
	}
	LOGI("hi12");
	//Determine required buffer size and allocate buffer
	buffer_YUV420P = (uint8_t *)av_malloc(sizeof(uint8_t)*
			avpicture_get_size(PIX_FMT_YUV420P, codec_context->width, codec_context->height));
	LOGI("hi13");
	//Assign buffer to image planes
	avpicture_fill((AVPicture *)pFrame_YUV420P, buffer_YUV420P,
			PIX_FMT_YUV420P, codec_context->width,codec_context->height);
	LOGI("hi14");

	struct SWSContext *pConvertCtx_YUV420P;
	LOGI("hi15");

	//format conversion context
	pConvertCtx_YUV420P = sws_getContext(codec_context->width, codec_context->height, codec_context->pix_fmt,
										codec_context->width, codec_context->height, PIX_FMT_YUV420P,
										SWS_SPLINE, NULL, NULL, NULL);
	LOGI("hi16");
	SDL_Window *pWindow1;
	SDL_Renderer *pRenderer1;
	SDL_Texture *bmpTex1;
	uint8_t *pixels1;
	int pitch1, size1;

	//allocate window renderer, texture
	pWindow1 = SDL_CreateWindow("YUV", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			codec_context->width, codec_context->height, SDL_WINDOW_SHOWN);
	pRenderer1 = SDL_CreateRenderer(pWindow1,-1,SDL_RENDERER_ACCELERATED);
	bmpTex1 = SDL_CreateTexture(pRenderer1,SDL_PIXELFORMAT_YV12,
			SDL_TEXTUREACCESS_STREAMING, codec_context->width, codec_context->height);
	size1 = codec_context->width * codec_context->height;
	if(pWindow1 == NULL | pRenderer1 == NULL | bmpTex1 == NULL) {
		LOGI("Could not open window1!");
		return -1;
	}
	LOGI("hi17");

	int i, frameDecoded;

	LOGI("hi18");

	while(running){

		//read frame

		if (av_read_frame(format_context,&packet) < 0){
			continue;
		}

		//decode the frame
		if (avcodec_decode_video2(codec_context,pFrame,&frameDecoded,&packet) < 0) {
			LOGI("Could not decode frame!");
			continue;
		}

		if (frameDecoded) {
			//convert frame to YUV
			sws_scale(pConvertCtx_YUV420P,pFrame->data, pFrame->linesize,0,
					codec_context->height, pFrame_YUV420P->data,pFrame_YUV420P->linesize);

			//copy converted YUV to SDL 2.0 texture
			SDL_LockTexture(bmpTex1, NULL, (void **)&pixels1, &pitch1);
				memcpy(pixels1, pFrame_YUV420P->data[0], size1);
				memcpy(pixels1 + size1, pFrame_YUV420P->data[2], size1/4);
				memcpy(pixels1 + size1*5/4, pFrame_YUV420P->data[1],size1/4);
			SDL_UnlockTexture(bmpTex1);
			SDL_UpdateTexture(bmpTex1, NULL, pixels1, pitch1);

			// refresh screen
			SDL_RenderClear(pRenderer1);
			SDL_RenderCopy(pRenderer1, bmpTex1, NULL, NULL);
			SDL_RenderPresent(pRenderer1);

		}
	}

	while(!running){

			//read frame

			if (av_read_frame(format_context,&packet) < 0){
				break;
			}

			//decode the frame
			if (avcodec_decode_video2(codec_context,pFrame,&frameDecoded,&packet) < 0) {
				LOGI("Could not decode frame!");
				continue;
			}

			if (frameDecoded) {
				//convert frame to YUV
				sws_scale(pConvertCtx_YUV420P,pFrame->data, pFrame->linesize,0,
						codec_context->height, pFrame_YUV420P->data,pFrame_YUV420P->linesize);

				//copy converted YUV to SDL 2.0 texture
				SDL_LockTexture(bmpTex1, NULL, (void **)&pixels1, &pitch1);
					memcpy(pixels1, pFrame_YUV420P->data[0], size1);
					memcpy(pixels1 + size1, pFrame_YUV420P->data[2], size1/4);
					memcpy(pixels1 + size1*5/4, pFrame_YUV420P->data[1],size1/4);
				SDL_UnlockTexture(bmpTex1);
				SDL_UpdateTexture(bmpTex1, NULL, pixels1, pitch1);

				// refresh screen
				SDL_RenderClear(pRenderer1);
				SDL_RenderCopy(pRenderer1, bmpTex1, NULL, NULL);
				SDL_RenderPresent(pRenderer1);

			}
		}

	LOGI("end!!!!");

	SDL_DestroyTexture(bmpTex1);
	SDL_DestroyRenderer(pRenderer1);
	SDL_DestroyWindow(pWindow1);

	return 0;
}

jint Java_com_example_cloudvideostreaming_MainActivity_EndCheck(JNIEnv * env, jobject this){

	running = 0;

	return 0;
};
