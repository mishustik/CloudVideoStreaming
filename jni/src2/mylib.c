#include <jni.h>
#include <android/log.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libswscale/swscale.h>
#include "libswscale/swscale_internal.h"
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>

#define LOG_TAG "mylib"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);

AVFrame* ReadFrame(const char* imageFileName, AVCodecContext *pCodecCtxdata);


jint Java_com_example_cloudvideostreaming_MainActivity_VideoDecoding(JNIEnv * env, jobject this, jstring filename, jstring filename2)
{
	AVFormatContext *pFormatCtx = avformat_alloc_context(),
					*pFormatCtxStream = avformat_alloc_context();
	jint			nvc = 0;
	int             i,videoStream;
	AVCodecContext  *pCodecCtx, *pCodecCtxEnc, *pCodecCtxStream;
	AVOutputFormat 	*OutStreamFormat;
	AVStream		*OutStream;
	AVCodec         *pCodec, *pCodecEnc;
	AVFrame         *pFrame, *pFrameEncOut = NULL;
	AVPacket        packet, packetEnc;
	int             frameFinished, Res, check_fwrite, check_size, got_packet_ptr;
	int             outbuf_size, out_size, size;
	uint8_t         *outbuf, *framebuffer;
	FILE			*NewVideoFile;
	char FileName[92];

	AVDictionary    *optionsDict = NULL;
	struct SwsContext *sws_ctx = NULL, *sws_ctx_enc = NULL;
	void *imglib, *imgsymb = NULL;
	void* (*lib_func)() = 0;
	typedef void (*simple_demo_function)(void);
	simple_demo_function demo_function;
	const char* error;


	av_register_all(); // registers all available file formats and codecs with the library
				   	   //so the will be used automatically when corresponding format/codec gets opened
	//////////////////////////////////DECODING DECLARATION//////////////////////////////////////////////

	av_init_packet(&packet);

	const jbyte *str, *str2;

	str = (*env)->GetStringUTFChars(env, filename, NULL);
	str2 = (*env)->GetStringUTFChars(env, filename2, NULL);

	//open video file
	if(avformat_open_input(&pFormatCtx,str,NULL,NULL)!=0)
		return -1;

	//retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1;

	//dump information about file onto standard error
	av_dump_format(pFormatCtx,0,str,0);

	//find the first video stream
	videoStream=-1;

	for (i=0;i<pFormatCtx->nb_streams;i++){
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			LOGI("videoStream=%d",videoStream);
			break;
		}
	}

	if(videoStream==-1)
		return -1;

	//get a pointer to the codec context for the video stream
	pCodecCtx=pFormatCtx->streams[videoStream]->codec;

	LOGI("pCodecCtx bitrate: %d",pCodecCtx->bit_rate);
	LOGI("pCodecCtx width: %d",pCodecCtx->width);
	LOGI("pCodecCtx height: %d",pCodecCtx->height);
	LOGI("pCodecCtx time_base: %d",pCodecCtx->time_base);
	LOGI("pCodecCtx gop_size: %d",pCodecCtx->gop_size);
	LOGI("pCodecCtx max_b frames: %d",pCodecCtx->max_b_frames);
	LOGI("pCodecCtx pix_fmt: %d",pCodecCtx->pix_fmt);

	//Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);

	if(pCodec==NULL)
		return -1;

	//Open codec
	if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
		return -1; //could not open codec

	//Allocate video frame
	pFrame=avcodec_alloc_frame();

	///////////////////DECODING DECLARATION FINISHED//////////////////////////////////////////////

	///////////////////ENCODING DECLARATION//////////////////////////////////

	//find codec for video encoding (to which format we want to code)

	pCodecEnc = avcodec_find_encoder(CODEC_ID_H264);
	if(!pCodecEnc){
		LOGE("Error finding encoder");
		exit(1);
	}

	//set the parameters for our codec context
	LOGI("hi");
