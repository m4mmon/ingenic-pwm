#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "pwm.h"

#define TIMEOUT		20

int main(int argc, char **argv)
{
	int ret;
	SUPWMChnAttr attr = {0};  // Initialize the attributes structure to zeros
	int state = 0;
	int chan = -1;
	int polarity = -1;
	int period = -1;
	int duty = -1;
	int enable = -1;
	int disable = -1;
	int opt;

	if(argc < 3){
		printf("Ingenic T31 PWM Control Utility\n");
		printf("Usage:\n");
		printf("pwm_ctrl -c chn -p polarity [-e enable] [-d disable] [-m duty] [-s state] [-r period]\n");
		printf("Enable PWM CH0: ./pwm_ctrl -c 0 -p 0 -e 1\n");
		printf("Disable PWM CH0: ./pwm_ctrl -c 0 -p 0 -e 1 -d 1\n");
		return 0;
	}

	ret = SU_PWM_Init();
	if(ret < 0) {
		printf("SU_PWM_Init error !\n");
		return 0;
	}

	// Parse options first
	while((opt = getopt(argc, argv, "c:p:e:d:m:s:r:")) != -1) {
		switch(opt) {
			case 'c':
				chan = atoi(optarg);
				break;
			case 'p':
				polarity = atoi(optarg);
				break;
			case 'e':
				enable = atoi(optarg);
				break;
			case 'd':
				disable = atoi(optarg);
				break;
			case 'm':
				duty = atoi(optarg);
				break;
			case 'r':
				period = atoi(optarg);
				break;
			default:
				printf("Usage: %s -c channel -p polarity -e enable -d disable -m duty -s state -r period\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

// Check if required attributes are set for creation/modification
if (enable == 1 && (chan == -1 )) {
    printf("Required attributes missing for enabling. Ensure you provide channel, polarity, period, and duty.\n");
    return 0;
} else if (disable == 1 && chan == -1) {
    printf("Channel number must be provided to disable.\n");
    return 0;
}

// If we're enabling, set the attributes and create the channel
if(enable == 1) {
    attr.period = period;
    attr.duty = duty;
    attr.polarity = polarity;

    // Create the PWM channel
    ret = SU_PWM_CreateChn(chan, &attr);
    if(ret < 0) {
        printf("SU_PWM_CreateChn error !\n");
        return 0;
    }

    ret = SU_PWM_EnableChn(chan);
    if(ret < 0) {
        printf("SU_PWM_EnableChn error !\n");
        return 0;
    }
    state = 1;
} else if(disable == 1) {
    SU_PWM_DisableChn(chan);
    state = 0;
}
	SU_PWM_DestroyChn(chan);

	SU_PWM_Exit();

	return 0;
}
