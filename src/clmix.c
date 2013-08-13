#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "mixtree.h"
#include "util.h"
#include "audioencoder.h"

/**
 * Show help text.
 */
void printhelp() {
	char *help=
		"Usage: mixer [options] <spec file>\n"
		"Mix sounds according to specifications file.\n"
		"\n"
		"Options:\n"
		"\n"
		"  -h              Show this help.\n"
		"  -o filename     Output to filename.\n"
		"                  If not specified, use stdout.\n"
		"  -p position     Start position in millisec.\n"
		"  -g              Show progress.\n"
		"  -f frame_size   Size in millisecs of chunks written\n"
		"                  to output stream, default 1000.\n"
		"  -t              Synchronize output so the length is\n "
		"                  approximately equal to the time elapsed.\n"
		"  -i frames       If -t is used, output this many frames\n"
		"                  before synchronizing output.\n"
		"  -r format       Select output format, default is mp3.\n"
		"                    Available: mp3 flv wav\n"
		"\n";

	printf("%s",help);
	exit(1);
}

/**
 * Report progress.
 */
void progress(int percent) {
	printf("\rProgress: %d%%",percent);
	fflush(stdout);
}

/**
 * Main.
 */
int main(int argc, char **argv) {
	char c;
	char *filename=NULL;
	int startpos=0;
	int frame_size=1000;
	int do_time_sync=0;
	int initial_frames=0;
	char *format=NULL;
	int show_progress=0;

	while ((c=getopt(argc,argv,"hf:p:o:i:tr:g")) != -1) {
		switch (c) {
			case 'h':
				printhelp();
				exit(1);
				break;

			case 'f':
				frame_size=atoi(optarg);
				break;

			case 'p':
				startpos=atoi(optarg);
				break;

			case 'o':
				filename=strdup(optarg);
				//filename=optarg;
				break;

			case 'i':
				initial_frames=atoi(optarg);
				break;

			case 'g':
				show_progress=1;
				break;

			case 't':
				do_time_sync=1;
				break;

			case 'r':
				format=optarg;
				break;

			default:
				fail("Usage: mixer [options] <spec file>\nTry mixer -h for help.");
				break;
		}
	}

	if (show_progress && !filename)
		fail("Cannot show progress if outputting to stdout.");

	if (argc-optind!=1)
		fail("Usage: mixer [options] <spec file>\nTry mixer -h for help.");

	AUDIOSOURCE *audiosource=mixtree_load_file(argv[optind]);

	int len=samples_to_millis(audiosource_get_numsamples(audiosource));
	TRACE("audiosource length: %d\n",len);
	TRACE("creating encoder\n");
	AUDIOENCODER *encoder;

	encoder=audioencoder_create(filename,audiosource,startpos,format);

	unsigned long start_time=millitime();
	int frame_count=0;
	int i;
	int percent=0, oldpercent=0;
	int pos=startpos;

	TRACE("calling eof...\n");
	TRACE("eof: %d\n",audioencoder_eof(encoder));

	if (show_progress)
		progress(0);

	for (i=0; i<initial_frames; i++)
		if (!audioencoder_eof(encoder))
			pos+=samples_to_millis(audioencoder_encode(encoder,frame_size));

	if (show_progress) {
		percent=100*(pos-startpos)/(len-startpos);
		progress(percent);
		oldpercent=percent;
	}

	while (!audioencoder_eof(encoder)) {
		pos+=samples_to_millis(audioencoder_encode(encoder,frame_size));
		frame_count++;

		if (show_progress) {
			percent=100*(pos-startpos)/(len-startpos);
			progress(percent);
			oldpercent=percent;
		}

		if (do_time_sync) {
			unsigned long should_be_at=start_time+frame_count*frame_size;
			unsigned long is_at=millitime();

			if (is_at<should_be_at)
				millisleep(should_be_at-is_at);
		}
	}

	audioencoder_dispose(encoder);

	if (show_progress) {
		progress(100);
		printf("\n");
	}

	TRACE("all done!\n");

	return 0;
}
