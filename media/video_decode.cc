/************************************************************************
* Copyright(c) 2012  Yang Xian
* All rights reserved.
*
* File:	VideoDecode.cpp
* Brief: ��Ƶ���룬���ѽ�����RGB��Ƶ֡����
* Version: 1.0
* Author: Yang Xian
* Email: yang_xian521@163.com
* Date:	2012/06/28
* History:
************************************************************************/
#include <stdio.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
};


void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf(szFilename, "frame%d.ppm", iFrame);
	pFile=fopen(szFilename, "wb");
	if(pFile==NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	// Write pixel data
	for(y=0; y<height; y++)
		fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

	// Close file
	fclose(pFile);
}

int main(int argc, char *argv[])
{
	AVFormatContext *pFormatCtx;
	int             i, videoStream;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame         *pFrame; 
	AVFrame         *pFrameRGB;
	AVPacket        packet;
	int             frameFinished;
	int             numBytes;
	uint8_t         *buffer;
	struct SwsContext *img_convert_ctx;

	if(argc < 2)
	{
		printf("Please provide a movie file\n");
		return -1;
	}
	// Register all formats and codecs
	// ��ʼ��ffmpeg��
	av_register_all();

	// Open video file
    if(av_open_input_file(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
	//if(avformat_open_input(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
		return -1; // Couldn't open file

	// Retrieve stream information
	// �����ļ�������Ϣ
	if(av_find_stream_info(pFormatCtx)<0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	// dumpֻ��һ�����Ժ���������ļ���������Ƶ���Ļ�����Ϣ��֡�ʡ��ֱ��ʡ���Ƶ�����ȵ�
	dump_format(pFormatCtx, 0, argv[1], 0);

	// Find the first video stream
	// �����ļ��������ҵ���һ����Ƶ��������¼���ı�����Ϣ
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
	{
		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
		{
			videoStream=i;
			break;
		}
	}
	if(videoStream==-1)
		return -1; // Didn't find a video stream

	// Get a pointer to the codec context for the video stream
	// �õ���Ƶ�������������ָ��
	pCodecCtx=pFormatCtx->streams[videoStream]->codec;

	// construct the scale context, conversing to PIX_FMT_RGB24
	// ���ݱ�����Ϣ������Ⱦ��ʽ
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
			pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	if(img_convert_ctx == NULL)
	{
		fprintf(stderr, "Cannot initialize the conversion context!\n");
// 		exit(1);
		return -1;
	}

	// Find the decoder for the video stream
	// �ڿ��������֧�ָø�ʽ�Ľ�����
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL)
	{
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}
	// Open codec
	// �򿪽�����
	if(avcodec_open(pCodecCtx, pCodec)<0)
		return -1; // Could not open codec

	// Allocate video frame
	// ����һ��ָ֡�룬ָ�������ԭʼ֡
	pFrame=avcodec_alloc_frame();

	// Allocate an AVFrame structure
	// ����һ��ָ֡�룬ָ����ת����rgb���֡
	pFrameRGB=avcodec_alloc_frame();
	if(pFrameRGB==NULL)
		return -1;

	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));	// buffer = new uint8_t[numBytes];

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	// ��pFrameRGB֡�����Ϸ�����ڴ�
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	// Read frames and save first five frames to disk
	i=0;
	while(av_read_frame(pFormatCtx, &packet)>=0)	// ��ȡһ��֡
	{
		// Is this a packet from the video stream?
		if(packet.stream_index==videoStream)
		{
			// Decode video frame
			// �����֡
			avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, packet.data, packet.size);

			// Did we get a video frame?
			if(frameFinished)
			{
				// Convert the image from its native format to RGB
				// img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, 
				// 				(AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, 
				//              pCodecCtx->height);
				
				// �Ѹ�֡ת����rgb

				// ���ֻ��ȡ�ؼ�֡���������
				// if (pFrame->key_frame == 1)
				sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);	
				// Save the frame to disk
				// ����ǰ5֡
				if(++i<=5)
				{
// 					char pic[200];
// 					sprintf(pic,"pic%d.bmp",i);
// 					av_create_bmp(pic, pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, 24);
					SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
				}
			}
		}

		// Free the packet that was allocated by av_read_frame
		// �ͷŶ�ȡ��֡�ڴ�
		av_free_packet(&packet);
	}

	// Free the RGB image
	av_free(buffer);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codec
	avcodec_close(pCodecCtx);

	// Close the video file
	av_close_input_file(pFormatCtx);

	return 0;
}
