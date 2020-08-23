#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL.h>
#include <libavutil/imgutils.h>

#define OUTPUT_YUV420P 0
#define ERROR -1;

int main(int argc, char* argv[]) {
	AVFormatContext* formatContext;
	int i, videoIndex;
	AVCodecContext* codecContext;
	AVCodec* codec;
	AVFrame* frame, * frameYUV;
	unsigned char* out_buffer;
	AVPacket* packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext* img_convert_context;

	char filepath[] = "test.h265";
	
	int screen_w = 0, screen_h = 0;
	SDL_Window* screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	FILE* file_yuv;
	av_register_all();
	avformat_network_init();
	formatContext = avformat_alloc_context();
	if (avformat_open_input(&formatContext,filepath,NULL,NULL)!=0)
	{
		printf("open file error.\n");
		return ERROR;
	}
	if (avformat_find_stream_info(formatContext,NULL)<0)
	{
		printf("find stream information error.\n");
		return ERROR;
	}
	videoIndex = -1;
	for (int i = 0; i < formatContext->nb_streams; i++)
	{
		if (formatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			videoIndex = i;
			break;
		}
	}
	if (videoIndex==-1)
	{
		printf("not find video stream.\n");
		return ERROR;
	}
	codecContext = formatContext->streams[videoIndex]->codec;
	codec = avcodec_find_encoder(codecContext->codec_id);
	if (codec==NULL)
	{
		printf("codec not find.\n");
		return ERROR;
	}
	if (avcodec_open2(codecContext,codec,NULL)<0)
	{
		printf("open codec error.\n");
		return ERROR;
	}
	frame = av_frame_alloc();
	frameYUV = av_frame_alloc();
	out_buffer = av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height, 1));
	av_image_fill_arrays(frameYUV->data,frameYUV->linesize,out_buffer,AV_PIX_FMT_YUV420P,codecContext->width,codecContext->height,1);
	packet = av_malloc(sizeof(AVPacket));
	printf("-----------------------file info-------------------------");
	av_dump_format(formatContext, 0, filepath,0);
	printf("---------------------------------------------------------");
	img_convert_context = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
		codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
#if OUTPUT_YUV420P
	file_yuv = fopen("output.yuv","wb+");
#endif // OUTPUT_YUV420P
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER))
	{
		printf("init SDL error -%s.\n",SDL_GetError());
		return ERROR;
	}
	screen_h = codecContext->height;
	screen_w = codecContext->width;

	screen = SDL_CreateWindow("Simplest ffmpeg player window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_VIDEO_OPENGL);
	if (!screen)
	{
		printf("SDL:not create window =exiting:%s\n",SDL_GetError());
		return ERROR;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

	return 0;
}