//	pCodecCtxEnc->profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;
	LOGI("hi");
	pCodecCtxEnc = avcodec_alloc_context3(pCodecEnc);
	pCodecCtxEnc->bit_rate = 9 * 10e5/*pCodecCtx->bit_rate*/;
	pCodecCtxEnc->width = 200;
	pCodecCtxEnc->height = 200;
	pCodecCtxEnc->time_base = (AVRational){1,25};
	pCodecCtxEnc->gop_size = 18;
	pCodecCtxEnc->max_b_frames = 0;
	pCodecCtxEnc->pix_fmt = PIX_FMT_YUV420P;

	//matching codec and codec context
	LOGI("hi");
	if(avcodec_open2(pCodecCtxEnc, pCodecEnc, &optionsDict) < 0){
		LOGE("could not open encoding codec");
		exit(1);
	}

	//open file for our final output video

	NewVideoFile = fopen("/sdcard/Download/modified_video0.mp4","wb");
	if(!NewVideoFile){
		LOGE("Output file for video is not opened");
		exit(1);
	}

	//allocate buffer for our final output video from which we will write to file
	outbuf_size = 100000;
	outbuf = (uint8_t*)av_malloc(outbuf_size);

	//final frame, it will be written to video file
	pFrameEncOut = avcodec_alloc_frame();

	//buffer for our final frame pFrameEncOut

	size = pCodecCtxEnc->width * pCodecCtxEnc->height;
	framebuffer = malloc((size * 3) / 2);

	//matching our final frame with it's buffer
	check_size = avpicture_fill((AVPicture *) pFrameEncOut, framebuffer, PIX_FMT_YUV420P, pCodecCtxEnc->width, pCodecCtxEnc->height);

	///////////////////////////////////ENCODING DECLARATION FINISHED//////////////////////////////////////////////

	///////////////////////////////////STARTING PROCESS///////////////////////////////////////////////////////////

	// Read and save frames
	__android_log_print(ANDROID_LOG_INFO,"SomeTag","$$$$$$$$$$$$$$$ Loop has started $$$$$$$$$$$$$$$$$");
	i=0;
	int z = 0;
	frameFinished = 0;
	got_packet_ptr = 0;
	packetEnc.data = outbuf;
	packetEnc.size = outbuf_size;

    while(av_read_frame(pFormatCtx, &packet)>=0) {    //while next frame exists do...
    	// Is this a packet from the video stream?
    	__android_log_print(ANDROID_LOG_INFO,"SomeTag","Current loop number is %d",z);

    	if(packet.stream_index==videoStream) {

    		LOGI("our video stream");
    		// Decode video from the packet into the picture to the frame
    		while (frameFinished == 0){
    			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
    			LOGE("another try");
    		}
    		LOGI("framefinished: %d",frameFinished);
    		// Did we get a video frame?
    		if(frameFinished) {

    			frameFinished = 0;
    			//initializing a packet matched to the buffer
    			av_init_packet(&packetEnc);

    			__android_log_print(ANDROID_LOG_INFO,"SomeTag","number of frame =: %d",i);
    			++i;
    			fflush(stdout);

    			//Scaling the frame which we obtained from the picture to our final frame
    			sws_ctx_enc =
    					sws_getContext
    					(
    							pFrame->width,
    							pFrame->height,
    							pFrame->format,
    							pCodecCtxEnc->width,
    							pCodecCtxEnc->height,
    							PIX_FMT_YUV420P,
    							SWS_BILINEAR,
    							NULL,
    							NULL,
    							NULL
    					);

    			//Scaling

    			Res = sws_scale(sws_ctx_enc, (const uint8_t* const*)pFrame->data, pFrame->linesize,0,pFrame->height, pFrameEncOut->data,pFrameEncOut->linesize);		//!

    			//encoding our final frame to final buffer

    			got_packet_ptr = 0;
    		//	pCodecCtxEnc->profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;

    			while (got_packet_ptr == 0){
    				LOGE("got_packet_ptr",got_packet_ptr);
    				out_size = avcodec_encode_video2(pCodecCtxEnc, &packetEnc, pFrameEncOut, &got_packet_ptr);
    			}

    			if(got_packet_ptr){
    				LOGE("got packet!");
    				//writing from final buffer to final video file
    				check_fwrite = fwrite(packetEnc.data,1,packetEnc.size,NewVideoFile);
    				LOGI("frame is written to video file. Total number of written elements: %d",check_fwrite);
    				av_free_packet(&packetEnc);
    			}


    			//shared library access try
   /*
    			imglib = dlopen("/obj/local/armeabi/libmylib.so", RTLD_NOW);
    			if(!imglib){
    				LOGE("error imglib: %s",dlerror());
    				exit(1);
    			}
    			LOGI("good");
    			lib_func=(void*(*)())dlsym(imglib,"gl_tester");
    			LOGI("lib_func: %p",lib_func);

    			LOGI("happy end");
    */

    	//		OutStreamFormat = av_guess_format(NULL,"/sdcard/Download/modified_video.mp4",NULL);
    	//		pFormatCtxStream->oformat = OutStreamFormat;

    			//create and initialize AVIOContext for accessing the resource indicated by url
    	//		avio_open2(&pFormatCtxStream->pb,"/sdcard/Download/modified_video.mp4",AVIO_FLAG_WRITE,NULL,NULL);

    			//adding new stream to a media file
    	//		OutStream = avformat_new_stream(pFormatCtxStream,NULL);
    	//		avcodec_copy_context(OutStream->codec,pCodecCtxStream);

    	//		OutStream->sample_aspect_ratio.num = pCodecCtxStream->sample_aspect_ratio.num;
    	//		OutStream->sample_aspect_ratio.den = pCodecCtxStream->sample_aspect_ratio.den;

    	//		OutStream->r_frame_rate = 50;
    	//		OutStream->avg_frame_rate = OutStream->r_frame_rate;
    	//		OutStream->time_base = av_inv_q( OutStream->r_frame_rate );
    	//		OutStream->codec->time_base = OutStream->time_base;

    	//		avformat_write_header(pFormatCtxStream,NULL);
    	//		snprintf(pFormatCtxStream->filename, sizeof(pFormatCtxStream->filename),"%s","/sdcard/Download/modified_video.mp4");


    		}
    	}

    	// Free the packet that was allocated by av_read_frame
    	av_free_packet(&packet);
    	z++;
    	if ((z % 500) == 0){

    		fclose(NewVideoFile);
    		nvc++;
    		sprintf(FileName, "/sdcard/Download/modified_video%d.mp4", nvc);
    		NewVideoFile = fopen(FileName,"wb");
    			if(!NewVideoFile){
    				LOGE("Output file for video is not opened");
    				exit(1);
    			}
    	}
    }
    LOGE("STEP14!");

    //add sequence to have a real encode file

    outbuf[0] = 0x00;
    outbuf[1] = 0x00;
    outbuf[2] = 0x01;
    outbuf[3] = 0xb7;
    fwrite(outbuf,1,4,NewVideoFile);
    av_free(outbuf);
    LOGE("STEP15!");

    // Free the YUV frame
    av_free(pFrame);
    LOGE("STEP16!");

    // Close the codec
    avcodec_close(pCodecCtx);
    LOGE("STEP17!");

    //Close output video file
    fclose(NewVideoFile);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    LOGI("STEP18!");
    LOGI("%d",nvc);

    return nvc;
}

