
#include "example_Iloczyn.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>


typedef struct StreamContext {
	AVCodecContext* decodec_ctx;
	AVCodecContext* encodec_ctx;
	AVFrame* decoded_frame;
} StreamContext;


static AVFormatContext* in_format_ctx;
static AVFormatContext* out_format_ctx;
static StreamContext* stream_context;
char* muxer_key;
char* muxer_value;
static int open_input_file(const char* filename) {
	int ret;
	unsigned int i;

	in_format_ctx = NULL;

	//Otworzenie input_format_ctx
	if ((avformat_open_input(&in_format_ctx, filename, NULL, NULL)) < 0) {

		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return -1;
	}



	//Odczyt pakietów danych w celu uzyskania informacji o strumieniach
	if ((avformat_find_stream_info(in_format_ctx, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return -1;
	}


	//Alokacja pamiêci dla stream_context
	stream_context = av_mallocz_array(in_format_ctx->nb_streams, sizeof(*stream_context->decodec_ctx));
	if (!stream_context) {
		return AVERROR(ENOMEM);
	}

	for (i = 0; i < in_format_ctx->nb_streams; i++) {
		AVStream* stream = in_format_ctx->streams[i];

		//Znalezienie dekodera dla tego strumienia
		AVCodec* dec = avcodec_find_decoder(stream->codecpar->codec_id);
		AVCodecContext* codec_context;
		if (!dec) {
			av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
			return AVERROR_DECODER_NOT_FOUND;
		}

		//Alokacja pamiêci dla codec_context dekodera
		codec_context = avcodec_alloc_context3(dec);
		if (!codec_context) {
			av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
			return AVERROR(ENOMEM);
		}
		//Skopiowanie parametrów dekodera do wejœciowego codec_context dekodera dla tego strumienia
		ret = avcodec_parameters_to_context(codec_context, stream->codecpar);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context" "for stream #%u\n", i);
			return ret;
		}


		//Reencode video & audi and remux subtitle etc. metadate

		if (codec_context->codec_type == AVMEDIA_TYPE_VIDEO || codec_context->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (codec_context->codec_type == AVMEDIA_TYPE_VIDEO) codec_context->framerate = av_guess_frame_rate(in_format_ctx, stream, NULL); //Zgadniêcie frame rate, bazuj¹c na obu kontenerach i informacji z kodeka
		//Otworzenie dekodera
			ret = avcodec_open2(codec_context, dec, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				return ret;
			}
		}
		stream_context[i].decodec_ctx = codec_context;
		stream_context[i].decoded_frame = av_frame_alloc();
		if (!stream_context[i].decoded_frame) return AVERROR(ENOMEM);

	}

	//Wyœwietlenie szczegó³ów odnoœnie wejœciowego pliku, takich jak czas trwania, przep³ywnoœæ bitowa, strumienie, format, metadane, kodek, sygnatura czasu
	av_dump_format(in_format_ctx, 0, filename, 0);
	return 0;

}
static int open_output_file(const char* filename, char* crf, char* preset) {
	AVStream* in_stream;
	AVStream* out_stream;

	AVCodecContext* decodec_ctx, * encodec_ctx;
	AVCodec* encoder;
	int ret;
	unsigned int i;
	out_format_ctx = NULL;

	avformat_alloc_output_context2(&out_format_ctx, NULL, NULL, filename);
	if (!out_format_ctx) {
		av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		return AVERROR_UNKNOWN;
	}
	for (i = 0; i < in_format_ctx->nb_streams; i++) {

		//Dodanie nowego strumienia  do pliku wyjœciowego
		out_stream = avformat_new_stream(out_format_ctx, NULL);
		if (!out_stream) {
			av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
			return AVERROR_UNKNOWN;
		}
		in_stream = in_format_ctx->streams[i];
		decodec_ctx = stream_context[i].decodec_ctx;
		//if (decodec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || decodec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
		if (decodec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {

			//Przekodowanie do tego samego kodeka:
			//encoder = avcodec_find_encoder(decodec_ctx->codec_id);
			encoder = avcodec_find_encoder_by_name("libx265");
			if (!encoder) {
				av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
				return AVERROR_INVALIDDATA;
			}
			encodec_ctx = avcodec_alloc_context3(encoder);
			if (!encodec_ctx) {
				av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
				return AVERROR(ENOMEM);
			}
			//Ustawienie tych samych parametrów (rozmiar obrazu, czêstotliwoœæ próbkowania itd...)

			av_opt_set(encodec_ctx->priv_data, "crf", crf, 0);
			av_opt_set(encodec_ctx->priv_data, "preset", preset, 0);


			encodec_ctx->height = decodec_ctx->height;
			encodec_ctx->width = decodec_ctx->width;
			encodec_ctx->sample_aspect_ratio = decodec_ctx->sample_aspect_ratio;
			if (encodec_ctx->pix_fmt) {
				encodec_ctx->pix_fmt = encoder->pix_fmts[0];
			}
			else
				encodec_ctx->pix_fmt = decodec_ctx->pix_fmt;

			encodec_ctx->bit_rate = decodec_ctx->bit_rate;
			encodec_ctx->rc_buffer_size = decodec_ctx->rc_buffer_size;
			encodec_ctx->rc_max_rate = decodec_ctx->rc_max_rate;
			encodec_ctx->rc_min_rate = decodec_ctx->rc_min_rate;
			AVRational framerate = av_guess_frame_rate(in_format_ctx, in_stream, NULL);
			//framerate.num = framerate.num * 2;
			encodec_ctx->time_base = decodec_ctx->time_base;//av_inv_q(framerate);

			out_stream->time_base = in_stream->time_base;

			if (out_format_ctx->oformat->flags & AVFMT_GLOBALHEADER) encodec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			//Third parameter can be used to pass settings to encoder 
			ret = avcodec_open2(encodec_ctx, encoder, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i); //Unsuported channel layout "0 channels"
				return ret;
			}

			ret = avcodec_parameters_from_context(out_stream->codecpar, encodec_ctx);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
				return ret;
			}
			//	out_stream->time_base = encodec_ctx->time_base;
			stream_context[i].encodec_ctx = encodec_ctx;
		}
		else if (decodec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			//Przekodowanie do tego samego kodeka:
			encoder = avcodec_find_encoder(decodec_ctx->codec_id);
			//	encoder = avcodec_find_encoder_by_name("aac");
			if (!encoder) {
				av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
				return AVERROR_INVALIDDATA;
			}
			encodec_ctx = avcodec_alloc_context3(encoder);
			if (!encodec_ctx) {
				av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
				return AVERROR(ENOMEM);
			}
			//Ustawienie tych samych parametrów (rozmiar obrazu, czêstotliwoœæ próbkowania itd...)


			encodec_ctx->sample_rate = decodec_ctx->sample_rate;
			encodec_ctx->channel_layout = decodec_ctx->channel_layout;
			encodec_ctx->channels = av_get_channel_layout_nb_channels(encodec_ctx->channel_layout); //<- Return the number of channels in the channel layout
			//Take a first format from list of supported formats
			encodec_ctx->sample_fmt = encoder->sample_fmts[0];

			encodec_ctx->time_base = (AVRational){ 1,encodec_ctx->sample_rate };


			if (out_format_ctx->oformat->flags & AVFMT_GLOBALHEADER) encodec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			//Third parameter can be used to pass settings to encoder 
			ret = avcodec_open2(encodec_ctx, encoder, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i); //Unsuported channel layout "0 channels"
				return ret;
			}

			ret = avcodec_parameters_from_context(out_stream->codecpar, encodec_ctx);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
				return ret;
			}
			out_stream->time_base = encodec_ctx->time_base;
			stream_context[i].encodec_ctx = encodec_ctx;
		}
		else if (decodec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
			av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
			return AVERROR_INVALIDDATA;
		}
		else {
			//if this stream must be remuxed (multipleksacja)
			ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Copying parameters for stream #%u failed\n", i);
				return ret;
			}
			out_stream->time_base = in_stream->time_base;
		}


	}

	av_dump_format(out_format_ctx, 0, filename, 1);
	if (!(out_format_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&out_format_ctx->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
			return ret;
		}
	}
	//Inicjalizacja multipleksacji, zapis wyjœciowego nag³ówka

	AVDictionary* muxer_dict = NULL;
	av_dict_set(&muxer_dict, muxer_key, muxer_value, 0);

	ret = avformat_write_header(out_format_ctx, &muxer_dict);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
		return ret;
	}
	return 0;




}
static int encode(AVFrame* frame, unsigned int stream_index, unsigned int duration) {
	//printf("%d ", frame->pkt_pos);
	StreamContext* stream = &stream_context[stream_index]; //Utworzenie StreamContext dla zgodnego stream
	int ret;
	AVPacket encodec_packet; //New Packet <- Wyjœciowy pakiet
	//av_log(NULL, AV_LOG_INFO,"Transcoding frame: \n ");
	encodec_packet.data = NULL;
	encodec_packet.size = 0;
	//Inicjacja opcjonalnych czêœci danego pakietu z domyœlnymi danymi
	av_init_packet(&encodec_packet);

	//Wys³anie surowej ramki do kodera na h.265
	ret = avcodec_send_frame(stream->encodec_ctx, frame);
	if (ret < 0)
		return ret;

	while (ret >= 0) {

		//Otrzymanie skompresowanej za pomoc¹ kodeka, ramki AVPacket.
		ret = avcodec_receive_packet(stream->encodec_ctx, &encodec_packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return 0;
		//Przygotowanie pakietu do multipleksacji
		encodec_packet.stream_index = stream_index;
		encodec_packet.duration = duration;
		//Konfiguracj¹ sygnatury czasowej: 
		av_packet_rescale_ts(&encodec_packet, stream->encodec_ctx->time_base, out_format_ctx->streams[stream_index]->time_base);
		av_log(NULL, AV_LOG_DEBUG, "Muxing frame \n");
		//Multipleksacja zakodowanej ramki/pakietu, zapis do wyjœciowego pliku
		ret = av_interleaved_write_frame(out_format_ctx, &encodec_packet);

	}
	return ret;
}
static int flush_encoder(unsigned int stream_index) {
	if (!(stream_context[stream_index].encodec_ctx->codec->capabilities & AV_CODEC_CAP_DELAY))
		return 0;
	av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
	return encode(NULL, stream_index, NULL);
}

char* read_line(int size, char separator)
{
	char format[32];
	sprintf_s(format, " %%%d[^%c]", size, separator); //Debuger w tym miejscu zg³asza wyj¹tek
	char* bufor = (char*)malloc(size + 1);
	scanf_s(format, bufor);
	return realloc(bufor, strlen(bufor) + 1);
}






	JNIEXPORT void JNICALL Java_example_Iloczyn_transcode
	(JNIEnv* env, jobject obj, jstring inputFilePath, jstring outputFilePath, jstring transcodingPreset, jstring crfValue) {

		const char* crf = (*env)->GetStringUTFChars(env, crfValue, NULL);
		const char* preset = (*env)->GetStringUTFChars(env, transcodingPreset, NULL);
		const char* input_file = (*env)->GetStringUTFChars(env, inputFilePath, NULL);
		const char* output_file = (*env)->GetStringUTFChars(env, outputFilePath, NULL);



		if (crf == NULL) {
			printf("Could not allocate memory!");
			exit(1);
		}
		
		int ret;
		AVPacket packet;
		packet.data = NULL;
		packet.size = 0;

		unsigned int stream_index;
		unsigned int i;

		if ((open_input_file(input_file)) < 0)
			goto end;
		printf("Przygotowanie wyjsciowego pliku: \n");
		if ((open_output_file(output_file, crf, preset)) < 0)
			goto end;


		//Read packets:
		while (1) {
			if ((ret = av_read_frame(in_format_ctx, &packet)) < 0) break;
			stream_index = packet.stream_index;
			av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n", stream_index);
			StreamContext* stream = &stream_context[stream_index];
			av_log(NULL, AV_LOG_DEBUG, "Starting to decode frame\n");
			//Konwersja znaczników czasu do nowego pakietu
			av_packet_rescale_ts(&packet, in_format_ctx->streams[stream_index]->time_base, stream->decodec_ctx->time_base);

			//Dostarczenie pustego AVPAcket do dekodera 
			ret = avcodec_send_packet(stream->decodec_ctx, &packet);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Error in decoding\n");
				break;
			}
			while (ret >= 0) {
				//Otrzymanie nieskompresowanej AVFRame
				ret = avcodec_receive_frame(stream->decodec_ctx, stream->decoded_frame);
				if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) break;
				else if (ret < 0) goto end;


				//PTS - Presentation timestamp in time_base units . (Time when frame should be shown to user)
				// Best Effort timestamp - frame timestamp estimated using varioous heuristic.

				ret = encode(stream->decoded_frame, stream_index, packet.duration);
				if (ret < 0) goto end;
			}
			av_packet_unref(&packet);
		}
		//flush  and encoders
		for (i = 0; i < in_format_ctx->nb_streams; i++) {
			ret = flush_encoder(i);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "error in flushing encoder\n");
				goto end;
			}
		}
		av_write_trailer(out_format_ctx);
	end:

		av_packet_unref(&packet);
		for (i = 0; i < in_format_ctx->nb_streams; i++) {
			avcodec_free_context(&stream_context[i].decodec_ctx);
			if (out_format_ctx && out_format_ctx->nb_streams > i && out_format_ctx->streams[i] && stream_context[i].encodec_ctx)
				avcodec_free_context(&stream_context->encodec_ctx);
			av_frame_free(&stream_context[i].decoded_frame);
		}
		av_free(stream_context);
		avformat_close_input(&in_format_ctx);
		if (out_format_ctx && !(out_format_ctx->oformat->flags & AVFMT_NOFILE))
			avio_closep(&out_format_ctx->pb);
		avformat_free_context(out_format_ctx);
		if (ret < 0)
			av_log(NULL, AV_LOG_ERROR, " Error occurred: %s\n", av_err2str(ret));

		return ret ? 1 : 0;
	}

	