jint Java_com_example_cloudvideostreaming_MainActivity_PlayingVideo(JNIEnv * env, jobject this, jstring filename){

	return 0;
}

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;

  // Open file
  if (iFrame / 10 == 0)
	  sprintf(szFilename, "/sdcard/Download/frame000%d.ppm", iFrame);
  else
	  if (iFrame / 100 == 0)
		  sprintf(szFilename, "/sdcard/Download/frame00%d.ppm", iFrame);
	  else
		  if (iFrame / 1000 == 0)
			  sprintf(szFilename, "/sdcard/Download/frame0%d.ppm", iFrame);
		  else
			  if (iFrame / 10000 == 0)
			  	  sprintf(szFilename, "/sdcard/Download/frame%d.ppm", iFrame);
//  __android_log_print(ANDROID_LOG_INFO,"SomeTag","iFrame is: %d, filename is: %s", iFrame, &szFilename);

  pFile=fopen(szFilename, "wb");	//wb - creates empty binary file for output operation
  if(pFile==NULL){
	__android_log_print(ANDROID_LOG_INFO,"SomeTag","error: %s", strerror(errno));
	return;
  }
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);

  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

  // Close file
  fclose(pFile);
}

extern AVFrame* ReadFrame(const char* imageFileName, AVCodecContext *pCodecCtxdata){

	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVFrame *pFrame = NULL;
	AVPacket packetEnc;
	AVCodec *pCodec = NULL;
	int Frame_Finished;

	if(avformat_open_input(&pFormatCtx,imageFileName,NULL,NULL)!=0){
	        	LOGE("error opening file");
	        	return NULL;
	        }

	av_dump_format(pFormatCtx,0,imageFileName,0);

	pCodecCtx = pFormatCtx->streams[0]->codec;
	//pCodecCtx->pix_fmt = PIX_FMT_YUV420P;

	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(!pCodec){
		LOGE("Codec not found!");
		return NULL;
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0){
		LOGE("Could not open codec\n");
		return NULL;
	}
//	LOGI("avcodec_open");

	pFrame = avcodec_alloc_frame();
	if(!pFrame){
		LOGE("Cannot allocate memory to AVFrameEnc");
		return NULL;
	}
//	LOGI("Frame is allocated");

	int numBytesforFrame = avpicture_get_size(PIX_FMT_RGB24, pCodecCtxdata->width, pCodecCtxdata->height);
	uint8_t *bufferforFrame = (uint8_t *) av_malloc(numBytesforFrame * sizeof(uint8_t));

	avpicture_fill((AVPicture *) pFrame, bufferforFrame, PIX_FMT_RGB24, pCodecCtxdata->width, pCodecCtxdata->height);

//	LOGI("picture is filled");
//	__android_log_print(ANDROID_LOG_INFO,"SomeTag","before pFrame data is %d",pFrame->data);
//	__android_log_print(ANDROID_LOG_INFO,"SomeTag","before pFrame format is %d",pFrame->format);
//	__android_log_print(ANDROID_LOG_INFO,"SomeTag","before pFrame pict type is %d",pFrame->pict_type);

	int framesNumber = 0;
	while (av_read_frame(pFormatCtx, &packetEnc) >= 0){
		if(packetEnc.stream_index != 0)
			continue;

//		__android_log_print(ANDROID_LOG_INFO,"SomeTag","size of packet is: %d", packetEnc.size);
//		__android_log_print(ANDROID_LOG_INFO,"SomeTag","data in packet: %s", &packetEnc.data);

		int ret =avcodec_decode_video2(pCodecCtx,pFrame, &Frame_Finished, &packetEnc);
//		LOGI("decoded");
		if (ret > 0){
//			LOGI("Frame is decoded, size is %d", ret);
//			__android_log_print(ANDROID_LOG_INFO,"SomeTag","pFrame data is %d",pFrame->data);
//			__android_log_print(ANDROID_LOG_INFO,"SomeTag","pFrame format is %d",pFrame->format);
//			__android_log_print(ANDROID_LOG_INFO,"SomeTag","pFrame pict type is %d",pFrame->pict_type);
//			__android_log_print(ANDROID_LOG_INFO,"SomeTag","YUV format is %d",PIX_FMT_YUV420P);
//			__android_log_print(ANDROID_LOG_INFO,"SomeTag","RGB24 format is %d",PIX_FMT_RGB24);
//			pFrame->quality = 4;
			free(bufferforFrame);
			avcodec_close(pCodecCtx);
			av_free(pCodecCtx);
			return pFrame;
		}
		else{
			__android_log_print(ANDROID_LOG_INFO,"SomeTag","error: %s", strerror(errno));
		}
	}
	return pFrame;
}


